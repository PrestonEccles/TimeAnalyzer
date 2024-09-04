#include "MidiDisplay.h"

//==============================================================================
const double g_quarterNoteTicks = 960;

//==============================================================================

MidiDisplay::MidiDisplay() : m_beatsToShow(0), noteDisplayWidth(2), m_beatSubDivisions(4), msTimeThreshold(20)
{
}
void MidiDisplay::paint(juce::Graphics& g)
{
	//measure grid
	g.setColour(juce::Colours::grey);
	for (int i = 0; i <= m_beatsToShow * m_beatSubDivisions; i++)
	{
		float lineThickness;
		if (i % (m_timeSignature.numerator * m_beatSubDivisions) == 0) //first beat in the measure
			lineThickness = 3.f;
		else if (i % m_beatSubDivisions == 0)
			lineThickness = 2.f;
		else if (i % (m_beatSubDivisions / 2) == 0)
			lineThickness = 1.f;
		else
			lineThickness = 0.5f;

		int beatPosition = ((double) i / m_beatSubDivisions) / m_beatsToShow * getWidth();
		g.drawLine(beatPosition, 0, beatPosition, getHeight(), lineThickness);
	}

	int noteRangeDistance = (m_highestNote - m_lowestNote + 1);
	float noteDisplayHeight = getHeight() / noteRangeDistance;

	//quantized midi hits
	g.setColour(quantizedColor);
	for (const MidiEvent& midi : m_quantizedMidi)
	{
		float beat = midi.tickStart / g_quarterNoteTicks;
		float startTimePosition = beat / m_beatsToShow * getWidth();
		float notePosition = (m_highestNote - midi.note) * noteDisplayHeight;
		g.fillRect(startTimePosition, notePosition, noteDisplayWidth, noteDisplayHeight);
	}

	//analyze midi hits
	for (const MidiEvent& midi : m_analyzedMidi)
	{
		if (std::abs(midi.msDifference) <= msTimeThreshold)
			g.setColour(onTimeColor);
		else if (midi.msDifference > 0) //late
			g.setColour(lateColor);
		else //early
			g.setColour(earlyColor);

		float beat = midi.tickStart / g_quarterNoteTicks;
		float startTimePosition = beat / m_beatsToShow * getWidth();
		float notePosition = (m_highestNote - midi.note) * noteDisplayHeight;
		g.fillRect(startTimePosition, notePosition, noteDisplayWidth, noteDisplayHeight);
	}
}

void MidiDisplay::resized()
{
}

void MidiDisplay::setQuantizedMidi(const juce::Array<MidiEvent>& newQuantizedMidi, juce::AudioPlayHead* playHead)
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
	m_beatsToShow = std::ceil(lastTick / g_quarterNoteTicks);

	if (playHead != nullptr)
	{
		m_timeSignature = *playHead->getPosition()->getTimeSignature();
	}
	else
	{
		m_timeSignature.numerator = 4;
		m_timeSignature.denominator = 4;
	}

	repaint();
}

void MidiDisplay::setAnalyzedMidi(const juce::Array<MidiEvent>& newAnalyzedMidi)
{
	m_analyzedMidi = newAnalyzedMidi; //copy

	repaint();
}
