#pragma once

#include "Globals.h"

struct MidiEvent
{
    MidiEvent()
    {
    }
    MidiEvent(const juce::MidiMessageSequence::MidiEventHolder& eventHolder, double bpm)
        : note(eventHolder.message.getNoteNumber()),
        debugMS(getMiliseconds(eventHolder.message.getTimeStamp(), bpm)),
        tickStart(eventHolder.message.getTimeStamp()),
        tickEnd(0)
    {
        if (eventHolder.noteOffObject != nullptr)
            tickEnd = eventHolder.noteOffObject->message.getTimeStamp();
        else
            tickEnd = tickStart;
    }
    MidiEvent(int note, double ms, double bpm)
        : note(note), debugMS(ms), tickStart(getTick(ms, bpm)), tickEnd(tickStart)
    {
    }
    

    inline static double getMiliseconds(double tick, double bpm)
    {
        double beatPosition = tick / g_quarterNoteTicks;
        double beatSecondsLength = 60 / bpm;
        double midiSeconds = beatPosition * beatSecondsLength;
        return std::round(midiSeconds * 1000);
    }

    inline static double getTick(double ms, double bpm)
    {
        double seconds = ms / 1000;
        double beatsPerSecond = bpm / 60;
        double beatPosition = seconds * beatsPerSecond;
        return beatPosition * g_quarterNoteTicks;
    }

    juce::String debugMidiEvent()
    {
        juce::String output;
        output += "note: " + juce::String(note);
        output += ", ms: " + juce::String(debugMS);
        output += ", tickStart: " + juce::String(tickStart);
        output += ", tickEnd: " + juce::String(tickEnd);
        return output;
    }

    int note = 0;
    double debugMS = 0;
    double tickStart = 0;
    double tickEnd = 0;
    int closestQuantizedIndex = -1;
};