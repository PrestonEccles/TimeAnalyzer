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
    MidiEvent() : note(0), ms(0), ticks()
    {
    }
    MidiEvent(int note, double ms, double ticks) : note(note), ms(ms), ticks(ticks)
    {
    }
    MidiEvent(const juce::MidiMessageSequence::MidiEventHolder& eventHolder, double bpm)
        : note(eventHolder.message.getNoteNumber()),
        ms(getMiliseconds(eventHolder.message.getTimeStamp(), bpm)), ticks(eventHolder.message.getTimeStamp())
    {
    }

    double getMiliseconds(double ticks, double bpm)
    {
        double beatPosition = ticks / 960;
        double beatSecondsLength = 60 / bpm;
        double midiSeconds = beatPosition * beatSecondsLength;
        return std::round(midiSeconds * 1000);
    }

    int note;
    double ms;
    double ticks;
};