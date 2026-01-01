#pragma once

#include "Globals.h"
#include "MidiEvent.h"

extern const double g_defaultQuarterNoteTicks;

class MidiDisplay : public juce::Component
{
public:
    //==============================================================================
    MidiDisplay() {}

    void paint(juce::Graphics& g) override;
    void resized() override;

    //==============================================================================
    void setQuantizedMidi(const vArray<MidiEvent>& newQuantizedMidi);
    void setAnalyzedMidi(const vArray<MidiEvent>& newAnalyzedMidi);
    void updateAnalyzedMidi();
    void clearAnalyzedMidi(bool repaintMidi);

    void setBpm(double bpm, bool repaintMidi);
    //set threshold for when a midi note is considered "on time" and not late or early
    void setTimeThreshold(double ms, bool repaintMidi);

    //the measures that the midi display should show
    void setMeasureRange(double measureStart, double length, bool repaintMidi);
    //relative to measure start
    void setRecordStart(double measure, bool repaintMidi);
    //retruns the absolute tick start
    double getRecordTickStart(int quarterNoteTicks) { return (m_beatStart + m_recordBeatStart) * quarterNoteTicks; }

    //==============================================================================
    juce::String debugMidiDisplay();

    //==============================================================================

    juce::AudioPlayHead::TimeSignature timeSignature;

private:
    vArray<MidiEvent> m_quantizedMidi;
    vArray<MidiEvent> m_analyzedMidi;

    int m_beatSubDivisions = 4;
    double m_quantizedBeatRange = 0;
    double m_recordBeatStart = 0;
    double m_beatStart = 0;
    double m_beatEnd = 0;
    int m_lowestNote = 0;
    int m_highestNote = 0;

    double m_bpm = 120;
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