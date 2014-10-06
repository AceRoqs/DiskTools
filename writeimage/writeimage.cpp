#include <Windows.h>
#include <tchar.h>
#include <cstdint>
#include <fstream>
#include <vector>

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

    const unsigned int bytes_per_sector = 512;
    const unsigned int sides = 2;
    const unsigned int tracks_per_side = 80;
    const unsigned int sectors_per_track = 18;
    std::vector<uint8_t> disk_image(bytes_per_sector * sides * tracks_per_side * sectors_per_track);

    // Writing to ofstream is much faster than writing to basic_ofstream<uint8_t>, but for the
    // sizes being written, it's okay in optimized builds.
    std::basic_ofstream<uint8_t> output_file("disk.img");
    output_file.write(&disk_image[0], disk_image.size());
    output_file.close();

    return error_level;
}

