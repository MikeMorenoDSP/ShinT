#include "PluginProcessor.h"
#include "PluginEditor.h"

ShinTAudioProcessor::ShinTAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : MagicProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), apvts (*this, nullptr, "PARAMETERS", createParameters())
#endif
{
    initSynth();
    magicState.setGuiValueTree(BinaryData::magic_xml, BinaryData::magic_xmlSize);
}

ShinTAudioProcessor::~ShinTAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout ShinTAudioProcessor::createParameters()
{
    juce::AudioProcessorValueTreeState::ParameterLayout parameters;
    
    parameters.add (std::make_unique<juce::AudioParameterInt>(juce::ParameterID ("osc1Waveform", 1), "osc1Waveform", 0, 2, 1));
    parameters.add (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID ("subOscLevel", 1), "subOscLevel", 0.0f, 1.0f, 0.0f));
    parameters.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("oscMix", 1), "oscMix", 0.0f, 1.0f, 0.0f));

    parameters.add (std::make_unique<juce::AudioParameterInt>(juce::ParameterID ("osc2Waveform", 1), "osc2Waveform", 0, 2, 1));
    parameters.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID("osc2Octave", 1), "osc2Octave", -2, 2, 0));
    parameters.add (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID ("osc2Pitch", 1), "osc2Pitch", -12.0f, 12.0f, 0.0f));

    parameters.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("cutoff", 1), "cutoff", 20.0f, 120.0f, 120.0f));
    parameters.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("resonance", 1), "resonance", 0.0f, 1.0f, 0.0f));
    parameters.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("ampEnvCutoff", 1), "ampEnvCutoff", 0.0f, 96.0f, 0.0f));
    parameters.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("Gain", 1), "Gain", -60.0f, 6.0f, 0.0f));


    parameters.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("ampEnvAttack", 1), "ampEnvAttack", 0.01f, 2.0f, 0.01f));
    parameters.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("ampEnvDecay", 1), "ampEnvDecay", 0.01f, 2.0f, 0.1f));
    parameters.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("ampEnvSustain", 1), "ampEnvSustain", 0.00000001f, 1.0f, 1.0f));
    parameters.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("ampEnvRelease", 1), "ampEnvRelease", 0.01f, 2.0f, 0.01f));

    parameters.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("lfoRate", 1), "lfoRate", -90.0f, 45.0f, -45.0f));
    parameters.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID("lfoWaveform", 1), "lfoWaveform", 0, 3, 0));
    parameters.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("lfoPitch", 1), "lfoPitch", 0.0f, 48.0f, 0.0f));
    parameters.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("lfoCutoff", 1), "lfoCutoff", 0.0f, 96.0f, 0.0f));


    return parameters;
}

void ShinTAudioProcessor::initSynth()
{
    synth.addSound (new SynthSound());
    
    for (auto i = 0; i < 10; i++)
        synth.addVoice (new SynthVoice());
}

const juce::String ShinTAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ShinTAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ShinTAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ShinTAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ShinTAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ShinTAudioProcessor::getNumPrograms()
{
    return 1;
}

int ShinTAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ShinTAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ShinTAudioProcessor::getProgramName (int index)
{
    return {};
}

void ShinTAudioProcessor::changeProgramName (int index, const juce::String& newName) {}

void ShinTAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate (sampleRate);
    
    for (int i = 0; i < synth.getNumVoices(); i++)
    {
        if (auto voice = dynamic_cast<SynthVoice*> (synth.getVoice(i)))
            voice->prepareToPlay (sampleRate, samplesPerBlock, getTotalNumInputChannels());
    }
}

void ShinTAudioProcessor::releaseResources() {}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ShinTAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void ShinTAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    for (int i = 0; i < synth.getNumVoices(); i++)
    {
        if (auto voice = dynamic_cast<SynthVoice*>(synth.getVoice(i)))
            voice->updateParameters (apvts);
    }
    
    synth.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());
}

//bool ShinTAudioProcessor::hasEditor() const
//{
//    return true;
//}
//
//juce::AudioProcessorEditor* ShinTAudioProcessor::createEditor()
//{
//    //return new ShinTAudioProcessorEditor (*this);
//    return new juce::GenericAudioProcessorEditor (*this);
//}

//void ShinTAudioProcessor::getStateInformation (juce::MemoryBlock& destData) {}
//
//
//void ShinTAudioProcessor::setStateInformation (const void* data, int sizeInBytes) {}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ShinTAudioProcessor();
}
