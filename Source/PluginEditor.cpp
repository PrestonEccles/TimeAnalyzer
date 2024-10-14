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
    : AudioProcessorEditor(&p), audioProcessor(p), m_msDetectNewMidiFrequency(1000)
{
    addAndMakeVisible(m_midiDisplay);
    m_midiDisplay.setVisible(true);

    addAndMakeVisible(midiResults);
    midiResults.setReadOnly(true);
    midiResults.setMultiLine(true);
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
    fontSize_Slider.onDragEnd = [&]()
    {
        audioProcessor.stateInfo.setProperty(NAME_OF(fontSize_Slider), fontSize_Slider.getValue(), nullptr);
    };

    addAndMakeVisible(drumNotes_Toggle);
    drumNotes_Toggle.onClick = [&]()
    {
        audioProcessor.stateInfo.setProperty(NAME_OF(drumNotes_Toggle),
                                             drumNotes_Toggle.getToggleState(), nullptr);
    };

    addAndMakeVisible(msTimeThreshold_Title);
    addAndMakeVisible(msTimeThreshold_Editor);
    msTimeThreshold_Editor.setSelectAllWhenFocused(true);
    msTimeThreshold_Editor.onTextChange = [&]()
    {
        audioProcessor.stateInfo.setProperty(NAME_OF(msTimeThreshold_Editor), msTimeThreshold_Editor.getText(), nullptr);
        m_midiDisplay.setTimeThreshold(msTimeThreshold_Editor.getText().getDoubleValue(), true);
    };

    {
        addAndMakeVisible(playHeadTempo);
        playHeadTempo.setReadOnly(true);


        addAndMakeVisible(editTempo_Toggle);
        editTempo_Toggle.onClick = [&]()
        {
            audioProcessor.stateInfo.setProperty(NAME_OF(editTempo_Toggle), editTempo_Toggle.getToggleState(), nullptr);
            tempo_Editor.setVisible(editTempo_Toggle.getToggleState());
        };

        addAndMakeVisible(tempo_Editor);
        tempo_Editor.setSelectAllWhenFocused(true);
        tempo_Editor.setVisible(editTempo_Toggle.getToggleState());
        tempo_Editor.onTextChange = [&]()
        {
            audioProcessor.stateInfo.setProperty(NAME_OF(tempo_Editor), tempo_Editor.getText(), nullptr);
        };
    }

    {
        addAndMakeVisible(measureStart_Title);
        addAndMakeVisible(measureStart_Editor);
        measureStart_Editor.setSelectAllWhenFocused(true);
        measureStart_Editor.onTextChange = [&]()
        {
            audioProcessor.stateInfo.setProperty(NAME_OF(measureStart_Editor), measureStart_Editor.getText(), nullptr);
            m_midiDisplay.setMeasureRange(measureStart_Editor.getText().getIntValue(), 
                                          measureRangeLength_Editor.getText().getIntValue(), true);
        };

        addAndMakeVisible(measureRangeLength_Title);
        addAndMakeVisible(measureRangeLength_Editor);
        measureRangeLength_Editor.setSelectAllWhenFocused(true);
        measureRangeLength_Editor.onTextChange = [&]()
        {
            audioProcessor.stateInfo.setProperty(NAME_OF(measureRangeLength_Editor), 
                                                 measureRangeLength_Editor.getText(), nullptr);
            m_midiDisplay.setMeasureRange(measureStart_Editor.getText().getIntValue(),
                                          measureRangeLength_Editor.getText().getIntValue(), true);
        };
    }

    addAndMakeVisible(midiDirectory_Editor);
    midiDirectory_Editor.setSelectAllWhenFocused(true);
    midiDirectory_Editor.onTextChange = [&]()
    {
        audioProcessor.stateInfo.setProperty(NAME_OF(midiDirectory_Editor), midiDirectory_Editor.getText(), nullptr);
    };

    addAndMakeVisible(setQuantizedMidiFile_Button);
    setQuantizedMidiFile_Button.onClick = [&]() { setQuantizedMidiFile(getNewMidiFile()); };

    addAndMakeVisible(refreshQuantizedMidi_Button);
    refreshQuantizedMidi_Button.onClick = [&]() { setQuantizedMidiFile(m_quantizedMidiFile); };

    addAndMakeVisible(analyzeMidiFile_Button);
    analyzeMidiFile_Button.onClick = [&]()
    {
        juce::File newMidiFile = getNewMidiFile();
        if (!getMidiFile(newMidiFile, midiFileToAnalyze))
        {
            detectNewMidiLog.setText("can't read midi file");
            return;
        }
        newestMidiFile = newMidiFile;
        analyzeMidiFile();
    };

    {
        addAndMakeVisible(detectNewMidi_Toggle);
        detectNewMidi_Toggle.onClick = [&]()
        {
            bool newState = detectNewMidi_Toggle.getToggleState();
            audioProcessor.stateInfo.setProperty(NAME_OF(detectNewMidi_Toggle), newState, nullptr);
            detectNewMidiFrequency_Title.setVisible(newState);
            detectNewMidiFrequency_Editor.setVisible(newState);

            if (!newState)
                stopTimer();
            else if (!isTimerRunning())
                startTimer(m_msDetectNewMidiFrequency);
        };

        addAndMakeVisible(detectNewMidiFrequency_Title);
        detectNewMidiFrequency_Title.setVisible(detectNewMidi_Toggle.getToggleState());

        addAndMakeVisible(detectNewMidiFrequency_Editor);
        detectNewMidiFrequency_Editor.setSelectAllWhenFocused(true);
        detectNewMidiFrequency_Editor.setVisible(detectNewMidi_Toggle.getToggleState());

        detectNewMidiFrequency_Editor.onTextChange = [&]()
        {
            int newFrequency = detectNewMidiFrequency_Editor.getText().getIntValue();
            if (newFrequency > 0 && newFrequency != m_msDetectNewMidiFrequency)
            {
                if (isTimerRunning())
                    stopTimer();
                startTimer(newFrequency);

                m_msDetectNewMidiFrequency = newFrequency;
                audioProcessor.stateInfo.setProperty(NAME_OF(detectNewMidiFrequency_Editor), newFrequency, nullptr);
            }
        };

        if (detectNewMidi_Toggle.getToggleState())
            startTimer(m_msDetectNewMidiFrequency);

        addAndMakeVisible(detectNewMidiLog); 
    }

    setResizable(true, true);

    if (audioProcessor.stateInfo.getProperty("width") && audioProcessor.stateInfo.getProperty("height"))
        setSize(audioProcessor.stateInfo.getProperty("width"), audioProcessor.stateInfo.getProperty("height"));
    else
        setSize(500, 500);

    audioProcessor.stateLoadedCallback = [this]() { loadStateInfo(); };
    loadStateInfo();
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
        int divisionsX = 3;
        auto tempoBounds = bounds.removeFromBottom(30);
        setQuantizedMidiFile_Button.setBounds(tempoBounds.withWidth(getWidth() / divisionsX));
        refreshQuantizedMidi_Button.setBounds(tempoBounds.withWidth(getWidth() / divisionsX)
                                              .withX(setQuantizedMidiFile_Button.getRight()));
        analyzeMidiFile_Button.setBounds(tempoBounds.withWidth(getWidth() / divisionsX)
                                              .withX(refreshQuantizedMidi_Button.getRight()));
    }
    {
        auto tempoBounds = bounds.removeFromBottom(30);
        midiDirectory_Editor.setBounds(tempoBounds.withWidth(getWidth() / 3));

        detectNewMidi_Toggle.setBounds(tempoBounds.withWidth(200).withX(midiDirectory_Editor.getRight() + 10));

        detectNewMidiFrequency_Title.setBounds(tempoBounds.withSize(0, 25)
                                               .withX(detectNewMidi_Toggle.getRight()));
        detectNewMidiFrequency_Title.changeWidthToFitText();
        detectNewMidiFrequency_Editor.setBounds(tempoBounds.withSize(50, 25)
                                                .withX(detectNewMidiFrequency_Title.getRight()));

        detectNewMidiLog.setBounds(tempoBounds.withX(detectNewMidiFrequency_Editor.getRight()).withRight(getWidth()));
    }
    {
        auto tempoBounds = bounds.removeFromBottom(30);
        playHeadTempo.setBounds(tempoBounds.withSize(100, 25));
        editTempo_Toggle.setBounds(tempoBounds.withSize(100, 25).withX(playHeadTempo.getRight() + 10));
        tempo_Editor.setBounds(tempoBounds.withSize(100, 25).withX(editTempo_Toggle.getRight()));

        measureStart_Title.setBounds(tempoBounds.withSize(0, 25).withX(tempo_Editor.getRight() + 10));
        measureStart_Title.changeWidthToFitText();
        measureStart_Editor.setBounds(tempoBounds.withSize(50, 25).withX(measureStart_Title.getRight()));

        measureRangeLength_Title.setBounds(tempoBounds.withSize(0, 25).withX(measureStart_Editor.getRight() + 10));
        measureRangeLength_Title.changeWidthToFitText();
        measureRangeLength_Editor.setBounds(tempoBounds.withSize(50, 25).withX(measureRangeLength_Title.getRight()));
    }
    {
        auto tempoBounds = bounds.removeFromBottom(30);
        drumNotes_Toggle.setBounds(tempoBounds.withWidth(100));

        msTimeThreshold_Title.setBounds(tempoBounds.withSize(0, 25).withX(drumNotes_Toggle.getRight() + 10));
        msTimeThreshold_Title.changeWidthToFitText();
        msTimeThreshold_Editor.setBounds(tempoBounds.withSize(50, 25).withX(msTimeThreshold_Title.getRight()));

        fontSize_Slider.setBounds(tempoBounds.withWidth(300).withX(msTimeThreshold_Editor.getRight() + 10));
    }
    m_midiDisplay.setBounds(bounds);
    midiResults.setBounds(bounds);
}

