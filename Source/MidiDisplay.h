#pragma once

#include <JuceHeader.h>
#include "MidiEvent.h"

extern const double g_quarterNoteTicks;

class MidiDisplay : public juce::Component
{
public:
    //==============================================================================
    MidiDisplay() {}

    void paint(juce::Graphics& g) override;
    void resized() override;

    //==============================================================================
    void setQuantizedMidi(const juce::Array<MidiEvent>& newQuantizedMidi);
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
        m_beatStart = measureStart * timeSignature.numerator;
        m_beatEnd = (measureStart + length) * timeSignature.numerator;
        if (repaintMidi)
            repaint();
    }

    //==============================================================================
    juce::String debugMidiDisplay()
    {
        juce::String output = "MidiDisplay:\n";

        output += "m_quantizedMidi:\n";
        for (auto& m : m_quantizedMidi)
        {
            output += m.debugMidiEvent() + "\n";
        }
        output += "\n";

        output += "m_analyzedMidi:\n";
        for (auto& m : m_analyzedMidi)
        {
            output += m.debugMidiEvent() + "\n";
        }
        output += "\n";

        output += "timeSignature.numerator: " + juce::String(timeSignature.numerator) + "\n";
        output += "timeSignature.denominator: " + juce::String(timeSignature.denominator) + "\n";
        output += "m_beatSubDivisions: " + juce::String(m_beatSubDivisions) + "\n";
        output += "m_quantizedBeatRange: " + juce::String(m_quantizedBeatRange) + "\n";
        output += "m_beatStart: " + juce::String(m_beatStart) + "\n";
        output += "m_beatEnd: " + juce::String(m_beatEnd) + "\n";
        output += "m_lowestNote: " + juce::String(m_lowestNote) + "\n";
        output += "m_highestNote: " + juce::String(m_highestNote) + "\n";
        output += "m_msTimeThreshold: " + juce::String(m_msTimeThreshold) + "\n";
        output += "noteDisplayWidth: " + juce::String(noteDisplayWidth) + "\n";
        output += "analyzedNoteDisplayWidth: " + juce::String(analyzedNoteDisplayWidth) + "\n";

        return output;
    }

    //==============================================================================

    juce::AudioPlayHead::TimeSignature timeSignature;

private:
    juce::Array<MidiEvent> m_quantizedMidi;
    juce::Array<MidiEvent> m_analyzedMidi;

    int m_beatSubDivisions = 4;
    double m_quantizedBeatRange = 0;
    int m_beatStart = 0;
    int m_beatEnd = 0;
    int m_lowestNote = 0;
    int m_highestNote = 0;

    //threshold for when a midi note is considered "on time" and not late or early
    double m_msTimeThreshold = 20;

    float noteDisplayWidth = 2;
    float analyzedNoteDisplayWidth = 4;
    const juce::Colour quantizedColor{ 0xffbbbbbb };
    const juce::Colour onTimeColor{ 0xff44dd44 };
    const juce::Colour lateColor{ 0xffdd4444 };
    const juce::Colour earlyColor{ 0xffd49306 };

    //==============================================================================
};