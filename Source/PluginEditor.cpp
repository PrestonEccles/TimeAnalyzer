/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TimeAnalyzerAudioProcessorEditor::TimeAnalyzerAudioProcessorEditor (TimeAnalyzerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    addAndMakeVisible(readMidiFile_Button);
    readMidiFile_Button.onClick = [&]() { getMidiFile(); };

    addAndMakeVisible(midiDirectory_Editor);
    midiDirectory_Editor.setText(audioProcessor.stateInfo.getProperty("midiDirectory"), false);
    midiDirectory_Editor.onTextChange = [&]() 
    {
        audioProcessor.stateInfo.setProperty("midiDirectory", midiDirectory_Editor.getText(), nullptr);
        setOldMidiFiles();
    };

    addAndMakeVisible(playHeadTempo);
    playHeadTempo.setReadOnly(true);

    addAndMakeVisible(editTempo_Toggle);
    editTempo_Toggle.setToggleState(audioProcessor.stateInfo.getProperty("editTempoToggle"), false);
    editTempo_Toggle.onClick = [&]()
    {
        audioProcessor.stateInfo.setProperty("editTempoToggle", editTempo_Toggle.getToggleState(), nullptr);
        tempo_Editor.setVisible(editTempo_Toggle.getToggleState());
    };

    addAndMakeVisible(tempo_Editor);
    tempo_Editor.setVisible(editTempo_Toggle.getToggleState());
    tempo_Editor.setText(audioProcessor.stateInfo.getProperty("editedTempo"));
    tempo_Editor.onTextChange = [&]()
    {
        audioProcessor.stateInfo.setProperty("editedTempo", tempo_Editor.getText(), nullptr);
    };

    addAndMakeVisible(midiResults);
    midiResults.setReadOnly(true);
    midiResults.setMultiLine(true);

    setSize (400, 300);
}

TimeAnalyzerAudioProcessorEditor::~TimeAnalyzerAudioProcessorEditor()
{
}

//==============================================================================
void TimeAnalyzerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

}

void TimeAnalyzerAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    readMidiFile_Button.setBounds(bounds.removeFromBottom(30));
    midiDirectory_Editor.setBounds(bounds.removeFromBottom(30));
    {
        auto tempoBounds = bounds.removeFromBottom(30);
        playHeadTempo.setBounds(tempoBounds.withSize(100, 25));
        editTempo_Toggle.setBounds(tempoBounds.withLeft(playHeadTempo.getRight()).withSize(100, 25));
        tempo_Editor.setBounds(tempoBounds.withLeft(editTempo_Toggle.getRight()).withSize(100, 25));
    }

    midiResults.setBounds(bounds);
}

void TimeAnalyzerAudioProcessorEditor::getMidiFile()
{
    setNewMidiFile();

    juce::MidiFile midi;
    juce::File readFile(currentMidiFile);
    juce::FileInputStream inputStream(readFile);
    if (inputStream.openedOk() && midi.readFrom(inputStream))
    {
        playHeadTempo.setText(juce::String(audioProcessor.playHeadBpm));

        double currentBpm = 1;
        if (editTempo_Toggle.getToggleState())
        {
            currentBpm = tempo_Editor.getText().getDoubleValue();
        }
        else
        {
            currentBpm = audioProcessor.playHeadBpm;
        }

        std::vector<const juce::MidiMessageSequence*> sequences;
        for (int i = 0; i < midi.getNumTracks(); i++)
        {
            sequences.push_back(midi.getTrack(i));
        }
        std::vector<juce::MidiMessageSequence::MidiEventHolder*> eventHolders;
        for (auto sequence : sequences)
        {
            for (int i = 0; i < sequence->getNumEvents(); i++)
            {
                eventHolders.push_back(sequence->getEventPointer(i));
            }
        }
        juce::String resultsString;
        for (auto eventHolder : eventHolders)
        {
            if (eventHolder->message.isNoteOn())
            {
                resultsString += juce::String() 
                    + "Note: " + juce::String(eventHolder->message.getNoteNumber())
                    + ", TimeStamp: " + juce::String(eventHolder->message.getTimeStamp())
                    + ", ms: " + juce::String(eventHolder->message.getTimeStamp() / 960 * 60 / currentBpm * 1000)
                    + juce::newLine;
            }
            else if (eventHolder->message.isTempoMetaEvent())
            {
                resultsString += juce::String()
                    + "getTempoSecondsPerQuarterNote: " + juce::String(eventHolder->message.getTempoSecondsPerQuarterNote())
                    + ", getTempoMetaEventTickLength: " + juce::String(eventHolder->message.getTempoMetaEventTickLength(0)) 
                    + juce::newLine;
            }
        }
        midiResults.setText(resultsString);
    }
    else
        DBG("could not read file \"" << readFile.getFullPathName() << "\"");
}

void TimeAnalyzerAudioProcessorEditor::setOldMidiFiles()
{
    juce::File newMidiDirectory(midiDirectory_Editor.getText().unquoted());
    if (!newMidiDirectory.exists() || !newMidiDirectory.isDirectory())
        return;

    oldMidiFiles.clear();

    for (juce::File childFile : newMidiDirectory.findChildFiles(juce::File::TypesOfFileToFind::findFiles, false))
    {
        if (childFile.getFileExtension() == ".mid") //is midi file?
        {
            oldMidiFiles.add(childFile);
        }
    }
}

void TimeAnalyzerAudioProcessorEditor::setNewMidiFile()
{
    juce::File newMidiDirectory(midiDirectory_Editor.getText().unquoted());
    if (!newMidiDirectory.exists() || !newMidiDirectory.isDirectory())
        return;

    for (juce::File childFile : newMidiDirectory.findChildFiles(juce::File::TypesOfFileToFind::findFiles, false))
    {
        if (childFile.getFileExtension() == ".mid") //is midi file?
        {
            bool isNewFile = true;
            for (juce::File oldFile : oldMidiFiles)
            {
                if (oldFile == childFile)
                {
                    isNewFile = false;
                    break;
                }
            }
            if (isNewFile)
            {
                setOldMidiFiles();
                currentMidiFile = childFile;
            }
        }
    }
}
