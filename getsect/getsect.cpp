/*
Copyright (C) 1999-2014 by Toby Jones.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#define _CRT_SECURE_NO_WARNINGS
#include <cassert>
#include <cstdint>
#include <array>
#include <memory>
#include <tchar.h>
#include <windows.h>
#include "directread.h"

//---------------------------------------------------------------------------
int _tmain(int argc, _In_count_(argc) PTSTR* argv)
{
    const unsigned int arg_program_name = 0;
    const unsigned int arg_sector       = 1;
    const unsigned int arg_output_file  = 2;

    const unsigned int sector_size = 512;

    std::array<uint8_t, sector_size> buffer;
    assert(buffer.size() < UINT_MAX);
    unsigned int buffer_size = static_cast<unsigned int>(buffer.size());

    if(3 != argc)
    {
        _tprintf(_TEXT("Usage: %s sector filename\r\n"), argv[arg_program_name]);
        _tprintf(_TEXT("To read MBR: %s 1 mbr.bin\r\n"), argv[arg_program_name]);
        return 0;
    }

    // ERRORLEVEL zero is the success code.
    int error_level = 0;

    if(SUCCEEDED(read_sector_from_disk(buffer.data(), &buffer_size, 0, _ttoi64(argv[arg_sector]))))
    {
        std::unique_ptr<FILE, int (*)(FILE*)> file(_tfopen(argv[arg_output_file], _TEXT("wb")), std::fclose);
        if(0 == file)
        {
            _ftprintf(stderr, _TEXT("%s: error opening %s.\r\n"), argv[arg_program_name], argv[arg_output_file]);
            error_level = 1;
        }
        else
        {
            if(std::fwrite(buffer.data(), 1, buffer_size, file.get()) != buffer_size)
            {
                _ftprintf(stderr, _TEXT("%s: error writing %s.\r\n"), argv[arg_program_name], argv[arg_output_file]);
                error_level = 1;
            }
        }
    }
    else
    {
        _ftprintf(stderr, _TEXT("%s: error reading sector.\r\n"), argv[arg_program_name]);
        error_level = 1;
    }

    return error_level;
}