void TimeAnalyzerAudioProcessorEditor::timerCallback()
{
    if (!m_quantizedMidiFile.exists() && quantizedMidi.isEmpty())
        return;

    if (audioProcessor.getPlayHead() != nullptr
        && audioProcessor.getPlayHead()->getPosition()->getIsRecording())
    {
        //detectNewMidiLog.setText("recording");
        return; //the host might be recording the newest midi file
    }

    if (quantizedMidi.isEmpty())
    {
        detectNewMidiLog.setText("Please Set a Quantized Midi File");
        return;
    }

    juce::File newMidiFile = getNewMidiFile();
    if (newMidiFile == newestMidiFile || newMidiFile == m_quantizedMidiFile)
    {
        return; //no new midi files
    }
    if (!getMidiFile(newMidiFile, midiFileToAnalyze))
    {
        detectNewMidiLog.setText("can't read midi file");
        return;
    }
    newestMidiFile = newMidiFile;

    analyzeMidiFile();
}

void TimeAnalyzerAudioProcessorEditor::setQuantizedMidiFile(juce::File quantizedMidiFile)
{
    playHeadTempo.setText(juce::String(audioProcessor.playHeadBpm));

    quantizedMidi.clear();
    juce::MidiFile quantizedMidiFileObject;
    if (!getMidiFile(quantizedMidiFile, quantizedMidiFileObject))
        return;
    readMidiFile(quantizedMidiFileObject, quantizedMidi);

    m_quantizedMidiFile = quantizedMidiFile;
    audioProcessor.stateInfo.setProperty(NAME_OF(m_quantizedMidiFile), m_quantizedMidiFile.getFullPathName(), nullptr);

    juce::String results = "Quantized Midi Info: \n";
    for (const MidiEvent& midi : quantizedMidi)
    {
        results += "Note: " + getMidiNoteName(midi.note)
            + ", num: " + juce::String(midi.note)
            + ", ms: " + juce::String(midi.ms) + juce::newLine;
    }
    midiResults.setText(results);

    m_midiDisplay.setQuantizedMidi(quantizedMidi, audioProcessor.getPlayHead());
}

