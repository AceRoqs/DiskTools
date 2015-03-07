#pragma once

namespace PlatformServices
{

std::vector<std::string> get_utf8_args(int argc, _In_reads_(argc) char** argv);
int fprintf_utf8(_In_ FILE* stream, _In_z_ const char* format, ...);

}

