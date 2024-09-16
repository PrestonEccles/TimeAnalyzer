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
    void setQuantizedMidiFile(juce::File quantizedMidiFile);
    void analyzeMidiFile();

    juce::File getNewMidiFile();
    bool getMidiFile(juce::File fileOfMidi, juce::MidiFile& out);
    void readMidiFile(juce::MidiFile midiFile, juce::Array<MidiEvent>& out);

    juce::String getMidiNoteName(juce::MidiMessage message);
    juce::String getMidiNoteName(int note);

    void debugTree(juce::ValueTree& tree);

    //==============================================================================
    void loadStateInfo();

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
    juce::TextButton msTimeThreshold_Title{ "Time Threshold (ms):" };
    juce::TextEditor msTimeThreshold_Editor;
    juce::Slider fontSize_Slider;
    juce::Font currentFont;

    juce::TextEditor playHeadTempo;
    juce::ToggleButton editTempo_Toggle{ "Edit Tempo" };
    juce::TextEditor tempo_Editor;
    juce::TextButton measureStart_Title{ "Measure Start:" };
    juce::TextEditor measureStart_Editor;
    juce::TextButton measureRangeLength_Title{ "Measure Range:" };
    juce::TextEditor measureRangeLength_Editor;

    juce::TextEditor midiDirectory_Editor;
    juce::ToggleButton detectNewMidi_Toggle{ "Detect New Midi to Analyze" };
    juce::TextButton detectNewMidiFrequency_Title{ "Frequency (ms):" };
    juce::TextEditor detectNewMidiFrequency_Editor;
    juce::TextEditor detectNewMidiLog;
    int m_msDetectNewMidiFrequency;

    juce::TextButton setQuantizedMidiFile_Button{ "Set Quantized Midi File" };
    juce::TextButton refreshQuantizedMidi_Button{ "Refresh Quantized Midi" };
    juce::TextButton analyzeMidiFile_Button{ "Analyze Midi File" };

    //==============================================================================
    juce::Array<MidiEvent> quantizedMidi;
    juce::File m_quantizedMidiFile;
    juce::File newestMidiFile;
    juce::MidiFile midiFileToAnalyze;

    //==============================================================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TimeAnalyzerAudioProcessorEditor)
};
