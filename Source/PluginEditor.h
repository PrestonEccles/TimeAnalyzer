/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "Globals.h"
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

    bool keyPressed(const juce::KeyPress& key) override;

    //==============================================================================
    void timerCallback() override;

    void setQuantizedMidiFile(juce::File quantizedMidiFile);
    void analyzeFile();
    void analyzeFile(juce::File fileToAnalyze);

    //get midi or audio file
    juce::File getNewFile(bool midiFile = true);

    bool canReadMidiFile(juce::File fileOfMidi);
    bool getMidiFile(juce::File fileOfMidi, juce::MidiFile& out);
    void readMidiFile(juce::MidiFile midiFile, juce::Array<MidiEvent>& out);

    bool canReadAudioFile(juce::File audioFile);
    void readAudioFile(juce::File audioFile, juce::Array<MidiEvent>& out);

    const int maxAudioFileMinuteLength = 10;

    juce::String getMidiNoteName(juce::MidiMessage message);
    juce::String getMidiNoteName(int note);

    void setPlayHeadInfo();

    inline static juce::TextEditor* g_debug_Display;
    inline static void debugLog(juce::String log) 
    { 
        if (g_debug_Display->isVisible()) 
            g_debug_Display->setText(g_debug_Display->getText() + log + "\n"); 
    }

    void debugTree(juce::ValueTree& tree);
    void debugPlugin(juce::String callFrom);

    //==============================================================================
    void loadStateInfo();
    int loadStateCount = 0;

    //==============================================================================

private:
    //==============================================================================
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    TimeAnalyzerAudioProcessor& audioProcessor;

    //==============================================================================
    //UI:

    void initializeUI();
    void resized() override;
    void paint(juce::Graphics& g) override;

    MidiDisplay m_midiDisplay;
    std::function<void()> saveMidiResultsCallback;

    juce::ToggleButton debug_Toggle{ "Debug" };
    juce::TextButton debugClear_Button{ "Clear" };
    juce::TextButton debugRefresh_Button{ "Refresh" };
    juce::TextEditor debug_Display;

    juce::TextButton msTimeThreshold_Title{ "Time Threshold (ms):" };
    juce::TextEditor msTimeThreshold_Editor;

    juce::TextButton playHeadTempo_Title{ "Tempo:" };
    juce::TextEditor playHeadTempo;
    juce::ToggleButton editTempo_Toggle{ "Edit Tempo" };
    juce::TextEditor tempo_Editor;
    juce::TextButton recordStartMeasure_Title{ "Record Start:" };
    juce::TextEditor recordStartMeasure_Editor;
    juce::TextButton measureStart_Title{ "Measure Start:" };
    juce::TextEditor measureStart_Editor;
    juce::TextButton measureStartIncrement{ "+" };
    juce::TextButton measureStartDecrement{ "-" };
    juce::ToggleButton lockAnalyzedMidi_Toggle{ "Lock Analyzed Midi" };
    int m_previousRecordStart = 0;
    int m_previousMeasureStart = 0;
    juce::TextButton measureRangeLength_Title{ "Measure Range:" };
    juce::TextEditor measureRangeLength_Editor;

    juce::TextButton midiDirectory_Title{ "Midi Folder Path:" };
    juce::TextEditor midiDirectory_Editor;
    juce::ToggleButton detectNewMidi_Toggle{ "Detect New Midi to Analyze" };
    juce::TextButton detectNewMidiFrequency_Title{ "Frequency (ms):" };
    juce::TextEditor detectNewMidiFrequency_Editor;
    juce::TextEditor detectNewMidiLog;
    int m_msDetectNewMidiFrequency;

    juce::TextButton setQuantizedMidiFile_Button{ "Set Quantized Midi File" };
    juce::TextButton refreshQuantizedMidi_Button{ "Refresh Quantized Midi" };
    juce::TextButton analyzeMidiFile_Button{ "Analyze Midi File" };

    juce::ToggleButton analyzeAudioFiles_Toggle{ "Analyze Audio Files" };
    juce::TextButton audioDBThreshold_Title{ "dB Threshold:" };
    juce::Slider audioDBThreshold_Slider;
    juce::TextButton audioHitDistance_Title{ "Hit Distance (ms):" };
    juce::TextEditor audioHitDistance_Editor;

    //==============================================================================
    juce::Array<MidiEvent> quantizedMidi;
    juce::File m_quantizedMidiFile;
    juce::File newestFile;
    juce::int64 newestFileSize = 0;
    juce::MidiFile midiFileToAnalyze;
    juce::File audioFileToAnalyze;

    //==============================================================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TimeAnalyzerAudioProcessorEditor)
};
