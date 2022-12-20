#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MultiBassAudioProcessor::MultiBassAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
    apvts(*this, nullptr, "PARAMS", createParameterLayout())
#endif
{    
    sampleRate = 0.0;
    level = dBtoRatio(LEVEL_DEFAULT);
    drive = dBtoRatio(DRIVE_DEFAULT);
    highLevel = dBtoRatio(HI_LVL_DEFAULT);
    bandpassQ = calcBandpassQ(DRIVE_DEFAULT);
    blend = BLEND_DEFAULT;
    xover = XOVER_DEFAULT;

    numChannels = getTotalNumInputChannels();

    for (int channel = 0; channel < numChannels; channel++)
    {
        lowerSplitters.push_back(std::make_unique<BandSplitter>());
        upperSplitters.push_back(std::make_unique<BandSplitter>());
        bandpassFilters.push_back(std::make_unique<Filter>());
    }

    apvts.addParameterListener(LEVEL_ID, this);
    apvts.addParameterListener(DRIVE_ID, this);
    apvts.addParameterListener(XOVER_ID, this);
    apvts.addParameterListener(HI_LVL_ID, this);
    apvts.addParameterListener(BLEND_ID, this);

    apvts.state = juce::ValueTree(JucePlugin_Name);
}

MultiBassAudioProcessor::~MultiBassAudioProcessor()
{
}

//==============================================================================
ParameterLayout MultiBassAudioProcessor::createParameterLayout()
{
    ParameterLayout params;

    params.add(std::make_unique<juce::AudioParameterFloat>(LEVEL_ID,
                                                           LEVEL_NAME,
                                                           juce::NormalisableRange<float>(LEVEL_MIN,
                                                                                          LEVEL_MAX,
                                                                                          LEVEL_INTERVAL),
                                                           LEVEL_DEFAULT));

    params.add(std::make_unique<juce::AudioParameterFloat>(DRIVE_ID,
                                                           DRIVE_NAME,
                                                           juce::NormalisableRange<float>(DRIVE_MIN,
                                                                                          DRIVE_MAX,
                                                                                          DRIVE_INTERVAL),
                                                           DRIVE_DEFAULT));

    juce::NormalisableRange<float> xoverRange(XOVER_MIN, XOVER_MAX, XOVER_INTERVAL);
    xoverRange.setSkewForCentre(XOVER_DEFAULT);
    params.add(std::make_unique<juce::AudioParameterFloat>(XOVER_ID,
                                                           XOVER_NAME,
                                                           xoverRange,
                                                           XOVER_DEFAULT));

    params.add(std::make_unique<juce::AudioParameterFloat>(HI_LVL_ID,
                                                           HI_LVL_NAME,
                                                           juce::NormalisableRange<float>(HI_LVL_MIN,
                                                                                          HI_LVL_MAX,
                                                                                          HI_LVL_INTERVAL),
                                                           HI_LVL_DEFAULT));

    params.add(std::make_unique<juce::AudioParameterFloat>(BLEND_ID,
                                                           BLEND_NAME,
                                                           juce::NormalisableRange<float>(BLEND_MIN,
                                                                                          BLEND_MAX,
                                                                                          BLEND_INTERVAL),
                                                           BLEND_DEFAULT));
    return params;
}

void MultiBassAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == LEVEL_ID)
        level = dBtoRatio(newValue);

    else if (parameterID == DRIVE_ID)
    {
        drive = dBtoRatio(newValue);
        bandpassQ = calcBandpassQ(newValue);

        for (int channel = 0; channel < numChannels; channel++)
            bandpassFilters[channel]->coefficients = Coefficients::makeBandPass(sampleRate,
                                                                                BANDPASS_FREQ,
                                                                                bandpassQ);
    }

    else if (parameterID == XOVER_ID)
    {
        xover = newValue;

        for (int channel = 0; channel < numChannels; channel++)
            lowerSplitters[channel]->calcCoeffs(sampleRate, xover);
    }

    else if (parameterID == HI_LVL_ID)
        highLevel = dBtoRatio(newValue);

    else if (parameterID == BLEND_ID)
        blend = newValue;
}

void MultiBassAudioProcessor::loadImpulseResponse()
{
    fileChooser = std::make_unique<juce::FileChooser>("Choose impulse response...",
                                                      juce::File::getSpecialLocation(juce::File::userHomeDirectory),
                                                      "*.wav");

    auto folderChooserFlags = juce::FileBrowserComponent::openMode;

    fileChooser->launchAsync(folderChooserFlags, [this](const juce::FileChooser& chooser)
                           {
                                 juce::File impulseResponse = chooser.getResult();

                                 convolution.loadImpulseResponse(impulseResponse,
                                                                 juce::dsp::Convolution::Stereo::yes,
                                                                 juce::dsp::Convolution::Trim::yes,
                                                                 0);
                           });
}

//==============================================================================
const juce::String MultiBassAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MultiBassAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MultiBassAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MultiBassAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MultiBassAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int MultiBassAudioProcessor::getNumPrograms() { return 1; }

int MultiBassAudioProcessor::getCurrentProgram() { return 0; }

void MultiBassAudioProcessor::setCurrentProgram (int index) {}

const juce::String MultiBassAudioProcessor::getProgramName (int index) { return {}; }

void MultiBassAudioProcessor::changeProgramName (int index, const juce::String& newName) {}

//==============================================================================
void MultiBassAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    this->sampleRate = sampleRate;
    
    for (int channel = 0; channel < numChannels; channel++)
    {
        lowerSplitters[channel]->calcCoeffs(sampleRate, xover);
        upperSplitters[channel]->calcCoeffs(sampleRate, UPPER_FREQ);
        bandpassFilters[channel]->coefficients = Coefficients::makeBandPass(sampleRate, 
                                                                            BANDPASS_FREQ,
                                                                            bandpassQ);
    }

    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    convolution.reset();
    convolution.prepare(spec);
}

void MultiBassAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MultiBassAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void MultiBassAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    juce::dsp::AudioBlock<float> block(buffer);

    // Main processing loop
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            std::array<float, 2> lowerBands = lowerSplitters[channel]->processSample(channelData[sample]);

            auto lowBand = lowerBands[0]; // Low band
            auto upperBand = lowerBands[1]; // Upper band that needs to be split further into mid and high bands

            std::array<float, 2> upperBands = upperSplitters[channel]->processSample(upperBand);

            // Mid and high band created from splitting the upper band created before
            auto midBand = upperBands[0];
            auto highBand = upperBands[1];

            highBand *= highLevel;

            upperBand = midBand + highBand; // Recombine mid and high into upper band
            upperBand = saturateSample(channel, upperBand, drive);

            // Weighted sum of lower and upper bands, with weighting determined by the "blend" variable
            channelData[sample] = 2.0f * ((1.0f - blend) * lowBand) + (blend * upperBand);

            channelData[sample] *= level;
        }
    }

    if (convolution.getCurrentIRSize() > 0)
        convolution.process(juce::dsp::ProcessContextReplacing<float>(block));
}

float MultiBassAudioProcessor::saturateSample(int channel, float sample, float gain)
{    
    auto x = sample;
    x = bandpassFilters[channel]->processSample(x);
    x = atan(gain * x) / sqrt(gain);
    return x;
}

//==============================================================================
bool MultiBassAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* MultiBassAudioProcessor::createEditor()
{
    return new MultiBassAudioProcessorEditor (*this);
}

//==============================================================================
void MultiBassAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
}

void MultiBassAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MultiBassAudioProcessor();
}
