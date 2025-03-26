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
    initializeUI();

    audioProcessor.stateLoadedCallback = [this]() { loadStateInfo(); };
    loadStateInfo();
}

TimeAnalyzerAudioProcessorEditor::~TimeAnalyzerAudioProcessorEditor()
{
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
    setPlayHeadInfo();

    quantizedMidi.clear();
    juce::MidiFile quantizedMidiFileObject;
    if (!getMidiFile(quantizedMidiFile, quantizedMidiFileObject))
        return;
    readMidiFile(quantizedMidiFileObject, quantizedMidi);

    m_quantizedMidiFile = quantizedMidiFile;
    audioProcessor.stateInfo.setProperty(NAME_OF(m_quantizedMidiFile), m_quantizedMidiFile.getFullPathName(), nullptr);

    m_midiDisplay.setQuantizedMidi(quantizedMidi);
    debugPlugin("setQuantizedMidiFile");
}

void TimeAnalyzerAudioProcessorEditor::analyzeMidiFile()
{
    if (quantizedMidi.isEmpty())
    {
        detectNewMidiLog.setText("Please Set a Quantized Midi File");
        return;
    }

    setPlayHeadInfo();

    juce::Array<MidiEvent> midiEventsToAnalyze;
    readMidiFile(midiFileToAnalyze, midiEventsToAnalyze);
    m_midiDisplay.setAnalyzedMidi(midiEventsToAnalyze);

    detectNewMidiLog.setText("");
    debugPlugin("analyzeMidiFile");
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
        currentBpm = playHeadTempo.getText().getDoubleValue();
    debugLog("readMidiFile::currentBpm: " + juce::String(currentBpm));

    for (int i = 0; i < midiFile.getNumTracks(); i++)
    {
        for (auto sequence : *midiFile.getTrack(i))
        {
            if (sequence->message.isNoteOn())
            {
                out.add(MidiEvent(*sequence, currentBpm));
                debugLog("readMidiFile::message.ms: " + juce::String(out.getLast().ms));
            }
        }
    }
}

juce::String TimeAnalyzerAudioProcessorEditor::getMidiNoteName(juce::MidiMessage message)
{
    return getMidiNoteName(message.getNoteNumber());
}

