#include "PreCompile.h"
#include "CheckHR.h"    // Pick up forward declarations to ensure correctness.
#include <PortableRuntime/Tracing.h>

namespace WindowsCommon
{

HRESULT_exception::HRESULT_exception(HRESULT hr) NOEXCEPT : HRESULT_exception(hr, nullptr)
{
}

HRESULT_exception::HRESULT_exception(HRESULT hr, _In_opt_z_ const char* message) NOEXCEPT : m_hr(hr), m_error_string(nullptr)
{
#ifdef _D3D9_H_
    // D3D errors should use D3D9_exception.
    assert(HRESULT_FACILITY(m_hr) != _FACD3D);
#endif

    const size_t message_length = message == nullptr ? 0 : strlen(message);

    WCHAR system_message[128];
    if(0 == FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                           nullptr,
                           m_hr,
                           0,
                           system_message,
                           ARRAYSIZE(system_message),
                           nullptr))
    {
        StringCchCopyW(system_message, ARRAYSIZE(system_message), L"Unknown");
    }

    WCHAR wide_error_string[1024];
    StringCchPrintfW(wide_error_string, ARRAYSIZE(wide_error_string), L"Error: %08x: %s", m_hr, system_message);
    const int byte_count = WideCharToMultiByte(CP_UTF8, 0, wide_error_string, -1, nullptr, 0, nullptr, nullptr);

    // Bound message_length to something reasonable to mitigate integer overflow potential on operator new.
    if((byte_count > 0) && (message_length < ARRAYSIZE(wide_error_string)))
    {
        std::unique_ptr<char[]> utf8_error_string(new(std::nothrow) char[byte_count + message_length]);
        if(utf8_error_string != nullptr)
        {
            if(message != nullptr)
            {
                strcpy_s(utf8_error_string.get(), byte_count + message_length, message);
            }

            if(WideCharToMultiByte(CP_UTF8, 0, wide_error_string, -1, utf8_error_string.get() + message_length, byte_count, nullptr, nullptr) == byte_count)
            {
                std::swap(utf8_error_string, m_error_string);
            }
            else
            {
                PortableRuntime::dprintf("Invalid conversion in exception: %08x.\n", hresult_from_last_error());
            }
        }
        else
        {
            PortableRuntime::dprintf("Buffer allocation failed in exception.\n");
        }
    }
    else
    {
        PortableRuntime::dprintf("Invalid conversion in exception: %08x.\n", hresult_from_last_error());
    }
}

HRESULT_exception::HRESULT_exception(const HRESULT_exception& that) NOEXCEPT
{
    *this = that;
}

HRESULT_exception& HRESULT_exception::operator=(const HRESULT_exception& that) NOEXCEPT
{
    if(this != &that)
    {
        m_hr = that.m_hr;

        const size_t buffer_size = strlen(that.m_error_string.get()) + 1;
        std::unique_ptr<char[]> error_string(new(std::nothrow) char[buffer_size]);
        if(error_string != nullptr)
        {
            strcpy_s(error_string.get(), buffer_size, that.m_error_string.get());
            std::swap(error_string, m_error_string);
        }
        else
        {
            PortableRuntime::dprintf("Buffer allocation failed in exception.\n");
        }
    }

    return *this;
}

// TODO: Keep this function until all projects have been converted away from it (particularly the Direct3D Exception class).
void HRESULT_exception::get_error_string(_Out_writes_z_(size) PTSTR error_string, size_t size) const NOEXCEPT
{
#ifdef _D3D9_H_
    // D3D errors should use D3D9_exception.
    assert(HRESULT_FACILITY(m_hr) != _FACD3D);
#endif

    // FORMAT_MESSAGE_IGNORE_INSERTS is required as arbitrary system messages may contain inserts.
    TCHAR message[128];
    if(0 == FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                          nullptr,
                          m_hr,
                          0,
                          message,
                          ARRAYSIZE(message),
                          nullptr))
    {
        StringCchCopy(message, ARRAYSIZE(message), TEXT("Unknown"));
    }

    StringCchPrintf(error_string, size, TEXT("Error: %08x: %s"), m_hr, message);
}

const char* HRESULT_exception::what() const NOEXCEPT
{
    return m_error_string != nullptr ? m_error_string.get() : std::exception::what();
}

HRESULT hresult_from_last_error() NOEXCEPT
{
    DWORD error = GetLastError();
    HRESULT hr = HRESULT_FROM_WIN32(error);
    assert(FAILED(hr));

    return hr;
}

void check_hr(HRESULT hr)
{
    assert(SUCCEEDED(hr));
    if(FAILED(hr))
    {
        assert(false);
        throw HRESULT_exception(hr);
    }
}

void check_windows_error(BOOL result)
{
    assert(result);
    if(!result)
    {
        HRESULT hr = hresult_from_last_error();
        assert(FAILED(hr));
        throw HRESULT_exception(hr);
    }
}

// TODO: This should go away when an extended check_exception is introduced.
void check_with_custom_hr(BOOL result, HRESULT hr)
{
    assert(result);
    if(!result)
    {
        assert(FAILED(hr));
        throw HRESULT_exception(hr);
    }
}

} // namespace WindowsCommon

