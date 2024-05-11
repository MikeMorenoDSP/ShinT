#include "SynthVoice.h"

SynthVoice::SynthVoice() {}

SynthVoice::~SynthVoice() {}

bool SynthVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<juce::SynthesiserSound*>(sound) != nullptr;
}

void SynthVoice::startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound *sound, int currentPitchWheelPosition)
{
    midiNote = midiNoteNumber;
    lfo.Reset();
    gate = true;
}

void SynthVoice::stopNote (float velocity, bool allowTailOff)
{
    gate = false;
    
    if (!allowTailOff || !ampEnv.IsRunning())
        clearCurrentNote();
}

void SynthVoice::controllerMoved (int controllerNumber, int newControllerValue) {}

void SynthVoice::pitchWheelMoved (int newPitchWheelValue) {}

void SynthVoice::prepareToPlay (double sampleRate, int samplesPerBlock, int numChannels)
{   
    juce::dsp::ProcessSpec spec;
    spec.sampleRate         = sampleRate;
    spec.numChannels        = static_cast<juce::uint32> (numChannels);
    spec.maximumBlockSize   = static_cast<juce::uint32> (samplesPerBlock);

    osc.Init(sampleRate);
    osc2.Init(sampleRate);
    subOsc.Init(sampleRate);

    ampEnv.Init(sampleRate);
    filter.Init(sampleRate);
    lfo.Init(sampleRate);

    osc.SetWaveform(daisysp::Oscillator::WAVE_POLYBLEP_SAW);
    osc2.SetWaveform(daisysp::Oscillator::WAVE_POLYBLEP_SAW);
    subOsc.SetWaveform(daisysp::Oscillator::WAVE_POLYBLEP_TRI);
    lfo.SetWaveform(daisysp::Oscillator::WAVE_TRI);

    ampEnv.SetAttackTime(0.01f, 0.0f);
    ampEnv.SetDecayTime(1.0f);
    ampEnv.SetSustainLevel(1.0f);
    ampEnv.SetReleaseTime(0.01f);

    gain.prepare (spec);

    isPrepared = true;
}

void SynthVoice::updateParameters(juce::AudioProcessorValueTreeState& apvts)
{
    auto gainValue = apvts.getRawParameterValue("Gain")->load();
    osc1WaveformParam = *apvts.getRawParameterValue("osc1Waveform");
    subOscLevelParam = *apvts.getRawParameterValue("subOscLevel");
    oscMixParam = *apvts.getRawParameterValue("oscMix");

    osc2WaveformParam = *apvts.getRawParameterValue("osc2Waveform");
    osc2OctaveParam = *apvts.getRawParameterValue("osc2Octave");
    osc2PitchParam = *apvts.getRawParameterValue("osc2Pitch");
    cutoffParam = *apvts.getRawParameterValue("cutoff");
    resonanceParam = *apvts.getRawParameterValue("resonance");
    ampEnvCutoffParam = *apvts.getRawParameterValue("ampEnvCutoff");

    ampEnvAttackParam = *apvts.getRawParameterValue("ampEnvAttack");
    ampEnvDecayParam = *apvts.getRawParameterValue("ampEnvDecay");
    ampEnvSustainParam = *apvts.getRawParameterValue("ampEnvSustain");
    ampEnvReleaseParam = *apvts.getRawParameterValue("ampEnvRelease");

    lfoRateParam = *apvts.getRawParameterValue("lfoRate");
    lfoWaveformParam = *apvts.getRawParameterValue("lfoWaveform");
    lfoPitchParam = *apvts.getRawParameterValue("lfoPitch");
    lfoCutoffParam = *apvts.getRawParameterValue("lfoCutoff");

    gain.setGainDecibels (gainValue);

    osc.SetWaveform(osc1WaveformParam + 5);
    osc2.SetWaveform(osc2WaveformParam + 5);
    lfo.SetWaveform(lfoWaveformParam + 1);

    ampEnv.SetAttackTime(ampEnvAttackParam);
    ampEnv.SetDecayTime(ampEnvDecayParam);
    ampEnv.SetSustainLevel(ampEnvSustainParam);
    ampEnv.SetReleaseTime(ampEnvReleaseParam);

    filter.SetFilterMode(daisysp::LadderFilter::FilterMode::LP24);
    filter.SetRes(resonanceParam);

}

void SynthVoice::renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    jassert (isPrepared);
    
    if (!isVoiceActive())
        return;
    
    synthBuffer.setSize (outputBuffer.getNumChannels(), numSamples, false, false, true);
    synthBuffer.clear();
    juce::dsp::AudioBlock<float> audioBlock { synthBuffer };
    juce::dsp::ProcessContextReplacing<float> block (audioBlock);

    lfo.SetFreq(2.0f);
  
    for (size_t i = 0; i < audioBlock.getNumSamples(); i++)
    {
        lfo.SetFreq(daisysp::mtof(lfoRateParam));
        float pitchLfo = lfo.Process() * lfoPitchParam;
        osc.SetFreq(daisysp::mtof(midiNote + pitchLfo));
        subOsc.SetFreq(daisysp::mtof(midiNote + pitchLfo - 12.0f));
        osc2.SetFreq(daisysp::mtof(midiNote + pitchLfo + osc2PitchParam + static_cast<float>(osc2OctaveParam*12)));

        float saws = osc.Process()* (1.0f - oscMixParam) + osc2.Process() * oscMixParam + subOsc.Process() * subOscLevelParam;
        float _ampEnv = ampEnv.Process(gate);
        float cutoffModulation = daisysp::mtof(cutoffParam + _ampEnv * ampEnvCutoffParam + lfo.Process() * lfoCutoffParam);
        filter.SetFreq(cutoffModulation);
        float output = _ampEnv * filter.Process(saws*0.5f);
        audioBlock.setSample(0, i, output);
        audioBlock.setSample(1, i, output);
    }

    gain.process (block);
    
    for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
    {
        outputBuffer.addFrom (channel, startSample, synthBuffer, channel, 0, numSamples);
        
        if (!ampEnv.IsRunning())
            clearCurrentNote();
    }
}
