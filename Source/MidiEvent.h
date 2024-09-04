/*
  ==============================================================================

    MidiEvent.h
    Created: 3 Sep 2024 6:18:13pm
    Author:  prest

  ==============================================================================
*/

#pragma once

struct MidiEvent
{
    MidiEvent() : MidiEvent(0, 0, 0, 0)
    {
    }
    MidiEvent(int note, double ms, double tickStart, double tickEnd) 
        : note(note), ms(ms), tickStart(tickStart), tickEnd(tickEnd)
    {
    }
    MidiEvent(const juce::MidiMessageSequence::MidiEventHolder& eventHolder, double bpm)
        : note(eventHolder.message.getNoteNumber()),
        ms(getMiliseconds(eventHolder.message.getTimeStamp(), bpm)),
        tickStart(eventHolder.message.getTimeStamp()),
        tickEnd(eventHolder.noteOffObject->message.getTimeStamp())
    {
    }

    double getMiliseconds(double tickStart, double bpm)
    {
        double beatPosition = tickStart / 960;
        double beatSecondsLength = 60 / bpm;
        double midiSeconds = beatPosition * beatSecondsLength;
        return std::round(midiSeconds * 1000);
    }

    int note;
    double ms;
    double tickStart;
    double tickEnd;
};