juce::String TimeAnalyzerAudioProcessorEditor::getMidiNoteName(int note)
{
    if (false) //get midi note names for drums
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

void TimeAnalyzerAudioProcessorEditor::setPlayHeadInfo()
{
    auto playHead = audioProcessor.getPlayHead();
    double previousTempo = playHeadTempo.getText().getDoubleValue();
    if (playHead && audioProcessor.audioProcessCount > 0)
    {

        double newTempo = *playHead->getPosition()->getBpm();
        playHeadTempo.setText(juce::String(newTempo));
        m_midiDisplay.timeSignature = *playHead->getPosition()->getTimeSignature();

        if (previousTempo != newTempo)
            m_midiDisplay.setBpm(newTempo, true);
    }

    debugLog("setPlayHeadInfo::playHeadTempo: " + playHeadTempo.getText());
    debugLog("setPlayHeadInfo::m_midiDisplay.timeSignature: " + juce::String(m_midiDisplay.timeSignature.numerator) + "/" + juce::String(m_midiDisplay.timeSignature.denominator));
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

void TimeAnalyzerAudioProcessorEditor::debugPlugin(juce::String callFrom)
{
    if (!debug_Toggle.getToggleState())
        return;

    juce::String debugText = "\n=========================================================\n";
    debugText += callFrom + ":\n\n";

    debugText += "loadStateCount: " + juce::String(loadStateCount) + "\n";

    debugText += m_midiDisplay.debugMidiDisplay() + "\n";

    debugText += "Play Head is Null: " + juce::String((int)(audioProcessor.getPlayHead() == nullptr)) + "\n";
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

void TimeAnalyzerAudioProcessorEditor::loadStateInfo()
{
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
    juce::var loadTempoEdit = audioProcessor.stateInfo.getProperty(NAME_OF(tempo_Editor));
    if (!loadTempoEdit.isVoid())
    {
        tempo_Editor.setText(loadTempoEdit, false);
        if (editTempo_Toggle.getToggleState())
            m_midiDisplay.setBpm(tempo_Editor.getText().getDoubleValue(), false);
    }
    else
    {
        measureStart_Editor.setText("0", false);
    }

    juce::var loadRecordStartMeasure = audioProcessor.stateInfo.getProperty(NAME_OF(recordStartMeasure_Editor));
    if (!loadRecordStartMeasure.isVoid())
    {
        measureStart_Editor.setText(loadRecordStartMeasure, false);
        m_midiDisplay.setRecordStart(recordStartMeasure_Editor.getText().getIntValue(), false);
    }
    else
    {
        measureStart_Editor.setText("0", false);
    }

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

    juce::var quantizedMidiFilePath = audioProcessor.stateInfo.getProperty(NAME_OF(m_quantizedMidiFile));
    if (!quantizedMidiFilePath.isVoid())
    {
        setQuantizedMidiFile(juce::File(quantizedMidiFilePath.toString()));
    }

    juce::var debugToggle = audioProcessor.stateInfo.getProperty(NAME_OF(debug_Toggle));
    if (!debugToggle.isVoid())
        debug_Toggle.setToggleState(debugToggle, true);

    loadStateCount++;
    debugPlugin("loadStateInfo");
}

void TimeAnalyzerAudioProcessorEditor::initializeUI()
{
    g_debug_Display = &debug_Display;

    addAndMakeVisible(m_midiDisplay);
    m_midiDisplay.setVisible(true);

    //debug
    {
        addAndMakeVisible(debug_Display);
        debug_Display.setMultiLine(true, true);
        debug_Display.setVisible(false);

        addAndMakeVisible(debug_Toggle);
        debug_Toggle.onClick = [&]()
        {
            debug_Display.setVisible(debug_Toggle.getToggleState());
            m_midiDisplay.setVisible(!debug_Toggle.getToggleState());
            audioProcessor.stateInfo.setProperty(NAME_OF(debug_Toggle), debug_Toggle.getToggleState(), nullptr);
        };
        addAndMakeVisible(debugClear_Button);
        debugClear_Button.onClick = [&]()
        {
            debug_Display.clear();
        };
        addAndMakeVisible(debugRefresh_Button);
        debugRefresh_Button.onClick = [&]()
        {
            debug_Display.clear();
            debugPlugin("debugRefresh_Button");
        };
    }

    addAndMakeVisible(msTimeThreshold_Title);
    addAndMakeVisible(msTimeThreshold_Editor);
    msTimeThreshold_Editor.setSelectAllWhenFocused(true);
    msTimeThreshold_Editor.onTextChange = [&]()
    {
        audioProcessor.stateInfo.setProperty(NAME_OF(msTimeThreshold_Editor), msTimeThreshold_Editor.getText(), nullptr);
        m_midiDisplay.setTimeThreshold(msTimeThreshold_Editor.getText().getDoubleValue(), true);
    };

    {
        addAndMakeVisible(playHeadTempo_Title);
        addAndMakeVisible(playHeadTempo);
        playHeadTempo.setReadOnly(true);
        playHeadTempo.setText("120");


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
            m_midiDisplay.setBpm(tempo_Editor.getText().getDoubleValue(), true);
        };
    }

    {
        addAndMakeVisible(recordStartMeasure_Title);
        addAndMakeVisible(recordStartMeasure_Editor);
        recordStartMeasure_Editor.setSelectAllWhenFocused(true);
        recordStartMeasure_Editor.onTextChange = [&]()
        {
            audioProcessor.stateInfo.setProperty(NAME_OF(recordStartMeasure_Editor), recordStartMeasure_Editor.getText(), nullptr);
            m_midiDisplay.setRecordStart(recordStartMeasure_Editor.getText().getIntValue(), true);
        };
        
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

    addAndMakeVisible(midiDirectory_Title);
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
        detectNewMidiLog.setTextToShowWhenEmpty("Log", juce::Colours::grey);
        detectNewMidiLog.setReadOnly(true);
    }

    setResizable(true, true);

    if (audioProcessor.stateInfo.getProperty("width") && audioProcessor.stateInfo.getProperty("height"))
        setSize(audioProcessor.stateInfo.getProperty("width"), audioProcessor.stateInfo.getProperty("height"));
    else
        setSize(500, 500);
}

void TimeAnalyzerAudioProcessorEditor::resized()
{
    audioProcessor.stateInfo.setProperty("width", getWidth(), nullptr);
    audioProcessor.stateInfo.setProperty("height", getHeight(), nullptr);

    Bounds bounds = getLocalBounds();

    {
        int divisionsX = 3;
        Bounds tempoBounds = bounds.removeFromBottom(30).withHeight(25);

        setQuantizedMidiFile_Button.setBounds(tempoBounds.withWidth(getWidth() / divisionsX));
        refreshQuantizedMidi_Button.setBounds(tempoBounds.withWidth(getWidth() / divisionsX)
                                              .withX(setQuantizedMidiFile_Button.getRight()));
        analyzeMidiFile_Button.setBounds(tempoBounds.withWidth(getWidth() / divisionsX)
                                         .withX(refreshQuantizedMidi_Button.getRight()));
    }
    {
        Bounds tempoBounds = bounds.removeFromBottom(30).withHeight(25);

        fitButtonInLeftBounds(tempoBounds, midiDirectory_Title);
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
        Bounds tempoBounds = bounds.removeFromBottom(30).withHeight(25);

        fitButtonInLeftBounds(tempoBounds, recordStartMeasure_Title);
        recordStartMeasure_Editor.setBounds(tempoBounds.removeFromLeft(40));

        tempoBounds.removeFromLeft(10);

        fitButtonInLeftBounds(tempoBounds, measureStart_Title);
        measureStart_Editor.setBounds(tempoBounds.removeFromLeft(40));

        fitButtonInLeftBounds(tempoBounds, measureRangeLength_Title);
        measureRangeLength_Editor.setBounds(tempoBounds.removeFromLeft(30));
    }
    {
        Bounds tempoBounds = bounds.removeFromBottom(30).withHeight(25);

        fitButtonInLeftBounds(tempoBounds, debug_Toggle);
        fitButtonInLeftBounds(tempoBounds, debugClear_Button);
        fitButtonInLeftBounds(tempoBounds, debugRefresh_Button);

        tempoBounds.removeFromLeft(10);

        fitButtonInLeftBounds(tempoBounds, playHeadTempo_Title);
        playHeadTempo.setBounds(tempoBounds.removeFromLeft(50));
        fitButtonInLeftBounds(tempoBounds, editTempo_Toggle);
        tempo_Editor.setBounds(tempoBounds.removeFromLeft(50));

        tempoBounds.removeFromLeft(10);

        fitButtonInLeftBounds(tempoBounds, msTimeThreshold_Title);
        msTimeThreshold_Editor.setBounds(tempoBounds.removeFromLeft(30));
    }
    bounds.removeFromBottom(10);

    m_midiDisplay.setBounds(bounds);
    debug_Display.setBounds(bounds);
}

void TimeAnalyzerAudioProcessorEditor::paint(juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

}
