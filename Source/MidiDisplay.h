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
    void setQuantizedMidi(const juce::Array<MidiEvent>& allMidi, juce::AudioPlayHead* playHead);

    //==============================================================================

private:
    //==============================================================================
    double m_beatsToShow;
    juce::AudioPlayHead::TimeSignature m_timeSignature;

    //==============================================================================
};