#include "MidiDisplay.h"

//==============================================================================
const double g_quarterNoteTicks = 960;

//==============================================================================

MidiDisplay::MidiDisplay() : m_beatsToShow(0)
{
}
void MidiDisplay::paint(juce::Graphics& g)
{
	for (int i = 0; i <= m_beatsToShow; i++)
	{
		float lineThickness = 1.f;
		if (i % m_timeSignature.numerator == 0) //first beat in the measure
		{
			lineThickness = 2.f;
		}

		int beatPosition = i / m_beatsToShow * getWidth();
		g.drawLine(beatPosition, 0, beatPosition, getHeight(), lineThickness);
	}
}

void MidiDisplay::resized()
{
}

void MidiDisplay::setQuantizedMidi(const juce::Array<MidiEvent>& allMidi, juce::AudioPlayHead* playHead)
{
	double lastEventTicks = 0;
	for (const MidiEvent& midi : allMidi)
	{
		if (midi.ticks > lastEventTicks)
		{
			lastEventTicks = midi.ticks;
		}
	}

	m_beatsToShow = std::ceil(lastEventTicks / g_quarterNoteTicks);
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
