#pragma once
#include <JuceHeader.h>
#include "SynthSound.h"
#include "DSP/oscillator.h"
#include "DSP/ladder.h"
#include "DSP/adsr.h"


class SynthVoice : public juce::SynthesiserVoice
{
public:
    
    SynthVoice();
    ~SynthVoice() override;
    
    bool canPlaySound (juce::SynthesiserSound*) override;
    
    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound *sound, int currentPitchWheelPosition) override;
    
    void stopNote (float velocity, bool allowTailOff) override;
    
    void controllerMoved (int controllerNumber, int newControllerValue) override;
    
    void pitchWheelMoved (int newPitchWheelValue) override;
    
    void prepareToPlay (double inSampleRate, int inSamplesPerBlock, int inNumChannels);
    
    void updateParameters (juce::AudioProcessorValueTreeState& apvts);
    
    void renderNextBlock (juce::AudioBuffer<float> &outputBuffer, int startSample, int numSamples) override;
    
private:
    
    bool isPrepared { false };
    int midiNote   { 0 };
    bool gate;

    daisysp::Oscillator osc, osc2, subOsc, lfo;

    daisysp::LadderFilter filter;
    daisysp::Adsr ampEnv;

    juce::dsp::Gain<float> gain;

    // Parametros
    int osc1WaveformParam;
    float subOscLevelParam;
    float oscMixParam;

    int osc2WaveformParam;
    int osc2OctaveParam;
    float osc2PitchParam;
    float cutoffParam;
    float resonanceParam;
    float ampEnvCutoffParam;

    float ampEnvAttackParam;
    float ampEnvDecayParam;
    float ampEnvSustainParam;
    float ampEnvReleaseParam;

    float lfoRateParam;
    int lfoWaveformParam;
    float lfoPitchParam;
    float lfoCutoffParam;

    juce::AudioBuffer<float> synthBuffer;
};
