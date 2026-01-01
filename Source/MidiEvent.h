#pragma once

#include "Globals.h"

struct MidiEvent
{
    MidiEvent()
    {
    }
    MidiEvent(const juce::MidiMessageSequence::MidiEventHolder& eventHolder, double bpm, int quarterNoteTicks)
        : note(eventHolder.message.getNoteNumber()),
        debugMS(getMiliseconds(eventHolder.message.getTimeStamp(), bpm, quarterNoteTicks)),
        tickStart(eventHolder.message.getTimeStamp()),
        tickEnd(0),
        quarterNoteTicks(quarterNoteTicks)
    {
        if (eventHolder.noteOffObject != nullptr)
            tickEnd = eventHolder.noteOffObject->message.getTimeStamp();
        else
            tickEnd = tickStart;
    }
    MidiEvent(double ms, double bpm, bool useQuantizedNote = true, int note = 0)
        : note(note), useQuantizedNote(useQuantizedNote), debugMS(ms), tickStart(getTick(ms, bpm, g_defaultQuarterNoteTicks)), tickEnd(tickStart)
    {
    }
    

    inline static double getMiliseconds(double tick, double bpm, int quarterNoteTicks)
    {
        double beatPosition = tick / quarterNoteTicks;
        double beatSecondsLength = 60 / bpm;
        double midiSeconds = beatPosition * beatSecondsLength;
        return std::round(midiSeconds * 1000);
    }

    inline static double getTick(double ms, double bpm, int quarterNoteTicks)
    {
        double seconds = ms / 1000;
        double beatsPerSecond = bpm / 60;
        double beatPosition = seconds * beatsPerSecond;
        return beatPosition * quarterNoteTicks;
    }

    double getTickStart(int newQuarterNoteTicks) { return tickStart / quarterNoteTicks * newQuarterNoteTicks; }

    juce::String debugMidiEvent()
    {
        juce::String output;
        output += "note: " + juce::String(note);
        output += ", ms: " + juce::String(debugMS);
        output += ", tickStart: " + juce::String(tickStart);
        output += ", tickEnd: " + juce::String(tickEnd);
        output += ", quarterNoteTicks: " + juce::String(quarterNoteTicks);
        return output;
    }

    int note = 0;
    bool useQuantizedNote = false;
    double debugMS = 0;
    double tickStart = 0;
    double tickEnd = 0;
    int quarterNoteTicks = g_defaultQuarterNoteTicks;
    int closestQuantizedIndex = -1;
};