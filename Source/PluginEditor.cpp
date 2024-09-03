/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#define NAME_OF(name) (#name)

//==============================================================================
TimeAnalyzerAudioProcessorEditor::TimeAnalyzerAudioProcessorEditor (TimeAnalyzerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    addAndMakeVisible(midiResults);
    midiResults.setReadOnly(true);
    midiResults.setMultiLine(true);
    midiResults.setText(audioProcessor.stateInfo.getProperty(NAME_OF(midiResults)));
    saveMidiResultsCallback = [&]()
    {
        audioProcessor.stateInfo.setProperty(NAME_OF(midiResults), midiResults.getText(), nullptr);
    };
    midiResults.onTextChange = saveMidiResultsCallback;

    currentFont = midiResults.getFont();
    addAndMakeVisible(fontSize_Slider);
    fontSize_Slider.setTextValueSuffix(" Font Size");
    fontSize_Slider.setRange(10, 120, 1);
    fontSize_Slider.onValueChange = [&]()
    {
        midiResults.onTextChange = nullptr;
        midiResults.setFont(currentFont.withHeight(fontSize_Slider.getValue()));

        juce::String resultsString = midiResults.getText();
        if (resultsString[resultsString.length() - 1] == '\n')
            midiResults.setText(resultsString.substring(0, resultsString.length() - 1));
        else
            midiResults.setText(resultsString + "\n");

        midiResults.onTextChange = saveMidiResultsCallback;
    };
    fontSize_Slider.setValue(audioProcessor.stateInfo.getProperty(NAME_OF(fontSize_Slider)));
    fontSize_Slider.onDragEnd = [&]()
    {
        audioProcessor.stateInfo.setProperty(NAME_OF(fontSize_Slider), fontSize_Slider.getValue(), nullptr);
    };

    addAndMakeVisible(rhythmInstrument_Toggle);
    rhythmInstrument_Toggle.setToggleState(audioProcessor.stateInfo.getProperty(NAME_OF(rhythmInstrument_Toggle)), true);
    rhythmInstrument_Toggle.onClick = [&]()
    {
        audioProcessor.stateInfo.setProperty(NAME_OF(rhythmInstrument_Toggle), 
                                             rhythmInstrument_Toggle.getToggleState(), nullptr);
    };

    addAndMakeVisible(playHeadTempo);
    playHeadTempo.setReadOnly(true);

    addAndMakeVisible(editTempo_Toggle);
    editTempo_Toggle.setToggleState(audioProcessor.stateInfo.getProperty(NAME_OF(editTempo_Toggle)), true);
    editTempo_Toggle.onClick = [&]()
    {
        audioProcessor.stateInfo.setProperty(NAME_OF(editTempo_Toggle), editTempo_Toggle.getToggleState(), nullptr);
        tempo_Editor.setVisible(editTempo_Toggle.getToggleState());
    };

    addAndMakeVisible(tempo_Editor);
    tempo_Editor.setVisible(editTempo_Toggle.getToggleState());
    tempo_Editor.setText(audioProcessor.stateInfo.getProperty(NAME_OF(tempo_Editor)));
    tempo_Editor.onTextChange = [&]()
    {
        audioProcessor.stateInfo.setProperty(NAME_OF(tempo_Editor), tempo_Editor.getText(), nullptr);
    };

    addAndMakeVisible(midiDirectory_Editor);
    midiDirectory_Editor.setSelectAllWhenFocused(true);
    midiDirectory_Editor.setText(audioProcessor.stateInfo.getProperty(NAME_OF(midiDirectory_Editor)));
    midiDirectory_Editor.onTextChange = [&]()
    {
        audioProcessor.stateInfo.setProperty(NAME_OF(midiDirectory_Editor), midiDirectory_Editor.getText(), nullptr);
    };

    addAndMakeVisible(setQuantizedMidiFile_Button);
    setQuantizedMidiFile_Button.onClick = [&]() { setQuantizedMidiFile(); };

    addAndMakeVisible(analyzeMidiFile_Button);
    analyzeMidiFile_Button.onClick = [&]() { analyzeMidiFile(); };

    setResizable(true, true);

    if (audioProcessor.stateInfo.getProperty("width") && audioProcessor.stateInfo.getProperty("height"))
        setSize(audioProcessor.stateInfo.getProperty("width"), audioProcessor.stateInfo.getProperty("height"));
    else
        setSize(500, 500);


    //load quantized midi
    for (auto midi : audioProcessor.stateInfo.getChildWithName(NAME_OF(quantizedMidi)))
    {
        quantizedMidi.add(MidiEvent(midi.getProperty(NAME_OF(MidiEvent::note)), midi.getProperty(NAME_OF(MidiEvent::ms))));
    }
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
    audioProcessor.stateInfo.setProperty("width", getWidth(), nullptr);
    audioProcessor.stateInfo.setProperty("height", getHeight(), nullptr);

    auto bounds = getLocalBounds();

    {
        auto tempoBounds = bounds.removeFromBottom(30);
        setQuantizedMidiFile_Button.setBounds(tempoBounds.withRight(getWidth() / 2));
        analyzeMidiFile_Button.setBounds(tempoBounds.withLeft(getWidth() / 2));
    }
    midiDirectory_Editor.setBounds(bounds.removeFromBottom(30));
    {
        auto tempoBounds = bounds.removeFromBottom(30);
        playHeadTempo.setBounds(tempoBounds.withSize(100, 25));
        editTempo_Toggle.setBounds(tempoBounds.withLeft(playHeadTempo.getRight()).withSize(100, 25));
        tempo_Editor.setBounds(tempoBounds.withLeft(editTempo_Toggle.getRight()).withSize(100, 25));
    }
    {
        auto tempoBounds = bounds.removeFromBottom(30);
        rhythmInstrument_Toggle.setBounds(tempoBounds.withRight(getWidth() / 2));
        fontSize_Slider.setBounds(tempoBounds.withLeft(getWidth() / 2));
    }

    midiResults.setBounds(bounds);
}

