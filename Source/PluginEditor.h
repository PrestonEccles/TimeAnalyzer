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
    void getMidiFile();
    void setOldMidiFiles();
    void setNewMidiFile();

    //==============================================================================

private:
    //==============================================================================
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    TimeAnalyzerAudioProcessor& audioProcessor;

    //==============================================================================
    juce::TextButton readMidiFile_Button{ "Read Midi File" };
    juce::TextEditor midiDirectory_Editor;
    juce::ToggleButton editTempo_Toggle{ "Edit Tempo" };
    juce::TextEditor tempo_Editor;
    juce::TextEditor playHeadTempo;
    juce::TextEditor midiResults;

    //==============================================================================
    juce::Array<juce::File> oldMidiFiles;
    juce::File currentMidiFile;

    //==============================================================================
    struct StateData
    {
        StateData() : midiDirectory(""), currentMidiFileName(""), editTempo(false)
        {
        }

        void loadState(juce::MemoryBlock newState)
        {
            if (newState.isEmpty())
                return;

            size_t readPosition = -1;
            size_t newStateSize = newState.getSize();


            juce::String newStateVersion;
            while (newState[++readPosition] != 0)
                newStateVersion += newState[readPosition];

            if (newStateVersion != stateVersion)
                return; //state data not compatible


            while (newState[++readPosition] != 0)
                midiDirectory += newState[readPosition];

            while (newState[++readPosition] != 0)
                currentMidiFileName += newState[readPosition];

            editTempo = newState[readPosition++];
        }

        juce::Array<char> getStateData()
        {
            juce::Array<char> saveState;
            appendStringToData(stateVersion, saveState);
            appendStringToData(midiDirectory, saveState);
            appendStringToData(currentMidiFileName, saveState);
            appendByteToData(editTempo, saveState);
            
            return saveState;
        }
        
        //increment when making code changes to the data being saved
        const juce::String stateVersion = "0";

        juce::String midiDirectory;
        //includes extension
        juce::String currentMidiFileName;
        bool editTempo;

    private:
        void appendStringToData(const juce::String& newString, juce::Array<char>& data)
        {
            juce::CharPointer_UTF8 stringReader(newString.getCharPointer());
            while (stringReader.isNotEmpty())
                data.add(*stringReader++);

            data.add(0); //null termination
        }
        void appendByteToData(const char& byte, juce::Array<char>& data)
        {
            data.add(byte);
            data.add(0); //null termination
        }
    };
    StateData currentStateData;

    juce::String midiDirectory;

    //==============================================================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TimeAnalyzerAudioProcessorEditor)
};
