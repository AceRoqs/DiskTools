/*
Copyright (C) 2000-2014 by Toby Jones.

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

// This program prints the partition table information for the first.
// fixed disk.  For Windows NT and XP, administrator privileges are
// required, and for Vista and above, the program needs to be elevated.
// Getting a handle to a disk requires Windows NT 4.0 or better.

#include <cstdint>
#include <array>
#include <tchar.h>
#include <windows.h>
#include "directread.h"

//---------------------------------------------------------------------------
// Display partition table data to stdout.
void output_partition_table_info(
    _In_count_c_(partition_table_entry_count) const Partition_table_entry* entry,
    unsigned int sector_size)
{
    for(unsigned int index = 0; index < partition_table_entry_count; ++index)
    {
        uint64_t part_size = static_cast<uint64_t>(entry[index].sectors) * sector_size;

        _tprintf(_TEXT("Partition %d:\r\n"), index);
        _tprintf(_TEXT(" Bootable: %s\r\n"), entry[index].bootable ? _TEXT("Yes") : _TEXT("No"));

        PCTSTR file_system_name = get_file_system_name(entry[index].file_system_type);
        if(nullptr != file_system_name)
        {
            _tprintf(_TEXT(" File System: %s\r\n"), file_system_name);
        }

        _tprintf(_TEXT(" Begin Head: %u\r\n"),      entry[index].begin_head);
        _tprintf(_TEXT(" Begin Cylinder: %u\r\n"),  entry[index].begin_cylinder);
        _tprintf(_TEXT(" Begin Sector: %u\r\n"),    entry[index].begin_sector);
        _tprintf(_TEXT(" End Head: %u\r\n"),        entry[index].end_head);
        _tprintf(_TEXT(" End Cylinder: %u\r\n"),    entry[index].end_cylinder);
        _tprintf(_TEXT(" End Sector: %u\r\n"),      entry[index].end_sector);
        _tprintf(_TEXT(" Start Sector: %u\r\n"),    entry[index].start_sector);
        _tprintf(_TEXT(" Sectors: %u\r\n"),         entry[index].sectors);
        _tprintf(_TEXT(" Size of partition: %I64u bytes\r\n\r\n"), part_size);
    }
}

//---------------------------------------------------------------------------
// _In_count_(argc) is the correct SAL annotation, but argc is not defined.
int _tmain(int, _In_ PTSTR*)
{
    // Fixed disks with partition tables will generally have a sector
    // size of 512 bytes (valid as of 2011).
    const unsigned int sector_size = 512;

    std::array<uint8_t, sector_size> buffer;
    unsigned int bytes_to_read = sector_size;

    // ERRORLEVEL zero is the success code.
    int error_level = 1;

    HRESULT hr = read_sector_from_disk(buffer.data(), &bytes_to_read, 0, 0);
    if(SUCCEEDED(hr))
    {
        if(sector_size == bytes_to_read)
        {
            // Final two bytes are a boot sector signature, and the partition table immediately preceeds it.
            unsigned int table_start = sector_size - 2 - (sizeof(Partition_table_entry) * partition_table_entry_count);
            auto entries = reinterpret_cast<Partition_table_entry*>(buffer.data() + table_start);
            output_partition_table_info(entries, sector_size);

            error_level = 0;
        }
        else
        {
            _ftprintf(stderr, _TEXT("Sector size is smaller than expected.  Can't find the partition table.\r\n"));
        }
    }
    else
    {
        if(__HRESULT_FROM_WIN32(ERROR_OPEN_FAILED) == hr)
        {
            _ftprintf(stderr, _TEXT("Error opening physical device. Are you administrator?\r\n"));
        }
        else if(TYPE_E_BUFFERTOOSMALL == hr)
        {
            _ftprintf(stderr, _TEXT("Buffer is too small. Sector size is larger than expected.\r\n"));
        }
        else if(__HRESULT_FROM_WIN32(ERROR_READ_FAULT) == hr)
        {
            _ftprintf(stderr, _TEXT("Error reading physical disk.\r\n"));
        }
        else
        {
            _ftprintf(stderr, _TEXT("Unexpected error occured.\r\n"));
        }
    }

    return error_level;
}

