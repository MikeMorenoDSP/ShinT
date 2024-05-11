#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class ShinTAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    
    ShinTAudioProcessorEditor (ShinTAudioProcessor&);
    ~ShinTAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:

    ShinTAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ShinTAudioProcessorEditor)
};
