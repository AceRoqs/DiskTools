#include <Windows.h>
#include <tchar.h>
#include <cstdint>

#pragma pack(push, 1)
struct Bios_parameter_block
{
    uint8_t OEM_name[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t file_allocation_table_count;
    uint16_t root_entry_count;
    uint16_t sector_count;
    uint8_t media_descriptor;
    uint16_t sectors_per_file_allocation_table;
    uint16_t sectors_per_track;
    uint16_t head_count;
    uint32_t hidden_sector_count;
    uint32_t huge_sector_count;
    uint8_t drive_number;
    uint8_t reserved;
    uint8_t boot_signature;
    uint32_t volume_id;
    uint8_t volume_label[11];
    uint8_t file_system_type[8];
};
#pragma pack(pop)

int _tmain(int argc, _In_count_(argc) PTSTR* argv)
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    // ERRORLEVEL zero is the success code.
    int error_level = 0;

    return error_level;
}

