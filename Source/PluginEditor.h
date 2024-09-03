/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class TimeAnalyzerAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    TimeAnalyzerAudioProcessorEditor (TimeAnalyzerAudioProcessor&);
    ~TimeAnalyzerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    //==============================================================================
    struct MidiEvent
    {
        MidiEvent(int noteNumber, double ms) : noteNumber(noteNumber), ms(ms) {}
        MidiEvent(const juce::MidiMessageSequence::MidiEventHolder& eventHolder, double bpm)
            : noteNumber(eventHolder.message.getNoteNumber()), ms(getMiliseconds(eventHolder, bpm)) {}

        double getMiliseconds(const juce::MidiMessageSequence::MidiEventHolder& eventHolder, double bpm)
        {
            double beatPosition = eventHolder.message.getTimeStamp() / 960;
            double beatSecondsLength = 60 / bpm;
            double midiSeconds = beatPosition * beatSecondsLength;
            return std::floor(midiSeconds * 1000);
        }

        int noteNumber;
        double ms;
    };

    void setQuantizedMidiFile();
    void analyzeMidiFile();

    juce::File getNewMidiFile();
    bool readMidiFile(juce::File midiFile, juce::Array<MidiEvent>& out);

    juce::String getMidiNoteName(juce::MidiMessage message);
    juce::String getMidiNoteName(int noteNumber);

    //==============================================================================

private:
    //==============================================================================
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    TimeAnalyzerAudioProcessor& audioProcessor;

    //==============================================================================

    juce::TextEditor midiResults;

    juce::ToggleButton rhythmInstrument_Toggle{ "Is Rhythm Instrument" };
    juce::TextEditor playHeadTempo;
    juce::ToggleButton editTempo_Toggle{ "Edit Tempo" };
    juce::TextEditor tempo_Editor;

    juce::TextEditor midiDirectory_Editor;

    juce::TextButton setQuantizedMidiFile_Button{ "Set Quantized Midi File" };
    juce::TextButton analyzeMidiFile_Button{ "Analyze Midi File" };

    //==============================================================================
    juce::Array<MidiEvent> quantizedMidi;

    //==============================================================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TimeAnalyzerAudioProcessorEditor)
};
