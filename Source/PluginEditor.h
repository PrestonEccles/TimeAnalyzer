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
    //analyzes "midiFileToAnalyze"
    void analyzeMidiFile();

    juce::File getNewMidiFile();
    bool getMidiFile(juce::File fileOfMidi, juce::MidiFile& out);
    void readMidiFile(juce::MidiFile midiFile, juce::Array<MidiEvent>& out);

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
    void debugPlugin(juce::String callFrom)
    {
        if (!debug_Toggle.getToggleState())
            return;

        juce::String debugText = "\n=========================================================\n";
        debugText += callFrom + ":\n\n";

        debugText += "loadStateCount: " + juce::String(loadStateCount) + "\n";

        debugText += m_midiDisplay.debugMidiDisplay() + "\n";

        debugText += "Play Head is Null: " + juce::String((int) (audioProcessor.getPlayHead() == nullptr)) + "\n";
        debugText += "audioProcessCount: " + juce::String(audioProcessor.audioProcessCount) + "\n";
        debugText += "\n";

        debugText += "msTimeThreshold_Editor: " + msTimeThreshold_Editor.getText() + "\n";
        debugText += "playHeadTempo: " + playHeadTempo.getText() + "\n";
        debugText += "tempo_Editor: " + tempo_Editor.getText() + "\n";
        debugText += "measureStart_Editor: " + measureStart_Editor.getText() + "\n";
        debugText += "measureRangeLength_Editor: " + measureRangeLength_Editor.getText() + "\n";
        debugText += "midiDirectory_Editor: " + midiDirectory_Editor.getText() + "\n";
        debugText += "detectNewMidiFrequency_Editor: " + detectNewMidiFrequency_Editor.getText() + "\n";
        debugText += "m_msDetectNewMidiFrequency: " + juce::String(m_msDetectNewMidiFrequency) + "\n\n";

        debugText += "quantizedMidi:\n";
        for (auto& m : quantizedMidi)
        {
            debugText += m.debugMidiEvent() + "\n";
        }
        debugText += "\n";

        debugText += "m_quantizedMidiFile: " + m_quantizedMidiFile.getFullPathName() + "\n";
        debugText += "newestMidiFile: " + newestMidiFile.getFullPathName() + "\n";

        debug_Display.setText(debug_Display.getText() + debugText);
    }

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
    MidiDisplay m_midiDisplay;
    std::function<void()> saveMidiResultsCallback;

    juce::ToggleButton debug_Toggle{ "Debug" };
    juce::TextButton debugClear_Button{ "Clear" };
    juce::TextButton debugRefresh_Button{ "Refresh" };
    juce::TextEditor debug_Display;

    juce::TextButton msTimeThreshold_Title{ "Time Threshold (ms):" };
    juce::TextEditor msTimeThreshold_Editor;

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
