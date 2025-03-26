#pragma once

#include <JuceHeader.h>

struct MidiEvent
{
    MidiEvent()
    {
    }
    MidiEvent(const juce::MidiMessageSequence::MidiEventHolder& eventHolder, double bpm)
        : note(eventHolder.message.getNoteNumber()),
        ms(getMiliseconds(eventHolder.message.getTimeStamp(), bpm)),
        tickStart(eventHolder.message.getTimeStamp()),
        tickEnd(0)
    {
        if (eventHolder.noteOffObject != nullptr)
            tickEnd = eventHolder.noteOffObject->message.getTimeStamp();
        else
            tickEnd = tickStart;
    }

    inline static double getMiliseconds(double tickStart, double bpm)
    {
        double beatPosition = tickStart / 960;
        double beatSecondsLength = 60 / bpm;
        double midiSeconds = beatPosition * beatSecondsLength;
        return std::round(midiSeconds * 1000);
    }

    juce::String debugMidiEvent()
    {
        juce::String output;
        output += "note: " + juce::String(note);
        output += ", ms: " + juce::String(ms);
        output += ", tickStart: " + juce::String(tickStart);
        output += ", tickEnd: " + juce::String(tickEnd);
        return output;
    }

    int note = 0;
    double ms = 0;
    double tickStart = 0;
    double tickEnd = 0;
    int closestQuantizedIndex = -1;
};