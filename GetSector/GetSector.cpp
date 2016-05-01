#include "PreCompile.h"
#include <DiskTools/DirectRead.h>
#include <WindowsCommon/CheckHR.h>
#include <WindowsCommon/DebuggerTracing.h>
#include <PortableRuntime/CheckException.h>
#include <PortableRuntime/Tracing.h>
#include <PortableRuntime/Unicode.h>

namespace DiskTools
{

void read_physical_drive_sector_to_file(uint8_t drive_number, uint64_t sector_number, _In_z_ const wchar_t* output_file_name);
std::vector<uint8_t> read_physical_drive_sector(uint8_t drive_number, uint64_t sector_number);

}

int wmain(int argc, _In_reads_(argc) wchar_t** argv)
{
    constexpr auto arg_program_name     = 0;
    constexpr auto arg_sector_number    = 1;
    constexpr auto arg_output_file_name = 2;

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

        if(argc == 3)
        {
            // There is no _wtoui64 function (and perhaps a private implementation is a good idea), but
            // reading an int64_t into a uint64_t will have no negative (ha!) consequences, as any sector
            // number is considered a valid sector to read.
            const uint64_t sector_number = _wtoi64(argv[arg_sector_number]);
            DiskTools::read_physical_drive_sector_to_file(0, sector_number, argv[arg_output_file_name]);
        }
        else
        {
            std::fwprintf(stderr, L"Usage: %s sector file_name\n", argv[arg_program_name]);
            std::fwprintf(stderr, L"To read the Master Boot Record: %s 1 mbr.bin\n", argv[arg_program_name]);
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

namespace DiskTools
{

_Use_decl_annotations_
void read_physical_drive_sector_to_file(uint8_t drive_number, uint64_t sector_number, const wchar_t* output_file_name)
{
    std::vector<uint8_t> sector = read_physical_drive_sector(drive_number, sector_number);

    std::basic_ofstream<uint8_t> output_file(output_file_name, std::ios::binary);
    CHECK_EXCEPTION(output_file.good(), u8"Error opening: " + PortableRuntime::utf8_from_utf16(output_file_name));

    output_file.write(sector.data(), sector.size());
    CHECK_EXCEPTION(!output_file.fail(), u8"Error writing output file.");
}

_Use_decl_annotations_
std::vector<uint8_t> read_physical_drive_sector(uint8_t drive_number, uint64_t sector_number)
{
    // TODO: 2016: Get the disk's configured sector size.
    const unsigned int sector_size = 512;

    std::vector<uint8_t> buffer;
    buffer.resize(sector_size);

    unsigned int buffer_size = sector_size;
    const HRESULT hr = read_sector_from_disk(buffer.data(), &buffer_size, drive_number, sector_number);
    CHECK_HR(hr);

    return buffer;
}

}

