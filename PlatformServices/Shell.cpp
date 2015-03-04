#include "PreCompile.h"
#include "Shell.h"          // Pick up forward declarations to ensure correctness.
#include <PortableRuntime/Unicode.h>

#ifdef WIN32
#include <WindowsCommon/CheckHR.h>
#include <WindowsCommon/ScopedWindowsTypes.h>
#endif

namespace PlatformServices
{

std::vector<std::string> get_utf8_args(int argc, _In_reads_(argc) char** argv)
{
    std::vector<std::string> args;

#ifdef WIN32
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    const auto command_line = GetCommandLineW();

    int arg_count;
    const auto naked_args = CommandLineToArgvW(command_line, &arg_count);
    CHECK_WINDOWS_ERROR(naked_args != nullptr);
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

