/*
Copyright (C) 2000-2011 by Toby Jones.

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

#ifndef DIRECTREAD_H
#define DIRECTREAD_H

//---------------------------------------------------------------------------
const unsigned int partition_table_entry_count = 4;

#pragma pack(push, 1)
struct Partition_table_entry
{
    uint8_t bootable;
    uint8_t begin_head;
    uint8_t begin_sector;
    uint8_t begin_cylinder;
    uint8_t file_system_type;
    uint8_t end_head;
    uint8_t end_sector;
    uint8_t end_cylinder;
    uint32_t start_sector;
    uint32_t sectors;
};
#pragma pack(pop)

extern PCTSTR cdrom_0;
extern PCTSTR physical_disk_0;
extern PCTSTR physical_disk_1;

//---------------------------------------------------------------------------
PCTSTR get_file_system_name(uint8_t file_system_type);
bool is_extended_partition(uint8_t file_system_type);

HANDLE get_disk_handle(uint8_t disk_number);
HRESULT read_sector_from_handle(
    _Out_cap_post_count_(*buffer_size, *buffer_size) uint8_t* buffer,
    _Inout_ unsigned int* buffer_size,
    _In_ HANDLE handle,
    uint64_t sector_number);
HRESULT read_sector_from_disk(
    _Out_cap_post_count_(*buffer_size, *buffer_size) uint8_t* buffer,
    _Inout_ unsigned int* buffer_size,
    uint8_t disk_number,
    uint64_t sector_number);

#endif

