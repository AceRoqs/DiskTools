#pragma once

namespace WindowsCommon
{

class Clock
{
private:
    LARGE_INTEGER m_frequency;
    LARGE_INTEGER m_last_counter;

public:
    Clock() noexcept;
    float ellapsed_milliseconds() noexcept;
};

}

