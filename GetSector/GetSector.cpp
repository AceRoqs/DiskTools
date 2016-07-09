#include "PreCompile.h"
#include <DiskTools/DirectRead.h>
#include <WindowsCommon/CheckHR.h>
#include <WindowsCommon/DebuggerTracing.h>
#include <PortableRuntime/CheckException.h>
#include <PortableRuntime/Tracing.h>
#include <PortableRuntime/Unicode.h>

// TODO: 2016: Temp for prototyping.
#include <functional>
#include <WindowsCommon/ScopedWindowsTypes.h>

namespace WindowsCommon {
std::vector<std::string> args_from_command_line()
{
    std::vector<std::string> args;

    const auto command_line = GetCommandLineW();

    int arg_count;
    const auto naked_args = CommandLineToArgvW(command_line, &arg_count);
    CHECK_BOOL_LAST_ERROR(naked_args != nullptr);

    const auto wide_args = WindowsCommon::make_scoped_local(naked_args);

    std::for_each(naked_args + 1, naked_args + arg_count, [&args](PCWSTR arg)
    {
        args.push_back(PortableRuntime::utf8_from_utf16(arg));
    });

    return args;
}

}

// TODO: 2016: Argument parsing should go in some sort of parsing library.
// PortableRuntime should be for cross-cutting concerns - error handling, tracing, etc.
namespace PortableRuntime {

#if 0
std::vector<std::string> args_from_argv(int argc, _In_reads_(argc) wchar_t** argv)
{
    std::vector<std::string> args;

    std::for_each(argv + 1, argv + argc, [&args](PCWSTR arg)
    {
        args.push_back(PortableRuntime::utf8_from_utf16(arg));
    });

    return args;
}
#endif

struct Argument_descriptor
{
    unsigned int kind;
    const char* long_name;
    char short_name;
    bool requires_parameter;
    const char* description;
};

static std::string argument_name_from_long_name(const std::string& long_name)
{
    // Strip leading "--" from long_name.
    return long_name.substr(2, std::string::npos);
}

void validate_argument_map(const std::vector<Argument_descriptor>& argument_map)
{
    const auto size = argument_map.size();
    for(size_t ix = 0; ix < size; ++ix)
    {
        // argument_map kind must match it's index, to guarantee that the enum
        // for the kind also matches the index.  This also guarantees the enum
        // monotonically increases, and that there are no gaps.  This may be
        // helpful for future optimizations.
        assert(argument_map[ix].kind == ix);
    }

    // Validate short name arguments do not dupe.  Arguments are case sensitive.
    static_assert(sizeof(Argument_descriptor::short_name) == 1, "'used' array size depends on short_name being 1 byte.");
    bool used[256] {};
    for(size_t ix = 0; ix < size; ++ix)
    {
        if(argument_map[ix].short_name != 0)
        {
            assert(!used[argument_map[ix].short_name]);
            used[argument_map[ix].short_name] = true;
        }
    }
}

// Allows arguments to be specified more than once, with the last argument to take priority.
// Output is a map from ID to parameter (or "true" if no parameter required).
// Only arguments passed in the argument_map are allowed.
// Parameter validation must be done by client, as required parameters might have complex invariants,
// such as mutual exclusion, which cannot easily be represented in a table.
std::unordered_map<unsigned int, std::string> options_from_allowed_args(const std::vector<std::string>& arguments, const std::vector<Argument_descriptor>& argument_map)
{
    std::unordered_map<unsigned int, std::string> options;

    const auto end = std::cend(arguments);
    for(auto argument = std::cbegin(arguments); argument != end; ++argument)
    {
        CHECK_EXCEPTION((argument->length() >= 2) && ((*argument)[0] == u8'-'), u8"Unrecognized argument: " + *argument);

        std::function<bool(const Argument_descriptor&)> predicate;
        if((*argument)[1] == u8'-')
        {
            // Handle long arguments, which are multi-character arguments prefixed with "--".
            const std::string argument_name = argument_name_from_long_name(*argument);
            predicate = [argument_name](const Argument_descriptor& descriptor)
            {
                return argument_name == descriptor.long_name;
            };
        }
        else
        {
            // Handle single character arguments, which are single character arguments prefixed with '-'.
            CHECK_EXCEPTION(argument->length() == 2, u8"Unrecognized argument: " + *argument);

            const char argument_character = (*argument)[1];
            predicate = [argument_character](const Argument_descriptor& descriptor)
            {
                return argument_character == descriptor.short_name;
            };
        }

        const auto& descriptor = std::find_if(std::cbegin(argument_map), std::cend(argument_map), predicate);

        // Validate that the argument was found in the passed in argument_map.
        CHECK_EXCEPTION(descriptor != std::cend(argument_map), u8"Unrecognized argument: " + *argument);

        // Get the key.
        unsigned int key = descriptor->kind;

        // Get parameter for the argument.
        const bool requires_parameter = argument_map[key].requires_parameter;
        if(requires_parameter)
        {
            CHECK_EXCEPTION((argument + 1) != std::cend(arguments), u8"Argument missing required parameter: " + *argument);
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

std::string Options_help_text(const std::vector<PortableRuntime::Argument_descriptor>& argument_map)
{
    std::string help_text;

    size_t allocation_size = 0;
    size_t tab_index = 0;
    const auto size = argument_map.size();
    for(size_t ix = 0; ix < size; ++ix)
    {
        if(argument_map[ix].description != nullptr)
        {
            const size_t long_name_length = strlen(argument_map[ix].long_name);

            allocation_size += 9;   // "  -X, --\n".
            allocation_size += long_name_length;
            allocation_size += strlen(argument_map[ix].description);

            if(long_name_length > tab_index)
            {
                tab_index = long_name_length;
            }
        }
    }
    tab_index += 2; // Default tab distance.

    help_text.reserve(allocation_size);

    for(size_t ix = 0; ix < size; ++ix)
    {
        if(argument_map[ix].description != nullptr)
        {
            help_text += "  -";
            help_text += argument_map[ix].short_name;
            help_text += ", --";
            help_text += argument_map[ix].long_name;

            size_t space_count = tab_index;
            space_count -= strlen(argument_map[ix].long_name);
            for(size_t count = 0; count < space_count; ++count)
            {
                help_text += " ";
            }
            help_text += argument_map[ix].description;
            help_text += "\n";
        }
    }

    return help_text;
}

}

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

}

int wmain(int argc, _In_reads_(argc) wchar_t** argv)
{
    (void)argc;     // Unreferenced parameter.

    constexpr auto arg_program_name = 0;

    // ERRORLEVEL zero is the success code.
    int error_level = 0;

    try
    {
        WindowsCommon::UTF8_console_code_page code_page;
        PortableRuntime::set_dprintf(WindowsCommon::debugger_dprintf);

        // Set wprintf output to UTF-8 in Windows console.
        // CHECK_EXCEPTION ensures against the case that the CRT invalid parameter handler
        // routine is set by a global constructor.
        CHECK_EXCEPTION(_setmode(_fileno(stdout), _O_U8TEXT) != -1, u8"Failed to set UTF-8 output mode.");
        CHECK_EXCEPTION(_setmode(_fileno(stderr), _O_U8TEXT) != -1, u8"Failed to set UTF-8 output mode.");

        // TODO: 2016: Everything after this should be in its own function.
        enum
        {
            Argument_logical_sector = 0,
            Argument_file_name,
            Argument_help,
        };

        const std::vector<PortableRuntime::Argument_descriptor> argument_map =
        {
            { Argument_logical_sector, u8"logical-sector", u8's', true,  u8"The logical block address (LBA) of the sector to read." },
            { Argument_file_name,      u8"file-name",      u8'f', true,  u8"The name of the file to hold the output. This file will be overwritten." },
            { Argument_help,           u8"help",           u8'?', false, nullptr },
        };
#ifndef NDEBUG
        PortableRuntime::validate_argument_map(argument_map);
#endif

        const auto arguments = WindowsCommon::args_from_command_line();
        assert(arguments.size() == (static_cast<size_t>(argc) - 1));
        const auto options = PortableRuntime::options_from_allowed_args(arguments, argument_map);

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
            std::fwprintf(stderr, L"Usage: %s [options]\nOptions:\n", PathFindFileNameW(argv[arg_program_name]));
            std::fwprintf(stderr, PortableRuntime::utf16_from_utf8(PortableRuntime::Options_help_text(argument_map)).c_str());
            std::fwprintf(stderr,
                          L"\nTo read the Master Boot Record:\n %s -%c 1 -%c mbr.bin\n",
                          PathFindFileNameW(argv[arg_program_name]),
                          argument_map[Argument_logical_sector].short_name,
                          argument_map[Argument_file_name].short_name);
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

