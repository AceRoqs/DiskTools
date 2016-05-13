#include "PreCompile.h"
#include <DiskTools/DirectRead.h>
#include <WindowsCommon/CheckHR.h>
#include <WindowsCommon/DebuggerTracing.h>
#include <PortableRuntime/CheckException.h>
#include <PortableRuntime/Tracing.h>
#include <PortableRuntime/Unicode.h>

namespace GetSector
{

// TODO: 2016: This should go in WindowsCommon namespace.
static std::vector<uint8_t> read_physical_drive_sector(uint8_t drive_number, uint64_t sector_number)
{
    // TODO: 2016: Get the disk's configured sector size.
    const unsigned int sector_size = 512;

    std::vector<uint8_t> buffer;
    buffer.resize(sector_size);

    unsigned int buffer_size = sector_size;
    const HRESULT hr = DiskTools::read_sector_from_disk(buffer.data(), &buffer_size, drive_number, sector_number);
    CHECK_HR(hr);

    return buffer;
}

static void read_physical_drive_sector_to_file(uint8_t drive_number, uint64_t sector_number, _In_z_ const wchar_t* output_file_name)
{
    std::vector<uint8_t> sector = read_physical_drive_sector(drive_number, sector_number);

    std::basic_ofstream<uint8_t> output_file(output_file_name, std::ios::binary);
    CHECK_EXCEPTION(output_file.good(), u8"Error opening: " + PortableRuntime::utf8_from_utf16(output_file_name));

    output_file.write(sector.data(), sector.size());
    CHECK_EXCEPTION(!output_file.fail(), u8"Error writing output file.");
}

}

// Inspired by GNU getopt_long.
struct Option
{
    // TODO: 2016: May need option_required as well.
    // TODO: 2016: May need description for help text?
    std::string long_name;
    char short_name;
    bool* flag;
    bool argument_required;
    std::string* argument;      // TODO: 2016: Consider just making each option a lambda.
};

class Argument_iterator
{
public:
    Argument_iterator(std::vector<std::string> arguments, std::vector<Option> options)
    {
    }

    std::string argument()
    {
        return u8"";
    }
};

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

        bool show_usage = false;
        bool show_version = false;
        std::string sector_number_string;
        std::string file_name;
        const static std::vector<Option> options =
        {
            // TODO: 2016: help/version should be automatically generated.
            { u8"help",     u8'h', &show_usage,   false, nullptr },
            { u8"version",  u8'v', &show_version, false, nullptr },
            { u8"sector",   u8's', nullptr,       true,  &sector_number_string },
            { u8"filename", u8'f', nullptr,       true,  &file_name },
        };

        if(argc == 3)
        {
            // There is no _wtoui64 function (and perhaps a private implementation is a good idea), but
            // reading an int64_t into a uint64_t will have no negative (ha!) consequences, as any sector
            // number is considered a valid sector to read.
            const uint64_t sector_number = _wtoi64(argv[arg_sector_number]);
            GetSector::read_physical_drive_sector_to_file(0, sector_number, argv[arg_output_file_name]);
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

