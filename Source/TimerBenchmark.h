#pragma once

#include <JuceHeader.h>
#include <chrono>

class TimerBench
{
public:
    TimerBench();
    TimerBench(juce::String label);
    ~TimerBench();

    juce::String getLabel() { return m_label;  }

    void Start();
    void Stop(juce::String label);
    juce::String StopAndGetTime() { return StopAndGetTime(m_label); }
    juce::String StopAndGetTime(juce::String label);

    juce::String formatTime(long long time);
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimepoint;
    juce::String m_label;

private:
    JUCE_LEAK_DETECTOR(TimerBench)
};