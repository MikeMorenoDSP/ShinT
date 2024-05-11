#include "PluginProcessor.h"
#include "PluginEditor.h"

ShinTAudioProcessorEditor::ShinTAudioProcessorEditor (ShinTAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (400, 300);
}

ShinTAudioProcessorEditor::~ShinTAudioProcessorEditor() {}

void ShinTAudioProcessorEditor::paint (juce::Graphics& g) {}

void ShinTAudioProcessorEditor::resized() {}
