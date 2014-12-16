#include "PreCompile.h"
#include "DirectRead.h"

int _tmain(int argc, _In_count_(argc) PTSTR* argv)
{
    const unsigned int arg_program_name = 0;
    const unsigned int arg_output_file  = 1;

    // ERRORLEVEL zero is the success code.
    int error_level = 0;

    if(2 != argc)
    {
        _tprintf(_TEXT("Usage: %s filename.iso\r\n"), argv[arg_program_name]);
        return 0;
    }

    auto handle_deleter = [](HANDLE handle)
    {
        if(INVALID_HANDLE_VALUE != handle)
        {
            ::CloseHandle(handle);
        }
    };

    std::unique_ptr<void, std::function<void (HANDLE handle)>> disk_handle(
        ::CreateFile(DiskTools::get_filename_cdrom_0(),
                     GENERIC_READ,
                     FILE_SHARE_READ,
                     nullptr,
                     OPEN_EXISTING,
                     FILE_ATTRIBUTE_NORMAL,
                     nullptr),
        handle_deleter);

    std::unique_ptr<void, std::function<void (HANDLE handle)>> output_file(
        ::CreateFile(argv[arg_output_file],
                     GENERIC_WRITE,
                     0,
                     nullptr,
                     CREATE_ALWAYS,
                     FILE_ATTRIBUTE_NORMAL,
                     nullptr),
        handle_deleter);

    if((disk_handle.get() != INVALID_HANDLE_VALUE) && (output_file.get() != INVALID_HANDLE_VALUE))
    {
        DWORD bytes_returned;
        GET_LENGTH_INFORMATION length_information;
        if(::DeviceIoControl(disk_handle.get(),
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
                    if(::ReadFile(disk_handle.get(), buffer.get(), amount_to_read, &amount_read, nullptr) == 0)
                    {
                        _ftprintf(stderr, _TEXT("Error reading disk (%u).\r\n"), ::GetLastError());
                        error_level = 1;
                        break;
                    }

                    if(::WriteFile(output_file.get(), buffer.get(), amount_read, &amount_read, nullptr) == 0)
                    {
                        _ftprintf(stderr, _TEXT("Error writing file (%u).\r\n"), ::GetLastError());
                        error_level = 1;
                        break;
                    }

                    bytes_left -= amount_to_read;
                }
            }
            catch(const std::bad_alloc&)
            {
                _ftprintf(stderr, _TEXT("Not enough memory to allocate the transfer buffer.\r\n"));
                error_level = 1;
            }
        }
        else
        {
            _ftprintf(stderr, _TEXT("Unexpected error occured (%u).\r\n"), ::GetLastError());
            error_level = 1;
        }
    }
    else
    {
        _ftprintf(stderr, _TEXT("Unable to open input or output device.\r\n"));
        error_level = 1;
    }

    return error_level;
}

