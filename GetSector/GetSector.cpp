#include "PreCompile.h"
#include <DiskTools/DirectRead.h>
#include <Parsing/CommandLine.h>
#include <WindowsCommon/CheckHR.h>
#include <WindowsCommon/CommandLine.h>
#include <WindowsCommon/DebuggerTracing.h>
#include <PortableRuntime/CheckException.h>
#include <PortableRuntime/Tracing.h>
#include <PortableRuntime/Unicode.h>

// TODO: 2016: Temp for prototyping.
#include <functional>
#include <WindowsCommon/ScopedWindowsTypes.h>

namespace GetSector
{

// TODO: 2016: This should be the replacement for read_sector_from_disk, but it should go in the
// WindowsCommon namespace (as should DiskTools), as it uses exceptions and returns a buffer directly.
static std::vector<uint8_t> read_physical_drive_sector(uint8_t drive_number, uint64_t sector_number)
{
    // TODO: 2016: Get the disk's configured sector size.
    constexpr unsigned int sector_size = 512;

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

    std::ofstream output_file(output_file_name, std::ios::binary | std::ios::trunc);
    CHECK_EXCEPTION(output_file.good(), u8"Error opening: " + PortableRuntime::utf8_from_utf16(output_file_name));

    output_file.write(reinterpret_cast<const char*>(sector.data()), sector.size());
    CHECK_EXCEPTION(!output_file.fail(), u8"Error writing output file.");
}

static int parse_arguments_and_execute()
{
    enum
    {
        Argument_logical_sector = 0,
        Argument_file_name,
        Argument_help,
    };

    const std::vector<Parsing::Argument_descriptor> argument_map =
    {
        { Argument_logical_sector, u8"logical-sector", u8's', true,  u8"The logical block address (LBA) of the sector to read." },
        { Argument_file_name,      u8"file-name",      u8'f', true,  u8"The name of the file to hold the output. This file will be overwritten." },
        { Argument_help,           u8"help",           u8'?', false, nullptr },
    };
#ifndef NDEBUG
    Parsing::validate_argument_map(argument_map);
#endif

    const auto arguments = WindowsCommon::args_from_command_line();
    const auto options = Parsing::options_from_allowed_args(arguments, argument_map);

    int error_level = 0;
    if(options.count(Argument_help) == 0)
    {
        CHECK_EXCEPTION(options.count(Argument_logical_sector) > 0, u8"Missing a required argument: --" + std::string(argument_map[Argument_logical_sector].long_name));
        CHECK_EXCEPTION(options.count(Argument_file_name) > 0,      u8"Missing a required argument: --" + std::string(argument_map[Argument_file_name].long_name));

        // There is no _atoui64 function (and perhaps a private implementation is a good idea), but
        // reading an int64_t into a uint64_t will have no negative (ha!) consequences, as any sector
        // number is considered a valid sector to read.
        const uint64_t sector_number = _atoi64(options.at(Argument_logical_sector).c_str());
        GetSector::read_physical_drive_sector_to_file(0, sector_number, PortableRuntime::utf16_from_utf8(options.at(Argument_file_name)).c_str());
    }
    else
    {
        constexpr auto arg_program_name = 0;

        // Hold a reference to program_name_long for the duration of the output functions.
        const auto program_name_long = PortableRuntime::utf16_from_utf8(arguments[arg_program_name]);
        const auto program_name = PathFindFileNameW(program_name_long.c_str());

        std::fwprintf(stderr, L"Usage: %s [options]\nOptions:\n", program_name);
        std::fwprintf(stderr, PortableRuntime::utf16_from_utf8(Parsing::Options_help_text(argument_map)).c_str());
        std::fwprintf(stderr,
                      L"\nTo read the Master Boot Record:\n  %s -%c 1 -%c mbr.bin\n",
                      program_name,
                      argument_map[Argument_logical_sector].short_name,
                      argument_map[Argument_file_name].short_name);
        error_level = 1;
    }

    return error_level;
}

}

int wmain(int argc, _In_reads_(argc) wchar_t** argv)
{
    (void)argc;     // Unreferenced parameter.
    (void)argv;

    // ERRORLEVEL zero is the success code.
    int error_level;

    // Set outside the try block so error messages use the proper code page.
    // This class does not throw.
    WindowsCommon::UTF8_console_code_page code_page;

    try
    {
        PortableRuntime::set_dprintf(WindowsCommon::debugger_dprintf);

        // Set wprintf output to UTF-8 in Windows console.
        // CHECK_EXCEPTION ensures against the case that the CRT invalid parameter handler
        // routine is set by a global constructor.
        CHECK_EXCEPTION(_setmode(_fileno(stdout), _O_U8TEXT) != -1, u8"Failed to set UTF-8 output mode.");
        CHECK_EXCEPTION(_setmode(_fileno(stderr), _O_U8TEXT) != -1, u8"Failed to set UTF-8 output mode.");

        assert(WindowsCommon::args_from_command_line().size() == (static_cast<size_t>(argc)));
        error_level = GetSector::parse_arguments_and_execute();
    }
    catch(const std::exception& ex)
    {
        std::fwprintf(stderr, L"\n%s\n", PortableRuntime::utf16_from_utf8(ex.what()).c_str());
        error_level = 1;
    }

    return error_level;
}

