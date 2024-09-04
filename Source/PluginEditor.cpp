/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#define NAME_OF(name) (#name)

//==============================================================================
TimeAnalyzerAudioProcessorEditor::TimeAnalyzerAudioProcessorEditor(TimeAnalyzerAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    addAndMakeVisible(m_midiDisplay);
    m_midiDisplay.setVisible(true);

    addAndMakeVisible(midiResults);
    midiResults.setReadOnly(true);
    midiResults.setMultiLine(true);
    midiResults.setText(audioProcessor.stateInfo.getProperty(NAME_OF(midiResults)));
    saveMidiResultsCallback = [&]()
    {
        audioProcessor.stateInfo.setProperty(NAME_OF(midiResults), midiResults.getText(), nullptr);
    };
    midiResults.onTextChange = saveMidiResultsCallback;
    midiResults.setVisible(false);

    currentFont = midiResults.getFont();
    addAndMakeVisible(fontSize_Slider);
    fontSize_Slider.setTextValueSuffix(" Font Size");
    fontSize_Slider.setRange(10, 50, .5);
    fontSize_Slider.onValueChange = [&]()
    {
        midiResults.onTextChange = nullptr;
        midiResults.setFont(currentFont.withHeight(fontSize_Slider.getValue()));

        juce::String resultsString = midiResults.getText();
        if (resultsString.isNotEmpty())
        {
            if (resultsString[resultsString.length() - 1] == '\n')
                midiResults.setText(resultsString.substring(0, resultsString.length() - 1));
            else
                midiResults.setText(resultsString + "\n");
        }

        midiResults.onTextChange = saveMidiResultsCallback;
    };
    fontSize_Slider.setValue(audioProcessor.stateInfo.getProperty(NAME_OF(fontSize_Slider)));
    fontSize_Slider.onDragEnd = [&]()
    {
        audioProcessor.stateInfo.setProperty(NAME_OF(fontSize_Slider), fontSize_Slider.getValue(), nullptr);
    };

    addAndMakeVisible(drumNotes_Toggle);
    drumNotes_Toggle.setToggleState(audioProcessor.stateInfo.getProperty(NAME_OF(drumNotes_Toggle)), true);
    drumNotes_Toggle.onClick = [&]()
    {
        audioProcessor.stateInfo.setProperty(NAME_OF(drumNotes_Toggle),
                                             drumNotes_Toggle.getToggleState(), nullptr);
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
    setQuantizedMidiFile_Button.onClick = [&]() { setQuantizedMidiFile(getNewMidiFile()); };

    addAndMakeVisible(refreshQuantizedMidi_Button);
    refreshQuantizedMidi_Button.onClick = [&]() { setQuantizedMidiFile(m_quantizedMidiFile); };

    addAndMakeVisible(analyzeMidiFile_Button);
    analyzeMidiFile_Button.onClick = [&]() { analyzeMidiFile(); };

    addAndMakeVisible(detectNewMidi_Toggle);
    detectNewMidi_Toggle.setToggleState(audioProcessor.stateInfo.getProperty(NAME_OF(detectNewMidi_Toggle)), true);
    detectNewMidi_Toggle.onClick = [&]()
    {
        audioProcessor.stateInfo.setProperty(NAME_OF(detectNewMidi_Toggle), detectNewMidi_Toggle.getToggleState(), nullptr);

        if (!detectNewMidi_Toggle.getToggleState())
            stopTimer();
        else if (!isTimerRunning())
            startTimer(1000);
    };
    if (detectNewMidi_Toggle.getToggleState())
        startTimer(1000);

    setResizable(true, true);

    if (audioProcessor.stateInfo.getProperty("width") && audioProcessor.stateInfo.getProperty("height"))
        setSize(audioProcessor.stateInfo.getProperty("width"), audioProcessor.stateInfo.getProperty("height"));
    else
        setSize(500, 500);


    //load quantized midi
    for (auto midi : audioProcessor.stateInfo.getChildWithName(NAME_OF(quantizedMidi)))
    {
        quantizedMidi.add(MidiEvent(midi.getProperty(NAME_OF(MidiEvent::note)), 
                                    midi.getProperty(NAME_OF(MidiEvent::ms)),
                                    midi.getProperty(NAME_OF(MidiEvent::tickStart)),
                                    midi.getProperty(NAME_OF(MidiEvent::tickEnd))
                                    ));
    }

    if (!quantizedMidi.isEmpty())
        m_midiDisplay.setQuantizedMidi(quantizedMidi, audioProcessor.getPlayHead());
}

TimeAnalyzerAudioProcessorEditor::~TimeAnalyzerAudioProcessorEditor()
{
}

//==============================================================================
void TimeAnalyzerAudioProcessorEditor::paint(juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

}

void TimeAnalyzerAudioProcessorEditor::resized()
{
    audioProcessor.stateInfo.setProperty("width", getWidth(), nullptr);
    audioProcessor.stateInfo.setProperty("height", getHeight(), nullptr);

    auto bounds = getLocalBounds();

    {
        auto tempoBounds = bounds.removeFromBottom(30);
        setQuantizedMidiFile_Button.setBounds(tempoBounds.withRight(getWidth() / 3));
        analyzeMidiFile_Button.setBounds(tempoBounds.withLeft(setQuantizedMidiFile_Button.getRight()).withWidth(getWidth() / 3));
        detectNewMidi_Toggle.setBounds(tempoBounds.withLeft(analyzeMidiFile_Button.getRight()).withWidth(getWidth() / 3));
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
        drumNotes_Toggle.setBounds(tempoBounds.withRight(getWidth() / 2));
        fontSize_Slider.setBounds(tempoBounds.withLeft(getWidth() / 2));
    }
    m_midiDisplay.setBounds(bounds);
    midiResults.setBounds(bounds);
}

void TimeAnalyzerAudioProcessorEditor::timerCallback()
{
    if (!m_quantizedMidiFile.exists() && quantizedMidi.isEmpty())
        return;

    if (audioProcessor.getPlayHead()->getPosition()->getIsRecording())
        return; //the host might be recording the newest midi file

    if (quantizedMidi.isEmpty())
    {
        midiResults.setText("Please Set a Quantized Midi File");
        return;
    }

    juce::File newMidiFile = getNewMidiFile();
    if (newMidiFile == newestMidiFile || newMidiFile == m_quantizedMidiFile)
    {
        return; //no new midi files
    }

    newestMidiFile = newMidiFile;
    analyzeMidiFile();
}

void TimeAnalyzerAudioProcessorEditor::setQuantizedMidiFile(juce::File quantizedMidiFile)
{
    playHeadTempo.setText(juce::String(audioProcessor.playHeadBpm));

    quantizedMidi.clear();
    if (!readMidiFile(quantizedMidiFile, quantizedMidi))
    {
        midiResults.setText("No Midi Files Detected");
        return;
    }
    m_quantizedMidiFile = quantizedMidiFile;

    juce::String results = "Quantized Midi Info: \n";
    for (const MidiEvent& midi : quantizedMidi)
    {
        results += "Note: " + getMidiNoteName(midi.note)
            + ", num: " + juce::String(midi.note)
            + ", ms: " + juce::String(midi.ms) + juce::newLine;
    }
    midiResults.setText(results);

    m_midiDisplay.setQuantizedMidi(quantizedMidi, audioProcessor.getPlayHead());


    //save quantized midi into state info
    juce::ValueTree quantizedMidiTree(NAME_OF(quantizedMidi));
    for (const MidiEvent& midi : quantizedMidi)
    {
        juce::ValueTree midiValue(NAME_OF(MidiEvent));
        midiValue.setProperty(NAME_OF(MidiEvent::note), midi.note, nullptr);
        midiValue.setProperty(NAME_OF(MidiEvent::ms), midi.ms, nullptr);
        midiValue.setProperty(NAME_OF(MidiEvent::tickStart), midi.tickStart, nullptr);
        midiValue.setProperty(NAME_OF(MidiEvent::tickEnd), midi.tickEnd, nullptr);
        quantizedMidiTree.appendChild(midiValue, nullptr);
    }

    //clear existing quantized midi trees
    while (audioProcessor.stateInfo.getChildWithName(NAME_OF(quantizedMidi)).isValid())
        audioProcessor.stateInfo.removeChild(audioProcessor.stateInfo.getChildWithName(NAME_OF(quantizedMidi)), nullptr);

    audioProcessor.stateInfo.appendChild(quantizedMidiTree, nullptr); //save
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
    for (const MidiEvent& midi : midiToAnalyze)
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

    m_midiDisplay.setAnalyzedMidi(midiToAnalyze);
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
    juce::FileInputStream inputStream(midiFile);
    if (inputStream.failedToOpen())
        return false;
    if (!midi.readFrom(inputStream))
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
    if (drumNotes_Toggle.getToggleState())
    {
        switch (note)
        {
            case 38:
                return "Snare";
            case 40:
                return "Snare Rim";
            case 47:
                return "Crash Cymbal 2";
            case 58:
                return "Low Floor Tom";
            case 23:
                return "Mid-Open Hi-Hat";
            default:
                juce::String rhythmName = juce::MidiMessage::getRhythmInstrumentName(note);
                if (rhythmName.isNotEmpty())
                    return rhythmName;
        }
    }

    return juce::MidiMessage::getMidiNoteName(note, true, true, 4);
}

void TimeAnalyzerAudioProcessorEditor::debugTree(juce::ValueTree& tree)
{
    for (int i = 0; i < tree.getNumProperties(); i++)
    {
        juce::String propertyID = tree.getPropertyName(i).toString();
    }

    for (auto child : tree)
    {
        juce::String childID = child.getType().toString();
        if (childID != NAME_OF(MidiEvent))
            int stop = 1;
        else
        {
            double debugTickStart = child.getProperty(NAME_OF(MidiEvent::tickStart));
            double debugTickEnd = child.getProperty(NAME_OF(MidiEvent::tickEnd));
        }
        debugTree(child);
    }
}
