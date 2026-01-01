#include "MidiDisplay.h"

//==============================================================================

void MidiDisplay::paint(juce::Graphics& g)
{
	int displayPadding = 100;
	int displayWidth = getWidth() - displayPadding;
	int displayOffset = displayPadding / 2;

	static int repaintCounter = 0;
	repaintCounter++;

	double beatRange;
	if (m_beatStart >= 0 && m_beatStart < m_beatEnd) //valid range?
	{
		beatRange = (m_beatEnd - m_beatStart);
	}
	else
	{
		//use the range set by the quantized midi
		m_beatStart = 0;
		m_beatEnd = m_quantizedBeatRange;
		beatRange = m_quantizedBeatRange;
	}

	//measure grid
	g.setColour(juce::Colours::grey);
	for (int i = 0; i <= beatRange * m_beatSubDivisions; i++)
	{
		float lineThickness;
		if (i % (timeSignature.numerator * m_beatSubDivisions) == 0) //first beat in the measure
			lineThickness = 3.f;
		else if (i % m_beatSubDivisions == 0)
			lineThickness = 2.f;
		else if (i % (m_beatSubDivisions / 2) == 0)
			lineThickness = 1.f;
		else
			lineThickness = 0.5f;

		int beatPosition = ((double) i / m_beatSubDivisions) / beatRange * displayWidth + displayOffset;
		g.drawLine(beatPosition, 0, beatPosition, getHeight(), lineThickness);
	}

	int noteRangeDistance = (m_highestNote - m_lowestNote + 1);
	float noteDisplayHeight = getHeight() / noteRangeDistance;

	//quantized midi hits
	g.setColour(quantizedColor);
	for (const MidiEvent& midi : m_quantizedMidi)
	{
		//relative to the display range rather than the midi file
		float relativeBeat = midi.tickStart / midi.quarterNoteTicks - m_beatStart;

		float startTimePosition = relativeBeat / beatRange * displayWidth + displayOffset;
		float pitchPosition = (m_highestNote - midi.note) * noteDisplayHeight;
		g.fillRect(startTimePosition, pitchPosition, noteDisplayWidth, noteDisplayHeight);
	}

	//analyze midi hits
	for (const MidiEvent& midi : m_analyzedMidi)
	{
		//relative to record start
		double tickStart = midi.tickStart + getRecordTickStart(midi.quarterNoteTicks);
		double msStart = MidiEvent::getMiliseconds(tickStart, m_bpm, midi.quarterNoteTicks);

		if (midi.closestQuantizedIndex < 0 || midi.closestQuantizedIndex >= m_quantizedMidi.size())
			continue;
		MidiEvent& quantizedMidi = m_quantizedMidi[midi.closestQuantizedIndex];
		double quantizedMSStart = MidiEvent::getMiliseconds(quantizedMidi.tickStart, m_bpm, quantizedMidi.quarterNoteTicks);

		double msDifference = msStart - quantizedMSStart;
		if (std::abs(msDifference) <= m_msTimeThreshold)
			g.setColour(onTimeColor);
		else if (msDifference > 0) //late
			g.setColour(lateColor);
		else //early
			g.setColour(earlyColor);

		//relative to the display range rather than the midi file
		float relativeBeat = tickStart / midi.quarterNoteTicks - m_beatStart;

		float startTimePosition = relativeBeat / beatRange * displayWidth + displayOffset;
		int note = midi.useQuantizedNote ? quantizedMidi.note : midi.note;
		float pitchPosition = (m_highestNote - note) * noteDisplayHeight;
		g.fillRect(startTimePosition, pitchPosition, analyzedNoteDisplayWidth, noteDisplayHeight);
	}
}

void MidiDisplay::resized()
{
}

void MidiDisplay::setQuantizedMidi(const vArray<MidiEvent>& newQuantizedMidi)
{
	m_analyzedMidi.clear();
	m_quantizedMidi = newQuantizedMidi; //copy

	double lastTick = 0;
	m_lowestNote = m_quantizedMidi[0].note;
	m_highestNote = m_quantizedMidi[0].note;
	for (const MidiEvent& midi : m_quantizedMidi)
	{
		if (midi.tickEnd > lastTick)
		{
			lastTick = midi.tickEnd;
		}

		if (midi.note < m_lowestNote)
			m_lowestNote = midi.note;
		if (midi.note > m_highestNote)
			m_highestNote = midi.note;
	}
	m_lowestNote -= 1; //padding
	m_highestNote += 1; //padding
	m_quantizedBeatRange = std::ceil(lastTick / m_quantizedMidi[0].quarterNoteTicks);

	updateAnalyzedMidi();
}

