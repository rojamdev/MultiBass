#pragma once

#include <JuceHeader.h>
#include "BandSplitter.h"
#include "Constants.h"

typedef juce::AudioProcessorValueTreeState::ParameterLayout ParameterLayout;
typedef juce::dsp::NoiseGate<float> NoiseGate;
typedef juce::dsp::Compressor<float> Compressor;
typedef juce::dsp::IIR::Filter<float> Filter;
typedef juce::dsp::IIR::Coefficients<float> Coefficients;

//==============================================================================
class MultiBassAudioProcessor  : public juce::AudioProcessor,
                                 public juce::AudioProcessorValueTreeState::Listener,
                                 public juce::ChangeBroadcaster
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    MultiBassAudioProcessor();
    ~MultiBassAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    void loadImpulseResponse();

    //==============================================================================
    juce::AudioProcessorValueTreeState apvts;

private:
    //==============================================================================
    ParameterLayout createParameterLayout();
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    float saturateSample(int channel, float sample, float gain);

    inline float dBtoRatio(float dB)
    {
        return pow(10, dB / 20);
    }

    inline float calcBandpassQ(float gain)
    {
        return ((gain / Q_CONSTANT) + 0.01f);
    }

    //==============================================================================
    std::vector<std::unique_ptr<BandSplitter>> lowerSplitters, upperSplitters;
    std::vector<std::unique_ptr<Filter>> bandpassFilters;
    juce::dsp::Convolution convolution;
    juce::dsp::ProcessSpec spec;
    int numChannels;
    double sampleRate;
    float level, drive, xover, highLevel, blend, bandpassQ;
    std::unique_ptr<juce::FileChooser> fileChooser;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiBassAudioProcessor)
};
