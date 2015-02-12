#include "PreCompile.h"
#include <DiskTools/DirectRead.h>
#include <PortableRuntime/Unicode.h>
#include <WindowsCommon/CheckHR.h>
#include <WindowsCommon/ScopedWindowsTypes.h>
#include <WindowsCommon/Wrappers.h>

namespace RipISO
{

// TODO: This should eventually go in the PlatformServices library.
std::vector<std::string> get_utf8_args(int argc, _In_reads_(argc) char** argv)
{
    std::vector<std::string> args;

#ifdef WIN32
    UNREFERENCED_PARAMETER(argv);

    const auto command_line = GetCommandLineW();

    int arg_count;
    const auto naked_args = CommandLineToArgvW(command_line, &arg_count);
    WindowsCommon::check_windows_error(naked_args != nullptr);
    assert(argc == arg_count);

    const auto wide_args = WindowsCommon::make_scoped_local(naked_args);

    std::for_each(naked_args, naked_args + arg_count, [&args](PCWSTR arg)
    {
        args.push_back(PortableRuntime::utf8_from_utf16(arg));
    });
#else
    std::for_each(argv, argv + argc, [&args](char* arg)
    {
        args.push_back(arg);
    });
#endif

    return args;
}

}

int main(int argc, _In_reads_(argc) char** argv)
{
    const unsigned int arg_program_name = 0;
    const unsigned int arg_output_file  = 1;

    // ERRORLEVEL zero is the success code.
    int error_level = 0;

    // TODO: Consider try/catch at this scope.
    const auto args = RipISO::get_utf8_args(argc, argv);

    if(2 != args.size())
    {
        printf("Usage: %s file_name.iso\n", args[arg_program_name].c_str());
        return 0;
    }

    try
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
            args[arg_output_file].c_str(),
            GENERIC_WRITE,
            0,
            nullptr,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);

        DWORD bytes_returned;
        GET_LENGTH_INFORMATION length_information;
        if(DeviceIoControl(disk_handle,
                           IOCTL_DISK_GET_LENGTH_INFO,
                           nullptr,
                           0,
                           &length_information,
                           sizeof(length_information),
                           &bytes_returned,
                           nullptr) != 0)
        {
            try
            {
                const unsigned int buffer_size = 1024 * 1024;
                std::unique_ptr<uint8_t> buffer(new uint8_t[buffer_size]);

                ULONGLONG bytes_left = length_information.Length.QuadPart;
                while(bytes_left > 0)
                {
                    // Cast is safe as buffer_size is less than MAX_DWORD.
                    DWORD amount_to_read = bytes_left > buffer_size ? buffer_size : static_cast<DWORD>(bytes_left);

                    // This is reasonably slow, but it is fast enough for single CDs or DVDs.
                    // A fast approach might be to use uncached aligned async reads, at the
                    // expense of considerable complexity.
                    DWORD amount_read;
                    if(ReadFile(disk_handle, buffer.get(), amount_to_read, &amount_read, nullptr) == 0)
                    {
                        _ftprintf(stderr, _TEXT("Error reading disk (%u).\r\n"), GetLastError());
                        error_level = 1;
                        break;
                    }

                    if(WriteFile(output_file, buffer.get(), amount_read, &amount_read, nullptr) == 0)
                    {
                        _ftprintf(stderr, _TEXT("Error writing file (%u).\r\n"), GetLastError());
                        error_level = 1;
                        break;
                    }

                    bytes_left -= amount_to_read;
                }
            }
            catch(const std::bad_alloc& ex)
            {
                (ex);
                _ftprintf(stderr, _TEXT("Not enough memory to allocate the transfer buffer.\r\n"));
                error_level = 1;
            }
        }
        else
        {
            _ftprintf(stderr, _TEXT("Unexpected error occured (%u).\r\n"), GetLastError());
            error_level = 1;
        }
    }
    catch(...)
    {
        _ftprintf(stderr, _TEXT("Unable to open input or output device.\r\n"));
        error_level = 1;
    }

    return error_level;
}

