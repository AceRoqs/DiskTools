#include <Windows.h>
#include <tchar.h>
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

#define FAT_MAX_FILENAME_LENGTH 8
#define FAT_MAX_EXTENSION_LENGTH 3

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
    uint8_t volume_label[FAT_MAX_FILENAME_LENGTH + FAT_MAX_EXTENSION_LENGTH];
    uint8_t file_system_type[8];
};

struct Root_directory_entry
{
    uint8_t filename[FAT_MAX_FILENAME_LENGTH];
    uint8_t extension[FAT_MAX_EXTENSION_LENGTH];
    uint8_t attributes;
    uint16_t reserved;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access_date;
    uint16_t ignored;
    uint16_t last_write_time;
    uint16_t last_write_date;
    uint16_t first_logical_cluster;
    uint32_t file_size;
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

void usage()
{
    std::cerr << "writeimage [-b=file] [-l=label] -f=file.img\n";
    std::cerr << "    -f=file   Output filename\n";
    std::cerr << "    -b=file   Install bootsector from \"file\"\n";
    std::cerr << "    -l=label  Set volume label to \"label\"\n";
    std::cerr << std::endl;

#if 0
Usage: bfi [-v] [-t=type] [-o=file] [-o=file] [-l=mylabel] [-b=file]
           -f=file.img path [path ...]

   -v         Verbose mode (talk more)
   -t=type    Disktype use string "144", "120" or "288" or number:
              4=720K,6=1440K,7=2880K,8=DMF2048,9=DMF1024,10=1680K
              0=160K,1=180K,2=320K,3=360K,5=1200K
              Default is 1.44MB
   -f=file    Image filename
   -o=file    Order file, put these file on the image first
   -l=mylabel Set volume label to "mylabel"
   -b=file    Install bootsector from "file"
   path       Input folder(s) to inject files from
#endif
}

bool is_legal_fat_character(wchar_t ch)
{
    if((ch >= L'A') && (ch <= L'Z'))
    {
        return true;
    }
    if((ch >= L'0') && (ch <= L'9'))
    {
        return true;
    }
    if((ch >= 128) && (ch <= 255))
    {
        return true;
    }

    const char legal_chars[] = ".!#$%&'()-@^_`{}~";
    for(auto iter = std::cbegin(legal_chars); iter != std::cend(legal_chars); ++iter)
    {
        if(ch == *iter)
        {
            return true;
        }
    }

    return false;
}

// TODO: Consider outputting a std::string instead of std::wstring.
std::wstring sanitize_label(const std::wstring& input_label)
{
    static_assert(sizeof(Bios_parameter_block().volume_label) == (sizeof(Root_directory_entry().filename) + sizeof(Root_directory_entry().extension)),
                  "Directory entry and BPB must match size for volume label.");

    std::wstring output_label(input_label);

    output_label.erase(sizeof(Bios_parameter_block().volume_label), std::wstring::npos);
    std::transform(std::begin(output_label), std::end(output_label), std::begin(output_label), ::towupper);
    std::for_each(std::cbegin(output_label), std::cend(output_label), [](wchar_t ch)
    {
        if(!is_legal_fat_character(ch))
        {
            throw std::exception();
        }
    });

    return std::move(output_label);
}

void output_boot_sector(int argc, _In_count_(argc) PTSTR* argv)
{
    std::wstring boot_sector_file_name;
    std::wstring image_file_name;
    std::wstring label;
    for(int ii = 0; ii < argc; ++ii)
    {
        if(_tcsncmp(argv[ii], _T("-b="), 3) == 0)
        {
            boot_sector_file_name = &argv[ii][3];
        }
        else if(_tcsncmp(argv[ii], _T("-f="), 3) == 0)
        {
            image_file_name = &argv[ii][3];
        }
        else if(_tcsncmp(argv[ii], _T("-l="), 3) == 0)
        {
            label = &argv[ii][3];
        }
    }

    label = sanitize_label(label);
    auto boot_sector = get_default_boot_sector();
    const auto file_allocation_table = get_empty_file_allocation_table(9);
    const auto root_directory = get_empty_root_directory(14);

    if(image_file_name.empty())
    {
        image_file_name = L"file.img";
    }
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
    std::basic_ofstream<uint8_t> output_file(image_file_name, std::ios::out | std::ios::binary);
    output_file.write(&disk_image[0], disk_image.size());
    output_file.close();
}

int _tmain(int argc, _In_count_(argc) PTSTR* argv)
{
    // ERRORLEVEL zero is the success code.
    int error_level = 0;

    if(argc < 2)
    {
        usage();
    }
    else
    {
        try
        {
            output_boot_sector(argc, argv);
        }
        catch(...)
        {
            error_level = 1;
        }
    }

    return error_level;
}

