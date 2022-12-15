#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "CustomLookAndFeel.h"

typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;

//==============================================================================
class MultiBassAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                       public juce::Button::Listener
{
public:
    MultiBassAudioProcessorEditor (MultiBassAudioProcessor&);
    ~MultiBassAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    //==============================================================================
    void createControl(juce::String parameterID,
                       juce::String parameterName,
                       juce::Slider& slider,
                       juce::Label& label,
                       std::unique_ptr<SliderAttachment>& attachment,
                       int posX, int posY,
                       int height, int width,
                       bool showTextBox);

    void buttonClicked(juce::Button* button) override;

    //==============================================================================
    MultiBassAudioProcessor& audioProcessor;

    juce::Label levelLabel, driveLabel, xoverLabel, highLevelLabel;
    juce::Slider levelSlider, driveSlider, xoverSlider, highLevelSlider;

    std::unique_ptr<SliderAttachment>
        levelAttachment, driveAttachment, xoverAttachment, highLevelAttachment;

    juce::TextButton loadFileButton;

    CustomLookAndFeel lookAndFeel;
    juce::Font defaultFont;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiBassAudioProcessorEditor)
};