void TimeAnalyzerAudioProcessorEditor::setQuantizedMidiFile()
{
    playHeadTempo.setText(juce::String(audioProcessor.playHeadBpm));

    quantizedMidi.clear();
    if (!readMidiFile(getNewMidiFile(), quantizedMidi))
    {
        midiResults.setText("No Midi Files Detected");
        return;
    }

    juce::String results = "Quantized Midi Info: \n";
    for (auto midi : quantizedMidi)
    {
        results += "Note: " + getMidiNoteName(midi.note) + ", ms: " + juce::String(midi.ms) + juce::newLine;
    }
    midiResults.setText(results);

    //save midi into state info
    juce::ValueTree quantizedMidiTree(NAME_OF(quantizedMidi));
    for (auto midi : quantizedMidi)
    {
        juce::ValueTree midiValue(NAME_OF(MidiEvent));
        midiValue.setProperty(NAME_OF(MidiEvent::note), midi.note, nullptr);
        midiValue.setProperty(NAME_OF(MidiEvent::ms), midi.ms, nullptr);
        quantizedMidiTree.appendChild(midiValue, nullptr);
    }

    audioProcessor.stateInfo.appendChild(quantizedMidiTree, nullptr);
}

void TimeAnalyzerAudioProcessorEditor::analyzeMidiFile()
{
    if (quantizedMidi.isEmpty())
    {
        midiResults.setText("Please Set a Quantized Midi File");
        return;
    }

    playHeadTempo.setText(juce::String(audioProcessor.playHeadBpm));

    juce::Array<MidiEvent> midiToAnalyze;
    if (!readMidiFile(getNewMidiFile(), midiToAnalyze))
    {
        midiResults.setText("No Midi Files Detected");
        return;
    }

    juce::String results = "Analysis: \n";
    for (MidiEvent midi : midiToAnalyze)
    {
        double lowestDifference = midi.ms - quantizedMidi[0].ms;
        int closestQuantizedIndex = 0;
        for (int i = 1; i < quantizedMidi.size(); i++)
        {
            if (std::abs(midi.ms - quantizedMidi[i].ms) < std::abs(lowestDifference))
            {
                lowestDifference = midi.ms - quantizedMidi[i].ms;
                closestQuantizedIndex = i;
            }
        }

        results += "Note: " + getMidiNoteName(midi.note) + ", ms Diff: " + juce::String(lowestDifference) + juce::newLine;
    }
    midiResults.setText(results);
}

juce::File TimeAnalyzerAudioProcessorEditor::getNewMidiFile()
{
    juce::File newMidiDirectory(midiDirectory_Editor.getText().unquoted());
    if (!newMidiDirectory.exists() || !newMidiDirectory.isDirectory())
        return juce::File();

    juce::File newestMidiFile;
    for (juce::File childFile : newMidiDirectory.findChildFiles(juce::File::TypesOfFileToFind::findFiles, false))
    {
        if (childFile.getFileExtension() == ".mid") //is midi file?
        {
            if (!newestMidiFile.exists())
            {
                newestMidiFile = childFile; //initialize first file
                continue;
            }

            if (childFile.getCreationTime() > newestMidiFile.getCreationTime())
                newestMidiFile = childFile;
        }
    }
    return newestMidiFile;
}

bool TimeAnalyzerAudioProcessorEditor::readMidiFile(juce::File midiFile, juce::Array<MidiEvent>& out)
{
    if (!midiFile.exists())
        return false;

    double currentBpm;
    if (editTempo_Toggle.getToggleState())
        currentBpm = tempo_Editor.getText().getDoubleValue();
    else
        currentBpm = audioProcessor.playHeadBpm;

    juce::MidiFile midi;
    juce::File readFile(midiFile);
    juce::FileInputStream inputStream(readFile);
    if (inputStream.failedToOpen() || !midi.readFrom(inputStream))
        return false;

    for (int i = 0; i < midi.getNumTracks(); i++)
    {
        for (auto sequence : *midi.getTrack(i))
        {
            if (sequence->message.isNoteOn())
                out.add(MidiEvent(*sequence, currentBpm));
        }
    }

    return true;
}

juce::String TimeAnalyzerAudioProcessorEditor::getMidiNoteName(juce::MidiMessage message)
{
    return getMidiNoteName(message.getNoteNumber());
}

juce::String TimeAnalyzerAudioProcessorEditor::getMidiNoteName(int note)
{
    if (rhythmInstrument_Toggle.getToggleState())
        return juce::MidiMessage::getRhythmInstrumentName(note);

    return juce::MidiMessage::getMidiNoteName(note, true, true, 4);
}
