#include "TimerBenchmark.h"

//==============================================================================

TimerBench::TimerBench()
{
    Start();
    m_label = "";
}

TimerBench::TimerBench(juce::String label)
{
    Start();
    m_label = label;
}

TimerBench::~TimerBench()
{
    if (m_label != "")
        Stop(m_label);
}

void TimerBench::Start()
{
    m_StartTimepoint = std::chrono::high_resolution_clock::now();
}

void TimerBench::Stop(juce::String label)
{
    auto endTimepoint = std::chrono::high_resolution_clock::now();
    auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
    auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

    auto duration = end - start;

    DBG(label + ": " << formatTime(duration));
}

juce::String TimerBench::StopAndGetTime(juce::String label)
{
    auto endTimepoint = std::chrono::high_resolution_clock::now();
    auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
    auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

    auto duration = end - start;
    return label + ": " + formatTime(duration);
}

juce::String TimerBench::formatTime(long long time)
{
    juce::String timeString = juce::String(time);
    juce::String output;
    output.preallocateBytes(timeString.length());

    //inserting comma every 3 digits
    int index = 0;
    int offset = timeString.length() % 3;
    while (index < offset)
    {
        output += timeString[index++];
    }
    while (index < timeString.length())
    {
        if (index != 0 && (index - offset) % 3 == 0)
            output += ',';

        output += timeString[index++];
    }

    return output;
}
