#pragma once

#include <JuceHeader.h>

struct MidiEvent
{
    MidiEvent() : MidiEvent(0, 0, 0, 0, 0)
    {
    }
    MidiEvent(int note, double ms, double tickStart, double tickEnd, double msDifference)
        : note(note), ms(ms), tickStart(tickStart), tickEnd(tickEnd), msDifference(msDifference)
    {
    }
    MidiEvent(const juce::MidiMessageSequence::MidiEventHolder& eventHolder, double bpm)
        : note(eventHolder.message.getNoteNumber()),
        ms(getMiliseconds(eventHolder.message.getTimeStamp(), bpm)),
        tickStart(eventHolder.message.getTimeStamp()),
        tickEnd(0),
        msDifference(0)
    {
        if (eventHolder.noteOffObject != nullptr)
            tickEnd = eventHolder.noteOffObject->message.getTimeStamp();
        else
            tickEnd = tickStart;
    }

    double getMiliseconds(double tickStart, double bpm)
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
        output += ", msDifference: " + juce::String(msDifference);
        return output;
    }

    int note;
    double ms;
    double tickStart;
    double tickEnd;
    double msDifference;
};