void MidiDisplay::setAnalyzedMidi(const vArray<MidiEvent>& newAnalyzedMidi)
{
	m_analyzedMidi = newAnalyzedMidi;
	updateAnalyzedMidi();
}

void MidiDisplay::updateAnalyzedMidi()
{
	for (MidiEvent& midi : m_analyzedMidi)
	{
		//relative to the record start
		double tickStart = midi.tickStart + getRecordTickStart(midi.quarterNoteTicks);

		double lowestDifference = tickStart - m_quantizedMidi[0].getTickStart(midi.quarterNoteTicks);
		int closestQuantizedIndex = 0;
		for (int i = 1; i < m_quantizedMidi.size(); i++)
		{
			if (std::abs(tickStart - m_quantizedMidi[i].getTickStart(midi.quarterNoteTicks)) < std::abs(lowestDifference))
			{
				lowestDifference = tickStart - m_quantizedMidi[i].getTickStart(midi.quarterNoteTicks);
				closestQuantizedIndex = i;
			}
		}
		midi.closestQuantizedIndex = closestQuantizedIndex;
	}

	repaint();
}

void MidiDisplay::clearAnalyzedMidi(bool repaintMidi)
{
	m_analyzedMidi.clear();
	if (repaintMidi)
		repaint();
}

void MidiDisplay::setBpm(double bpm, bool repaintMidi)
{
	if (bpm < 0)
		return; //not valid

	m_bpm = bpm;
	if (repaintMidi)
		repaint();
}

void MidiDisplay::setTimeThreshold(double ms, bool repaintMidi)
{
	if (ms < 0)
		return; //not valid

	m_msTimeThreshold = ms;
	if (repaintMidi)
		repaint();
}

void MidiDisplay::setMeasureRange(double measureStart, double length, bool repaintMidi)
{
	m_beatStart = measureStart * timeSignature.numerator;
	m_beatEnd = (measureStart + length) * timeSignature.numerator;
	if (repaintMidi)
		updateAnalyzedMidi();
}

void MidiDisplay::setRecordStart(double measure, bool repaintMidi)
{
	m_recordBeatStart = measure * timeSignature.numerator;
	if (repaintMidi)
		updateAnalyzedMidi();
}

juce::String MidiDisplay::debugMidiDisplay()
{
	juce::String output = "MidiDisplay:\n";

	output += "m_quantizedMidi:\n";
	for (auto& m : m_quantizedMidi)
	{
		output += m.debugMidiEvent() + "\n";
	}
	output += "\n";

	output += "m_analyzedMidi:\n";
	for (auto& m : m_analyzedMidi)
	{
		output += m.debugMidiEvent() + "\n";
	}
	output += "\n";

	output += "timeSignature.numerator: " + juce::String(timeSignature.numerator) + "\n";
	output += "timeSignature.denominator: " + juce::String(timeSignature.denominator) + "\n";
	output += "m_beatSubDivisions: " + juce::String(m_beatSubDivisions) + "\n";
	output += "m_quantizedBeatRange: " + juce::String(m_quantizedBeatRange) + "\n";
	output += "m_beatStart: " + juce::String(m_beatStart) + "\n";
	output += "m_beatEnd: " + juce::String(m_beatEnd) + "\n";
	output += "m_lowestNote: " + juce::String(m_lowestNote) + "\n";
	output += "m_highestNote: " + juce::String(m_highestNote) + "\n";
	output += "m_msTimeThreshold: " + juce::String(m_msTimeThreshold) + "\n";
	output += "noteDisplayWidth: " + juce::String(noteDisplayWidth) + "\n";
	output += "analyzedNoteDisplayWidth: " + juce::String(analyzedNoteDisplayWidth) + "\n";

	return output;
}