void TimeAnalyzerAudioProcessorEditor::analyzeMidiFile()
{
    if (quantizedMidi.isEmpty())
    {
        midiResults.setText("Please Set a Quantized Midi File");
        return;
    }

    playHeadTempo.setText(juce::String(audioProcessor.playHeadBpm));

    juce::Array<MidiEvent> midiEventsToAnalyze;
    readMidiFile(midiFileToAnalyze, midiEventsToAnalyze);
    juce::String results = "Analysis: \n";
    for (MidiEvent& midi : midiEventsToAnalyze)
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
        midi.msDifference = lowestDifference;
        results += "Note: " + getMidiNoteName(midi.note) + ", ms Diff: " + juce::String(lowestDifference) + juce::newLine;
    }
    midiResults.setText(results);
    m_midiDisplay.setAnalyzedMidi(midiEventsToAnalyze);

    detectNewMidiLog.setText("");
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

bool TimeAnalyzerAudioProcessorEditor::getMidiFile(juce::File fileOfMidi, juce::MidiFile& out)
{
    juce::FileInputStream inputStream(fileOfMidi);
    if (inputStream.failedToOpen())
    {
        return false;
    }
    out.clear();
    if (!out.readFrom(inputStream))
    {
        return false;
    }
    return true;
}

void TimeAnalyzerAudioProcessorEditor::readMidiFile(juce::MidiFile midiFile, juce::Array<MidiEvent>& out)
{
    double currentBpm;
    if (editTempo_Toggle.getToggleState())
        currentBpm = tempo_Editor.getText().getDoubleValue();
    else
        currentBpm = audioProcessor.playHeadBpm;

    for (int i = 0; i < midiFile.getNumTracks(); i++)
    {
        for (auto sequence : *midiFile.getTrack(i))
        {
            if (sequence->message.isNoteOn())
                out.add(MidiEvent(*sequence, currentBpm));
        }
    }
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

void TimeAnalyzerAudioProcessorEditor::loadStateInfo()
{
    midiResults.setText(audioProcessor.stateInfo.getProperty(NAME_OF(midiResults)), false);

    fontSize_Slider.setValue(audioProcessor.stateInfo.getProperty(NAME_OF(fontSize_Slider)));

    juce::var loadMSTimeThreshold = audioProcessor.stateInfo.getProperty(NAME_OF(msTimeThreshold_Editor));
    if (!loadMSTimeThreshold.isVoid())
    {
        m_midiDisplay.setTimeThreshold(loadMSTimeThreshold, false);
        msTimeThreshold_Editor.setText(loadMSTimeThreshold, false);
    }
    else
    {
        m_midiDisplay.setTimeThreshold(0, false);
        msTimeThreshold_Editor.setText("", false);
    }

    editTempo_Toggle.setToggleState(audioProcessor.stateInfo.getProperty(NAME_OF(editTempo_Toggle)), true);
    tempo_Editor.setText(audioProcessor.stateInfo.getProperty(NAME_OF(tempo_Editor), false));

    juce::var measureStart = audioProcessor.stateInfo.getProperty(NAME_OF(measureStart_Editor));
    juce::var measureLength = audioProcessor.stateInfo.getProperty(NAME_OF(measureRangeLength_Editor));
    if (!measureStart.isVoid() && !measureLength.isVoid())
    {
        measureStart_Editor.setText(measureStart, false);
        measureRangeLength_Editor.setText(measureLength, false);
        m_midiDisplay.setMeasureRange(measureStart, measureLength, false);
    }
    else
    {
        measureStart_Editor.setText("", false);
        measureRangeLength_Editor.setText("", false);
    }

    midiDirectory_Editor.setText(audioProcessor.stateInfo.getProperty(NAME_OF(midiDirectory_Editor)), false);

    detectNewMidi_Toggle.setToggleState(audioProcessor.stateInfo.getProperty(NAME_OF(detectNewMidi_Toggle)), true);

    juce::var loadFrequency = audioProcessor.stateInfo.getProperty(NAME_OF(detectNewMidiFrequency_Editor));
    if (!loadFrequency.isVoid() && (int)loadFrequency > 0)
        m_msDetectNewMidiFrequency = loadFrequency;
    detectNewMidiFrequency_Editor.setText(juce::String(m_msDetectNewMidiFrequency));
    
    juce::String quantizedMidiFilePath = audioProcessor.stateInfo.getProperty(NAME_OF(m_quantizedMidiFile));
    if (quantizedMidiFilePath.isNotEmpty())
    {
        setQuantizedMidiFile(juce::File(quantizedMidiFilePath));
    }
}
