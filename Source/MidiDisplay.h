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

    //set threshold for when a midi note is considered "on time" and not late or early
    void setTimeThreshold(double ms, bool repaintMidi)
    {
        if (ms < 0)
            return; //not valid

        m_msTimeThreshold = ms;
        if (repaintMidi)
            repaint();
    }

    //the measures that the midi display should show
    void setMeasureRange(int measureStart, int length, bool repaintMidi)
    {
        m_beatStart = measureStart * m_timeSignature.numerator;
        m_beatEnd = (measureStart + length) * m_timeSignature.numerator;
        if (repaintMidi)
            repaint();
    }

    //==============================================================================

    //==============================================================================

private:
    //==============================================================================
    juce::Array<MidiEvent> m_quantizedMidi;
    juce::Array<MidiEvent> m_analyzedMidi;

    juce::AudioPlayHead::TimeSignature m_timeSignature;
    int m_beatSubDivisions;
    double m_quantizedBeatRange;
    int m_beatStart;
    int m_beatEnd;
    int m_lowestNote;
    int m_highestNote;

    //threshold for when a midi note is considered "on time" and not late or early
    double m_msTimeThreshold;

    float noteDisplayWidth;
    const juce::Colour quantizedColor{ 0xffbbbbbb };
    const juce::Colour onTimeColor{ 0xff44dd44 };
    const juce::Colour lateColor{ 0xffdd4444 };
    const juce::Colour earlyColor{ 0xffdddd44 };

    //==============================================================================
};