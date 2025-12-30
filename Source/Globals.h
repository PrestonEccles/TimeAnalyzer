#pragma once

#include <JuceHeader.h>

#define NAME_OF(name) (#name)

inline juce::Font getMonoFont(float fontHeight = 14.f) { return juce::Font("Cascadia Mono", fontHeight, 0); }
inline const juce::Colour g_defaultEditorColor = juce::Colour(38, 50, 56);

typedef juce::Rectangle<int> Bounds;
typedef juce::String jString;


//return the expanded area
inline Bounds expandBottomOfBounds(Bounds& bounds, int expand)
{
	bounds = bounds.withTrimmedBottom(-expand);
	return bounds.withTop(bounds.getBottom() - expand);
}

template<class ButtonType>
void fitButtonInLeftBounds(Bounds& bounds, ButtonType& button, int height = -1)
{
	button.setSize(0, bounds.getHeight());
	button.changeWidthToFitText();

	if (height == -1)
		height = bounds.getHeight();
	button.setBounds(bounds.removeFromLeft(button.getWidth()).withHeight(height));
}

inline juce::String getValueTreeID(juce::ValueTree& valueTree) { return valueTree.getType().toString(); }
inline juce::XmlElement::TextFormat getXmlNoWrapFormat()
{
	juce::XmlElement::TextFormat format;
	format.lineWrapLength = 0x7FFFFFFF;
	return format;
}
