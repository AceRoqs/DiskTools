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

const unsigned int bytes_per_sector = 512;
std::vector<uint8_t> get_default_boot_sector()
{
    std::vector<uint8_t> boot_sector(bytes_per_sector);

    // TODO: write jump, bpb, and boot code.
    boot_sector[bytes_per_sector - 2] = 0x55;
    boot_sector[bytes_per_sector - 1] = 0xaa;

    return std::move(boot_sector);
}

std::vector<uint8_t> get_empty_file_allocation_table(unsigned int sector_count)
{
    std::vector<uint8_t> file_allocation_table(sector_count * bytes_per_sector);

    file_allocation_table[0] = 0xf0;
    file_allocation_table[1] = 0xff;
    file_allocation_table[2] = 0xff;

    return std::move(file_allocation_table);
}

std::vector<uint8_t> get_empty_root_directory(unsigned int sector_count)
{
    std::vector<uint8_t> root_directory(sector_count * bytes_per_sector);

    return std::move(root_directory);
}

int _tmain(int argc, _In_count_(argc) PTSTR* argv)
{
    // ERRORLEVEL zero is the success code.
    int error_level = 0;

    std::wstring boot_sector_file_name;
    for(int ii = 0; ii < argc; ++ii)
    {
        if(_tcsncmp(argv[ii], _T("-b="), 3) == 0)
        {
            boot_sector_file_name = &argv[ii][3];
        }
    }

    auto boot_sector = get_default_boot_sector();
    const auto file_allocation_table = get_empty_file_allocation_table(9);
    const auto root_directory = get_empty_root_directory(14);

    if(!boot_sector_file_name.empty())
    {
        std::basic_ifstream<uint8_t> boot_sector_file(boot_sector_file_name, std::ios::in | std::ios::binary);
        boot_sector_file.read(&boot_sector[0], boot_sector.size());
    }

    const unsigned int sides = 2;
    const unsigned int tracks_per_side = 80;
    const unsigned int sectors_per_track = 18;
    std::vector<uint8_t> disk_image(bytes_per_sector * sides * tracks_per_side * sectors_per_track);

    auto iterator = std::begin(disk_image);
    iterator = std::copy(std::cbegin(boot_sector), std::cend(boot_sector), iterator);
    iterator = std::copy(std::cbegin(file_allocation_table), std::cend(file_allocation_table), iterator);
    iterator = std::copy(std::cbegin(file_allocation_table), std::cend(file_allocation_table), iterator);
    iterator = std::copy(std::cbegin(root_directory), std::cend(root_directory), iterator);
    std::fill(iterator, std::end(disk_image), 0xf6);    // Match behavior of bfi.exe, by Bart Lagerweij.

    // Writing to ofstream is much faster than writing to basic_ofstream<uint8_t>, but for the
    // sizes being written, it's okay in optimized builds.
    std::basic_ofstream<uint8_t> output_file("disk.img", std::ios::out | std::ios::binary);
    output_file.write(&disk_image[0], disk_image.size());
    output_file.close();

    return error_level;
}

