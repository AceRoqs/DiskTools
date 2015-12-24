#include "PreCompile.h"
#include "Clock.h"
#include "CheckHR.h"

namespace WindowsCommon
{

static LONGLONG milliseconds_from_seconds(LONGLONG seconds)
{
    return seconds * 1000;
}

Clock::Clock() noexcept
{
    m_last_counter.QuadPart = 0;

    BOOL supported = QueryPerformanceFrequency(&m_frequency);

    // http://msdn.microsoft.com/en-us/library/windows/desktop/ms644905(v=vs.85).aspx
    UNREFERENCED_PARAMETER(supported);
    assert(supported);  // On WinXP+, QueryPerformanceFrequency will always succeed (per MSDN).

    supported = QueryPerformanceCounter(&m_last_counter);

    // http://msdn.microsoft.com/en-us/library/windows/desktop/ms644904(v=vs.85).aspx
    UNREFERENCED_PARAMETER(supported);
    assert(supported);  // On WinXP+, QueryPerformanceCounter will always succeed (per MSDN).
}

float Clock::ellapsed_milliseconds() noexcept
{
    LARGE_INTEGER current_counter;
    const BOOL supported = QueryPerformanceCounter(&current_counter);

    // http://msdn.microsoft.com/en-us/library/windows/desktop/ms644904(v=vs.85).aspx
    UNREFERENCED_PARAMETER(supported);
    assert(supported);  // On WinXP+, QueryPerformanceCounter will always succeed (per MSDN).

    LARGE_INTEGER counter_diff;
    counter_diff.QuadPart = current_counter.QuadPart - m_last_counter.QuadPart;
    m_last_counter = current_counter;

    const float elapsed_milliseconds = milliseconds_from_seconds(counter_diff.QuadPart) / static_cast<float>(m_frequency.QuadPart);
    return elapsed_milliseconds;
}

}

