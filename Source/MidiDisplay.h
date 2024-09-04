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
    void clearAnalyzedMidi(bool repaintMidi)
    {
        m_analyzedMidi.clear();
        if (repaintMidi)
            repaint();
    }

    //==============================================================================
    //threshold for when a midi note is considered "on time" and not late or early
    double msTimeThreshold;

    //==============================================================================

private:
    //==============================================================================
    int m_beatSubDivisions;
    double m_beatsToShow;
    int m_lowestNote;
    int m_highestNote;
    juce::AudioPlayHead::TimeSignature m_timeSignature;
    juce::Array<MidiEvent> m_quantizedMidi;
    juce::Array<MidiEvent> m_analyzedMidi;

    float noteDisplayWidth;
    const juce::Colour quantizedColor{ 0xffbbbbbb };
    const juce::Colour onTimeColor{ 0xff44dd44 };
    const juce::Colour lateColor{ 0xffdd4444 };
    const juce::Colour earlyColor{ 0xffdddd44 };

    //==============================================================================
};