#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MultiBassAudioProcessorEditor::MultiBassAudioProcessorEditor (MultiBassAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setLookAndFeel(&lookAndFeel);
    defaultFont.setBold(true);

    createControl(LEVEL_ID, LEVEL_NAME, levelSlider, levelLabel, levelAttachment, 0, 0, 100, 100, true);
    createControl(DRIVE_ID, DRIVE_NAME, driveSlider, driveLabel, driveAttachment, 100, 0, 100, 100, true);
    createControl(XOVER_ID, XOVER_NAME, xoverSlider, xoverLabel, xoverAttachment, 200, 0, 100, 100, true);
    createControl(HI_LVL_ID, HI_LVL_NAME, highLevelSlider, highLevelLabel, highLevelAttachment, 300, 0, 100, 100, true);
    createControl(BLEND_ID, BLEND_NAME, blendSlider, blendLabel, blendAttachment, 400, 0, 100, 100, true);

    addAndMakeVisible(loadFileButton);
    loadFileButton.setButtonText("Load IR");

    loadFileButton.onClick = [this]() {
        audioProcessor.loadImpulseResponse(); 
    };

    setSize(GUI_WIDTH, GUI_HEIGHT);
}

MultiBassAudioProcessorEditor::~MultiBassAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void MultiBassAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::darkgrey);
}

void MultiBassAudioProcessorEditor::resized()
{
    loadFileButton.setBounds(0, 150, 100, 40);
}

void MultiBassAudioProcessorEditor::createControl(juce::String parameterID, 
                                                  juce::String parameterName, 
                                                  juce::Slider& slider, juce::Label& label, 
                                                  std::unique_ptr<SliderAttachment>& attachment, 
                                                  int posX, int posY, 
                                                  int height, int width, 
                                                  bool showTextBox)
{
    addAndMakeVisible(slider);
    attachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, parameterID, slider);
    slider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    slider.setBounds(posX, posY, height, width);
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);

    juce::Slider::TextEntryBoxPosition boxPos;
    if (showTextBox) boxPos = juce::Slider::TextBoxBelow;
    else boxPos = juce::Slider::NoTextBox;
    slider.setTextBoxStyle(boxPos, true, 100, height / 5);

    addAndMakeVisible(label);
    label.setText(parameterName, juce::dontSendNotification);
    label.setBounds(posX, posY + height, width, height / 5);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(defaultFont);
}
