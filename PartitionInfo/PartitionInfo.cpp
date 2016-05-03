// This program prints the partition table information for the first.
// fixed disk.  For Windows NT and XP, administrator privileges are
// required, and for Vista and above, the program needs to be elevated.
// Getting a handle to a disk requires Windows NT 4.0 or better.

#include "PreCompile.h"
#include <DiskTools/DirectRead.h>
#include <WindowsCommon/DebuggerTracing.h>
#include <PortableRuntime/CheckException.h>
#include <PortableRuntime/Tracing.h>
#include <PortableRuntime/Unicode.h>

namespace PartitionInfo
{

// Display partition table data to stdout.
// TODO: Make this take an ostream and put it into DiskTools.
static void output_partition_table_info(
    _In_reads_(DiskTools::partition_table_entry_count) const DiskTools::Partition_table_entry* entry,
    unsigned int sector_size)
{
    for(unsigned int index = 0; index < DiskTools::partition_table_entry_count; ++index)
    {
        uint64_t part_size = static_cast<uint64_t>(entry[index].sectors) * sector_size;

        _tprintf(_TEXT("Partition %u:\r\n"), index);
        _tprintf(_TEXT(" Bootable: %s\r\n"), entry[index].bootable ? _TEXT("Yes") : _TEXT("No"));

        PCTSTR file_system_name = DiskTools::get_file_system_name(entry[index].file_system_type);
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

}

// _In_reads_(argc) is the correct SAL annotation, but argc is not defined.
int wmain(int argc, _In_reads_(argc) wchar_t** argv)
{
    (void)argc;     // Unreferenced parameter.
    (void)argv;
    // TODO: 2016: Consider tracing the unreferenced parameters.

    // ERRORLEVEL zero is the success code.
    int error_level = 0;

    try
    {
        PortableRuntime::set_dprintf(WindowsCommon::debugger_dprintf);

        // Set wprintf output to UTF-8 in Windows console.
        // CHECK_EXCEPTION ensures against the case that the CRT invalid parameter handler
        // routine is set by a global constructor.
        CHECK_EXCEPTION(_setmode(_fileno(stdout), _O_U8TEXT) != -1, u8"Failed to set UTF-8 output mode.");
        CHECK_EXCEPTION(_setmode(_fileno(stderr), _O_U8TEXT) != -1, u8"Failed to set UTF-8 output mode.");

        // Fixed disks with partition tables will generally have a sector
        // size of 512 bytes (valid as of 2011).
        const unsigned int sector_size = 512;

        std::array<uint8_t, sector_size> buffer;
        unsigned int bytes_to_read = sector_size;

        HRESULT hr = DiskTools::read_sector_from_disk(buffer.data(), &bytes_to_read, 0, 0);
        if(SUCCEEDED(hr))
        {
            if(sector_size == bytes_to_read)
            {
                // Final two bytes are a boot sector signature, and the partition table immediately preceeds it.
                unsigned int table_start = sector_size - 2 - (sizeof(DiskTools::Partition_table_entry) * DiskTools::partition_table_entry_count);
                auto entries = reinterpret_cast<DiskTools::Partition_table_entry*>(buffer.data() + table_start);
                PartitionInfo::output_partition_table_info(entries, sector_size);

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
    }
    catch(const std::exception& ex)
    {
        std::fwprintf(stderr, L"\n%s\n", PortableRuntime::utf16_from_utf8(ex.what()).c_str());
        error_level = 1;
    }

    return error_level;
}

