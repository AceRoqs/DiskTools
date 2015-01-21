#include "PreCompile.h"
#include <DiskTools/DirectRead.h>

int _tmain(int argc, _In_reads_(argc) PTSTR* argv)
{
    const unsigned int arg_program_name = 0;
    const unsigned int arg_sector       = 1;
    const unsigned int arg_output_file  = 2;

    const unsigned int sector_size = 512;

    std::array<uint8_t, sector_size> buffer;
    buffer.data();  // No-op for static analysis.  MSVC analyze complains about usage before init.
    assert(buffer.size() < UINT_MAX);
    unsigned int buffer_size = static_cast<unsigned int>(buffer.size());

    if(3 != argc)
    {
        _tprintf(_TEXT("Usage: %s sector filename\r\n"), argv[arg_program_name]);
        _tprintf(_TEXT("To read MBR: %s 1 mbr.bin\r\n"), argv[arg_program_name]);
        return 0;
    }

    // ERRORLEVEL zero is the success code.
    int error_level = 0;

    if(SUCCEEDED(DiskTools::read_sector_from_disk(buffer.data(), &buffer_size, 0, _ttoi64(argv[arg_sector]))))
    {
        std::unique_ptr<FILE, int (*)(FILE*)> file(_tfopen(argv[arg_output_file], _TEXT("wb")), std::fclose);
        if(0 == file)
        {
            _ftprintf(stderr, _TEXT("%s: error opening %s.\r\n"), argv[arg_program_name], argv[arg_output_file]);
            error_level = 1;
        }
        else
        {
            if(std::fwrite(buffer.data(), 1, buffer_size, file.get()) != buffer_size)
            {
                _ftprintf(stderr, _TEXT("%s: error writing %s.\r\n"), argv[arg_program_name], argv[arg_output_file]);
                error_level = 1;
            }
        }
    }
    else
    {
        _ftprintf(stderr, _TEXT("%s: error reading sector.\r\n"), argv[arg_program_name]);
        error_level = 1;
    }

    return error_level;
}

