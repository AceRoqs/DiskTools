#include "PreCompile.h"
#include "DebuggerTracing.h"

namespace WindowsCommon
{

void debugger_dprintf(_In_z_ const char* output_string) NOEXCEPT
{
    OutputDebugStringA(output_string);
}

}

