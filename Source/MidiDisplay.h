#pragma once

#include <JuceHeader.h>
#include "MidiEvent.h"

extern const double g_quarterNoteTicks;

struct MidiDisplay : public juce::Component
{
public:
    //==============================================================================
    MidiDisplay();

    void paint(juce::Graphics& g) override;
    void resized() override;

    //==============================================================================
    void setQuantizedMidi(const juce::Array<MidiEvent>& newQuantizedMidi, juce::AudioPlayHead* playHead);
    void setAnalyzedMidi(const juce::Array<MidiEvent>& newAnalyzedMidi);

    //==============================================================================

private:
    //==============================================================================
    int beatSubDivisions;
    double m_beatsToShow;
    int m_lowestNote;
    int m_highestNote;
    juce::AudioPlayHead::TimeSignature m_timeSignature;
    juce::Array<MidiEvent> m_quantizedMidi;
    juce::Array<MidiEvent> m_analyzedMidi;

    float noteDisplayWidth;

    //==============================================================================
};