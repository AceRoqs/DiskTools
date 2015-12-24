#pragma once

namespace WindowsCommon
{

void debugger_dprintf_fast(_In_z_ const char* output_string) noexcept;
void debugger_dprintf_utf8(_In_z_ const char* output_string) noexcept;

}

