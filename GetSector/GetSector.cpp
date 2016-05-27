#include "PreCompile.h"
#include <DiskTools/DirectRead.h>
#include <WindowsCommon/CheckHR.h>
#include <WindowsCommon/DebuggerTracing.h>
#include <PortableRuntime/CheckException.h>
#include <PortableRuntime/Tracing.h>
#include <PortableRuntime/Unicode.h>

#if 1
// TODO: 2016: Temp for prototyping.
namespace WindowsCommon {
std::vector<std::string> args_from_argv(int argc, _In_reads_(argc) wchar_t** argv)
{
    std::vector<std::string> args;

    std::for_each(argv + 1, argv + argc, [&args](PCWSTR arg)
    {
        args.push_back(PortableRuntime::utf8_from_utf16(arg));
    });

    return args;
}
}
#endif

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

struct Argument_descriptor
{
    std::string long_name;
    char short_name;
    bool requires_parameter;
};

static std::string argument_name_from_long_name(const std::string& long_name)
{
    // Strip leading "--" from long_name.
    return long_name.substr(2, std::string::npos);
}

// Allows arguments to be specified more than once, with the last argument to take priority.
// Output is a map from ID to parameter (or "true" if no parameter required).
// Only arguments passed in the argument_map are allowed.
// Parameter validation must be done by client, as required parameters might have complex invariants,
// such as mutual exclusion, which cannot easily be represented in a table.
std::unordered_map<unsigned int, std::string> options_from_allowed_args(const std::vector<std::string>& arguments, const std::unordered_map<unsigned int, Argument_descriptor>& argument_map)
{
    std::unordered_map<unsigned int, std::string> options;

    for(auto argument = std::cbegin(arguments); argument != std::cend(arguments); ++argument)
    {
        CHECK_ERROR((argument->length() >= 2) && ((*argument)[0] == u8'-'), u8"Invalid argument: " + *argument);

        unsigned int key;
        if((*argument)[1] == u8'-')
        {
            // Handle long arguments, which are multi-character arguments prefixed with "--".
            const std::string argument_name = argument_name_from_long_name(*argument);
            const auto& descriptor = std::find_if(std::cbegin(argument_map), std::cend(argument_map), [&argument_name](const std::pair<unsigned int, Argument_descriptor>& descriptor)
            {
                return argument_name == descriptor.second.long_name;
            });

            // Validate that the argument was found in the passed in argument_map.
            CHECK_ERROR(descriptor != std::cend(argument_map), u8"Invalid argument: " + *argument);

            // Get the key.
            key = descriptor->first;
        }
        else
        {
            // Handle single character arguments, which are single character arguments prefixed with '-'.
            CHECK_ERROR(argument->length() == 2, u8"Invalid argument: " + *argument);

            const char argument_character = (*argument)[1];
            const auto& descriptor = std::find_if(std::cbegin(argument_map), std::cend(argument_map), [argument_character](const std::pair<unsigned int, Argument_descriptor>& descriptor)
            {
                return argument_character == descriptor.second.short_name;
            });

            // Validate that the argument was found in the passed in argument_map.
            CHECK_ERROR(descriptor != std::cend(argument_map), u8"Invalid argument: " + *argument);

            // Get the key.
            key = descriptor->first;
        }

        // Get parameter for the argument.
        const bool requires_parameter = argument_map.at(key).requires_parameter;
        if(requires_parameter)
        {
            CHECK_ERROR((argument + 1) != std::cend(arguments), u8"Argument missing required parameter: " + *argument);
            ++argument;

            options[key] = *argument;
        }
        else
        {
            options[key] = u8"true";
        }
    }

    return options;
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

        enum
        {
            Argument_logical_sector = 0,
            Argument_file_name,
            Argument_help,
        };

        std::unordered_map<unsigned int, Argument_descriptor> argument_map =
        {
            // TODO: 2016: help/version should be automatically generated.
            // TODO: 2016: Consider how this might output to stdout instead of to a file.
            // TODO: 2016: Provide table validation functions for debugging.
            { Argument_logical_sector, { u8"logical-sector", u8's', true  } },
            { Argument_file_name,      { u8"file-name",      u8'f', true  } },
            { Argument_help,           { u8"help",           u8'h', false } },
        };

        const auto arguments = WindowsCommon::args_from_argv(argc, argv);
        const auto options = options_from_allowed_args(arguments, argument_map);

        if(options.count(Argument_help) == 0)
        {
            CHECK_EXCEPTION(options.count(Argument_logical_sector) > 0, u8"Missing a required argument: --" + argument_map[Argument_logical_sector].long_name);
            CHECK_EXCEPTION(options.count(Argument_file_name) > 0,      u8"Missing a required argument: --" + argument_map[Argument_file_name].long_name);

            // TODO: 2016: Encapsulate this into a "get_int" function, etc.
            // There is no _wtoui64 function (and perhaps a private implementation is a good idea), but
            // reading an int64_t into a uint64_t will have no negative (ha!) consequences, as any sector
            // number is considered a valid sector to read.
            const uint64_t sector_number = _atoi64(options.at(Argument_logical_sector).c_str());
            GetSector::read_physical_drive_sector_to_file(0, sector_number, PortableRuntime::utf16_from_utf8(options.at(Argument_file_name)).c_str());
        }
        else
        {
            // TODO: 2016: Fix help text (and autogenerate).
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

