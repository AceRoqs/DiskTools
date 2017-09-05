#include "PreCompile.h"
#include <DiskTools/DirectRead.h>
#include <WindowsCommon/CheckHR.h>
#include <WindowsCommon/DebuggerTracing.h>
#include <WindowsCommon/ScopedWindowsTypes.h>
#include <WindowsCommon/Wrappers.h>
#include <PortableRuntime/CheckException.h>
#include <PortableRuntime/Tracing.h>
#include <PortableRuntime/Unicode.h>
#include <PlatformServices/Shell.h>

namespace RipISO
{
    void rip_iso(_In_z_ const char* output_file_name)
    {
        const auto disk_handle = WindowsCommon::create_file(
            DiskTools::get_file_name_cdrom_0(),
            GENERIC_READ,
            FILE_SHARE_READ,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);

        const auto output_file = WindowsCommon::create_file(
            output_file_name,
            GENERIC_WRITE,
            0,
            nullptr,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);

        DWORD bytes_returned;
        GET_LENGTH_INFORMATION length_information;
        CHECK_BOOL_LAST_ERROR(DeviceIoControl(disk_handle,
                                              IOCTL_DISK_GET_LENGTH_INFO,
                                              nullptr,
                                              0,
                                              &length_information,
                                              sizeof(length_information),
                                              &bytes_returned,
                                              nullptr) != 0);
        constexpr unsigned int buffer_size = 1024 * 1024;
        // TODO: 2016: This should be a std::vector.
        std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(buffer_size);

        ULONGLONG bytes_left = length_information.Length.QuadPart;
        while(bytes_left > 0)
        {
            // Cast is safe as buffer_size is less than MAX_DWORD.
            DWORD amount_to_read = bytes_left > buffer_size ? buffer_size : static_cast<DWORD>(bytes_left);

            // This is reasonably slow, but it is fast enough for single CDs or DVDs.
            // A fast approach might be to use uncached aligned async reads, at the
            // expense of considerable complexity.
            DWORD amount_read;
            CHECK_BOOL_LAST_ERROR(ReadFile(disk_handle, buffer.get(), amount_to_read, &amount_read, nullptr) != 0);
            CHECK_BOOL_LAST_ERROR(WriteFile(output_file, buffer.get(), amount_read, &amount_read, nullptr) != 0);

            bytes_left -= amount_to_read;
        }
    }
}

int main(int argc, _In_reads_(argc) char** argv)
{
    // ERRORLEVEL zero is the success code.
    int error_level = 0;

    try
    {
        PortableRuntime::set_dprintf(WindowsCommon::debugger_dprintf);

        // Set wprintf output to UTF-8 in Windows console.
        // CHECK_EXCEPTION ensures against the case that the CRT invalid parameter handler
        // routine is set by a global constructor.
        CHECK_EXCEPTION(_setmode(_fileno(stdout), _O_U8TEXT) != -1, u8"Failed to set UTF-8 output mode.");
        CHECK_EXCEPTION(_setmode(_fileno(stderr), _O_U8TEXT) != -1, u8"Failed to set UTF-8 output mode.");

        constexpr unsigned int arg_program_name = 0;
        constexpr unsigned int arg_output_file  = 1;

        const auto args = PlatformServices::get_utf8_args(argc, argv);
        if(args.size() == 2)
        {
            RipISO::rip_iso(args[arg_output_file].c_str());
        }
        else
        {
            std::fwprintf(stderr, L"Usage: %s file_name.iso", PortableRuntime::utf16_from_utf8(args[arg_program_name]).c_str());
            error_level = 1;
        }
    }
    catch(const std::exception& ex)
    {
        std::fwprintf(stderr, L"\n%s\n", PortableRuntime::utf16_from_utf8(ex.what()).c_str());
        error_level = 1;
    }

    return error_level;
}

