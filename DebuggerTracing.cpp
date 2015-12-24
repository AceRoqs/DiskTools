#include "PreCompile.h"
#include "DebuggerTracing.h"            // Pick up forward declarations to ensure correctness.
#include <PortableRuntime/Unicode.h>

namespace WindowsCommon
{

// This is a fast version of debugger_dprintf_utf8, which sacrifices
// UTF-8 correctness for speed.  In most cases, this is fine, as the tracing
// strings are internal and controlled, and speed is more important than
// correctness for this type of debugging aid.
void debugger_dprintf_fast(_In_z_ const char* output_string) noexcept
{
    OutputDebugStringA(output_string);
}

// Note that it is not possible to ensure that the debugging tool that is capturing
// the traces has the fonts to display all required UTF-8 characters.
void debugger_dprintf_utf8(_In_z_ const char* output_string) noexcept
{
    OutputDebugStringW(PortableRuntime::utf16_from_utf8(output_string).c_str());
}

}

