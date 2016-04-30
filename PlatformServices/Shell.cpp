#include "PreCompile.h"
#include "Shell.h"          // Pick up forward declarations to ensure correctness.
#include <PortableRuntime/Tracing.h>
#include <PortableRuntime/Unicode.h>

#if defined(_WIN32) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#include <WindowsCommon/CheckHR.h>
#include <WindowsCommon/ScopedWindowsTypes.h>
#endif

namespace PlatformServices
{

std::vector<std::string> get_utf8_args(int argc, _In_reads_(argc) char** argv)
{
    std::vector<std::string> args;

#if defined(_WIN32) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
    (void)argc;     // Unreferenced parameter.
    (void)argv;
    PortableRuntime::dprintf(u8"Ignoring parameters: argc and argv: Using Unicode arguments instead.\n");

    const auto command_line = GetCommandLineW();

    int arg_count;
    const auto naked_args = CommandLineToArgvW(command_line, &arg_count);
    CHECK_BOOL_LAST_ERROR(naked_args != nullptr);
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

// TODO: 2016: Future plans below.  Not sure this is the correct approach anymore.
#if 0
int fprintf_utf8(_In_ FILE* stream, _In_z_ const char* format, ...)
{
    (void)stream;
    (void)format;

    return 0;
}
#endif

}

