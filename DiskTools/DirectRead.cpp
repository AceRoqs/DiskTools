// Direct disk access functions require administrator privileges and/or
// process elevation.

#include "PreCompile.h"
#include "DirectRead.h" // Pick up forward declarations to ensure correctness.

//---------------------------------------------------------------------------
PCTSTR cdrom_0         = TEXT("\\\\.\\CDROM0");
PCTSTR physical_disk_0 = TEXT("\\\\.\\PHYSICALDRIVE0");
PCTSTR physical_disk_1 = TEXT("\\\\.\\PHYSICALDRIVE1");

//---------------------------------------------------------------------------
// Mappings of file system types to string names.
// This table is not intended to be localized.
static const struct File_system_type_map
{
    PCTSTR name;
    unsigned char type;
} file_system_types[] =
{
    TEXT("None/Raw"),             0x00,
    TEXT("DOS FAT12"),            0x01,
    TEXT("DOS FAT16"),            0x04,
    TEXT("Extended"),             0x05,
    TEXT("DOS FAT16 (big)"),      0x06,
    TEXT("NTFS/HPFS"),            0x07,
    TEXT("Windows FAT32"),        0x0B,
    TEXT("Windows FAT32 (LBA)"),  0x0C,
    TEXT("Windows FAT16 (LBA)"),  0x0E,
    TEXT("Windows Extended"),     0x0F,
    TEXT("Hidden DOS FAT12"),     0x11,
    TEXT("Hidden DOS FAT16"),     0x14,
    TEXT("Hidden DOS FAT16"),     0x16,
    TEXT("Hidden OS/2 HPFS"),     0x17,
    TEXT("Linux"),                0x81,
    TEXT("Linux Swap"),           0x82,
    TEXT("Linux"),                0x83,
    TEXT("Linux Extended"),       0x85,
    TEXT("GUID Partition Table"), 0xEE,
};
const unsigned int file_system_type_extended1 = 0x05;
const unsigned int file_system_type_extended2 = 0x0F;

//---------------------------------------------------------------------------
static unsigned int file_system_type_count()
{
    return ARRAYSIZE(file_system_types);
}

//---------------------------------------------------------------------------
PCTSTR get_file_system_name(uint8_t file_system_type)
{
    PCTSTR file_system_name = nullptr;

    unsigned int count = file_system_type_count();

    // Find the File_system_type_map that matches the file_system_type.
    auto file_system_type_map = std::lower_bound(file_system_types,
                                                 file_system_types + count,
                                                 file_system_type,
                                                 [](const File_system_type_map& map, const uint8_t file_system_type)
    {
        return map.type < file_system_type;
    });

    if(file_system_type_map < file_system_types + count)
    {
        if(file_system_type_map->type == file_system_type)
        {
            file_system_name = file_system_type_map->name;
        }
    }

    return file_system_name;
}

//---------------------------------------------------------------------------
bool is_extended_partition(uint8_t file_system_type)
{
    return (file_system_type_extended1 == file_system_type) ||
           (file_system_type_extended2 == file_system_type);
}

//---------------------------------------------------------------------------
static HRESULT seek_to_offset(HANDLE handle, uint64_t byte_offset)
{
    HRESULT hr = S_OK;

    uint32_t low_dword, high_dword;
    low_dword  = static_cast<uint32_t>(byte_offset & 0xffffffff);
    high_dword = static_cast<uint32_t>(byte_offset >> 32);
    DWORD pointer = ::SetFilePointer(handle,
                                     static_cast<LONG>(low_dword),
                                     reinterpret_cast<PLONG>(&high_dword),
                                     FILE_BEGIN);
    DWORD last_error = ::GetLastError();

    if((INVALID_SET_FILE_POINTER == pointer) && (0 != last_error))
    {
        hr = HRESULT_FROM_WIN32(last_error);
    }

    return hr;
}

//---------------------------------------------------------------------------
static HRESULT can_buffer_hold_sector(
    HANDLE disk_handle,
    unsigned int buffer_size,
    unsigned int* minimum_buffer_size)
{
    HRESULT hr = S_OK;

    // Get the size of a sector.  Often this is 512 bytes for
    // non-shiny disk media.
    DWORD bytes_read;
    DISK_GEOMETRY disk_geometry;
    if(::DeviceIoControl(disk_handle,
                         IOCTL_DISK_GET_DRIVE_GEOMETRY,
                         nullptr,
                         0,
                         &disk_geometry,
                         sizeof(disk_geometry),
                         &bytes_read,
                         nullptr) != 0)
    {
        if(disk_geometry.BytesPerSector > buffer_size)
        {
            hr = TYPE_E_BUFFERTOOSMALL;
        }

        *minimum_buffer_size = disk_geometry.BytesPerSector;
    }
    else
    {
        DWORD last_error = ::GetLastError();
        hr = HRESULT_FROM_WIN32(last_error);
    }

    return S_OK;
}

//---------------------------------------------------------------------------
HANDLE get_disk_handle(uint8_t disk_number)
{
    static_assert(sizeof(disk_number) == 1, "disk_name array is too short to hold the disk_number.");
    TCHAR disk_name[ARRAYSIZE(TEXT("\\\\.\\PHYSICALDRIVE000"))];
    ::StringCchPrintf(disk_name, ARRAYSIZE(disk_name), TEXT("\\\\.\\PHYSICALDRIVE%u"), disk_number);

    // Open a read handle to the physical disk.
    // This call requires elevation to administrator.
    return ::CreateFile(disk_name,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        nullptr,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        nullptr);
}

//---------------------------------------------------------------------------
HRESULT read_sector_from_handle(
    _Out_cap_post_count_(*buffer_size, *buffer_size) uint8_t* buffer,
    _Inout_ unsigned int* buffer_size,
    _In_ HANDLE handle,
    uint64_t sector_number)
{
    HRESULT hr = can_buffer_hold_sector(handle, *buffer_size, buffer_size);

    if(SUCCEEDED(hr))
    {
        // Seek to the byte offset of the requested sector.
        uint64_t byte_offset = sector_number * (*buffer_size);
        hr = seek_to_offset(handle, byte_offset);
    }

    if(SUCCEEDED(hr))
    {
        // Read in the sector.
        DWORD bytes_read;
        if(::ReadFile(handle, buffer, *buffer_size, &bytes_read, nullptr) == 0)
        {
            hr = HRESULT_FROM_WIN32(ERROR_READ_FAULT);
        }
        *buffer_size = bytes_read;
    }

    return hr;
}

//---------------------------------------------------------------------------
HRESULT read_sector_from_disk(
    _Out_cap_post_count_(*buffer_size, *buffer_size) uint8_t* buffer,
    _Inout_ unsigned int* buffer_size,
    uint8_t disk_number,
    uint64_t sector_number)
{
    HRESULT hr = S_OK;

    // Open a read handle to the physical disk.
    // This call requires elevation to administrator.
    std::unique_ptr<void, std::function<void (HANDLE handle)>> disk_handle(
        get_disk_handle(disk_number),
        [](HANDLE handle)
        {
            if(INVALID_HANDLE_VALUE != handle)
            {
                ::CloseHandle(handle);
            }
        });

    if(INVALID_HANDLE_VALUE == disk_handle.get())
    {
        hr = HRESULT_FROM_WIN32(ERROR_OPEN_FAILED);
    }

    if(SUCCEEDED(hr))
    {
        hr = read_sector_from_handle(buffer, buffer_size, disk_handle.get(), sector_number);
    }

    return hr;
}

