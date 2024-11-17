#include "MidiDisplay.h"

//==============================================================================
const double g_quarterNoteTicks = 960;

//==============================================================================

void MidiDisplay::paint(juce::Graphics& g)
{
	int displayPadding = 100;
	int displayWidth = getWidth() - displayPadding;
	int displayOffset = displayPadding / 2;

	static int repaintCounter = 0;
	repaintCounter++;
	DBG("Repainted: " << repaintCounter);

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
		float relativeBeat = midi.tickStart / g_quarterNoteTicks - m_beatStart; 

		float startTimePosition = relativeBeat / beatRange * displayWidth + displayOffset;
		float pitchPosition = (m_highestNote - midi.note) * noteDisplayHeight;
		g.fillRect(startTimePosition, pitchPosition, noteDisplayWidth, noteDisplayHeight);
	}

	//analyze midi hits
	for (const MidiEvent& midi : m_analyzedMidi)
	{
		if (std::abs(midi.msDifference) <= m_msTimeThreshold)
			g.setColour(onTimeColor);
		else if (midi.msDifference > 0) //late
			g.setColour(lateColor);
		else //early
			g.setColour(earlyColor);

		//relative to the display range rather than the midi file
		float relativeBeat = midi.tickStart / g_quarterNoteTicks - m_beatStart; 

		float startTimePosition = relativeBeat / beatRange * displayWidth + displayOffset;
		float pitchPosition = (m_highestNote - midi.note) * noteDisplayHeight;
		g.fillRect(startTimePosition, pitchPosition, analyzedNoteDisplayWidth, noteDisplayHeight);
	}
}

void MidiDisplay::resized()
{
}

void MidiDisplay::setQuantizedMidi(const juce::Array<MidiEvent>& newQuantizedMidi)
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
	m_quantizedBeatRange = std::ceil(lastTick / g_quarterNoteTicks);

	repaint();
}

void MidiDisplay::setAnalyzedMidi(const juce::Array<MidiEvent>& newAnalyzedMidi)
{
	m_analyzedMidi = newAnalyzedMidi; //copy

	repaint();
}
