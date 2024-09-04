/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "MidiEvent.h"
#include "MidiDisplay.h"

//==============================================================================
/**
*/
class TimeAnalyzerAudioProcessorEditor  : public juce::AudioProcessorEditor, juce::Timer
{
public:
    TimeAnalyzerAudioProcessorEditor (TimeAnalyzerAudioProcessor&);
    ~TimeAnalyzerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

    //==============================================================================
    void setQuantizedMidiFile();
    void analyzeMidiFile();

    juce::File getNewMidiFile();
    bool readMidiFile(juce::File midiFile, juce::Array<MidiEvent>& out);

    juce::String getMidiNoteName(juce::MidiMessage message);
    juce::String getMidiNoteName(int note);

    //==============================================================================

private:
    //==============================================================================
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    TimeAnalyzerAudioProcessor& audioProcessor;

    //==============================================================================
    MidiDisplay m_midiDisplay;
    juce::TextEditor midiResults;
    std::function<void()> saveMidiResultsCallback;

    juce::ToggleButton drumNotes_Toggle{ "Is Drums?" };
    juce::Slider fontSize_Slider;
    juce::Font currentFont;

    juce::TextEditor playHeadTempo;
    juce::ToggleButton editTempo_Toggle{ "Edit Tempo" };
    juce::TextEditor tempo_Editor;

    juce::TextEditor midiDirectory_Editor;

    juce::TextButton setQuantizedMidiFile_Button{ "Set Quantized Midi File" };
    juce::TextButton analyzeMidiFile_Button{ "Analyze Midi File" };
    juce::ToggleButton detectNewMidi_Toggle{ "Detect New Midi to Analyze" };

    //==============================================================================
    juce::Array<MidiEvent> quantizedMidi;
    juce::File quantizedMidiFile;
    juce::File newestMidiFile;

    //==============================================================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TimeAnalyzerAudioProcessorEditor)
};
