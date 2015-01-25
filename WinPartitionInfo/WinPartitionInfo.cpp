#include "PreCompile.h"
#include "Resource.h"
#include <DiskTools/DirectRead.h>
#include <DiskTools/Verify.h>
#include <DiskTools/StringUtils.h>
#include <DiskTools/WindowUtils.h>

namespace WinPartitionInfo
{

const unsigned int sector_size = 512;
const unsigned int max_partitions = 32;

static const struct Listview_columns
{
    DWORD name;
    int format;
} listview_columns[] =
{
    IDS_BOOTABLE,       LVCFMT_CENTER,
    IDS_FILESYSTEMTYPE, LVCFMT_LEFT,
    IDS_SIZEINBYTES,    LVCFMT_RIGHT,
    IDS_DRIVENUMBER,    LVCFMT_RIGHT,
    IDS_BEGINHEAD,      LVCFMT_RIGHT,
    IDS_BEGINCYLINDER,  LVCFMT_RIGHT,
    IDS_BEGINSECTOR,    LVCFMT_RIGHT,
    IDS_ENDHEAD,        LVCFMT_RIGHT,
    IDS_ENDCYLINDER,    LVCFMT_RIGHT,
    IDS_ENDSECTOR,      LVCFMT_RIGHT,
    IDS_STARTSECTOR,    LVCFMT_RIGHT,
};

// This enumeration must match the order of listview_columns.
enum listview_column_ids
{
    bootable_entry = 0,
    file_system_entry,
    size_in_bytes,
    drive_number,
    begin_head,
    begin_cylinder,
    begin_sector,
    end_head,
    end_cylinder,
    end_sector,
    start_sector,
};

void get_yesno_string(
    bool is_yes,
    HINSTANCE instance,
    _Out_writes_z_(yesno_size) PTSTR yesno,
    _In_ unsigned int yesno_size)
{
    if(is_yes)
    {
        PortableRuntime::verify(LoadString(instance, IDS_YES, yesno, yesno_size) > 0);
    }
    else
    {
        PortableRuntime::verify(LoadString(instance, IDS_NO, yesno, yesno_size) > 0);
    }
    yesno[yesno_size - 1] = TEXT('\0');   // Suggested by static analysis.
}

void get_file_system_name_from_type(
    _Out_writes_z_(file_system_name_size) PTSTR file_system_name,
    _In_ unsigned int file_system_name_size,
    uint8_t file_system_type)
{
    PCTSTR name = DiskTools::get_file_system_name(file_system_type);

    if(nullptr != name)
    {
        WindowsCommon::verify_hr(StringCchPrintf(file_system_name,
                                                 file_system_name_size,
                                                 TEXT("(%02X) %s"),
                                                 file_system_type,
                                                 name));
    }
    else
    {
        WindowsCommon::verify_hr(StringCchPrintf(file_system_name,
                                                 file_system_name_size,
                                                 TEXT("(%02X)"),
                                                 file_system_type));
    }
}

// This function may be moved to a shared library at some point if
// the partitions vector is capped to a max per disk instead of a
// max total.
HRESULT read_disk_partitions_from_handle(
    _In_ std::vector<std::pair<uint8_t, DiskTools::Partition_table_entry>>* partitions,
    HANDLE disk_handle,
    uint8_t disk_number,
    uint32_t logical_partition_start_sector)
{
    std::array<uint8_t, sector_size> buffer;
    unsigned int bytes_to_read = sizeof(buffer);

    HRESULT hr = DiskTools::read_sector_from_handle(buffer.data(), &bytes_to_read, disk_handle, logical_partition_start_sector);
    if(SUCCEEDED(hr))
    {
        // In this implementation, bytes_to_read can never be above sector_size, but if
        // the buffer is defined as being larger than a default minimum sector size, then this
        // check is still correct.
        if(sector_size <= bytes_to_read)
        {
            // Final two bytes are a boot sector signature, and the partition table immediately precedes it.
            static_assert(sector_size >= 2 + (sizeof(DiskTools::Partition_table_entry) * partition_table_entry_count),
                          "sector_size must be large enough to contain a partition table.");
            unsigned int table_start = bytes_to_read - 2 - (sizeof(DiskTools::Partition_table_entry) * partition_table_entry_count);
            auto entries = reinterpret_cast<DiskTools::Partition_table_entry*>(buffer.data() + table_start);

            for(unsigned int entry_index = 0; entry_index < partition_table_entry_count; ++entry_index)
            {
                // If partition entry is blank, don't include it.
                if(0x00 == entries[entry_index].file_system_type)
                {
                    continue;
                }

                if(DiskTools::is_extended_partition(entries[entry_index].file_system_type))
                {
                    continue;
                }

                partitions->push_back(std::make_pair(disk_number, entries[entry_index]));

                if(max_partitions == partitions->size())
                {
                    break;
                }
            }

            // Cap the size of the partitions vector.
            if(max_partitions > partitions->size())
            {
                // Handle extended partitions.
                // http://en.wikipedia.org/wiki/Extended_Boot_Record
                for(unsigned int entry_index = 0; entry_index < partition_table_entry_count; ++entry_index)
                {
                    if(DiskTools::is_extended_partition(entries[entry_index].file_system_type))
                    {
                        hr = read_disk_partitions_from_handle(partitions,
                                                              disk_handle,
                                                              disk_number,
                                                              logical_partition_start_sector + entries[entry_index].start_sector);
                        break;
                    }
                }
            }
        }
        else
        {
            // Sectors smaller than sector_size are not supported, as sector_size is assumed to
            // be at least the size of a partition table (plus the two signature bytes at the end of the sector).
            // Caution should be used when printing out this HRESULT, as Windows will display this
            // as a "catastrophic error."
            hr = E_UNEXPECTED;
        }
    }

    return hr;
}

// This function may be moved to a shared library at some point if
// read_disk_partitions_from_handle is also moved.
HRESULT read_disk_partitions(
    _In_ std::vector<std::pair<uint8_t, DiskTools::Partition_table_entry>>* partitions,
    uint8_t disk_number,
    uint32_t logical_partition_start)
{
    HRESULT hr = S_OK;

    // Open a read handle to the physical disk.  Many sectors may be read
    // on a single disk (in the case of extended partitions), so save the
    // handle for reuse instead of reopening on every read.
    // This call requires elevation to administrator.
    std::unique_ptr<void, std::function<void (HANDLE handle)>> disk_handle(
        DiskTools::get_disk_handle(disk_number),
        [](HANDLE handle)
        {
            if(INVALID_HANDLE_VALUE != handle)
            {
                CloseHandle(handle);
            }
        });

    if(INVALID_HANDLE_VALUE == disk_handle.get())
    {
        hr = HRESULT_FROM_WIN32(ERROR_OPEN_FAILED);
    }

    if(SUCCEEDED(hr))
    {
        hr = read_disk_partitions_from_handle(partitions, disk_handle.get(), disk_number, logical_partition_start);
    }

    return hr;
}

void output_partition_table_info(
    _In_ const std::vector<std::pair<uint8_t, DiskTools::Partition_table_entry>>* partitions,
    _In_ HWND listview,
    _In_ HINSTANCE instance)
{
    assert(max_partitions >= partitions->size());

    unsigned int row = ListView_GetItemCount(listview);
    LVITEM item = {};
    for(auto partition = std::cbegin(*partitions); partition != std::cend(*partitions); ++partition)
    {
        // Insert a new row into the listview.
        item.iItem = row;
        if(ListView_InsertItem(listview, &item) != static_cast<int>(row))
        {
            throw std::bad_alloc();
        }

        std::array<TCHAR[32], ARRAYSIZE(listview_columns)> labels = {};

        // Built the set of strings for the row.
        // Since label strings are all the same length, all array sizes
        // are referenced as labels[0] to make the code more clear.
        get_yesno_string(partition->second.bootable == 0x80,
                         instance,
                         labels[bootable_entry],
                         ARRAYSIZE(labels[0]));
        get_file_system_name_from_type(labels[file_system_entry],
                                       ARRAYSIZE(labels[0]),
                                       partition->second.file_system_type);
        DiskTools::pretty_print32(partition->second.begin_head,     labels[begin_head],     ARRAYSIZE(labels[0]));
        DiskTools::pretty_print32(partition->second.begin_head,     labels[begin_head],     ARRAYSIZE(labels[0]));
        DiskTools::pretty_print32(partition->second.begin_cylinder, labels[begin_cylinder], ARRAYSIZE(labels[0]));
        DiskTools::pretty_print32(partition->second.begin_sector,   labels[begin_sector],   ARRAYSIZE(labels[0]));
        DiskTools::pretty_print32(partition->second.end_head,       labels[end_head],       ARRAYSIZE(labels[0]));
        DiskTools::pretty_print32(partition->second.end_cylinder,   labels[end_cylinder],   ARRAYSIZE(labels[0]));
        DiskTools::pretty_print32(partition->second.end_sector,     labels[end_sector],     ARRAYSIZE(labels[0]));
        DiskTools::pretty_print32(partition->second.start_sector,   labels[start_sector],   ARRAYSIZE(labels[0]));
        DiskTools::pretty_print64(static_cast<uint64_t>(partition->second.sectors) * sector_size,
                                  labels[size_in_bytes],
                                  ARRAYSIZE(labels[0]));
        DiskTools::pretty_print32(partition->first, labels[drive_number], ARRAYSIZE(labels[0]));

        // Add labels to all columns of this row.
        unsigned int column = 0;
        std::for_each(std::begin(labels), std::end(labels), [listview, row, &column](PTSTR label)
        {
            LVITEM row_item;
            row_item.iSubItem = column;
            row_item.pszText  = label;
            if(!SendMessage(listview, LVM_SETITEMTEXT, row, reinterpret_cast<LPARAM>(&row_item)))
            {
                throw std::bad_alloc();
            }

            ++column;
        });

        ++row;
    }
}

static void populate_listview(
    _In_ HWND listview,
    _In_ HINSTANCE instance)
{
    // Vector of all (drive, partition) pairs.
    std::vector<std::pair<uint8_t, DiskTools::Partition_table_entry>> partitions;

    // Read partition tables of first two disks.
    // Ignore read errors - any entry to the partitions list is
    // complete, and any missing entries should be obvious to
    // the advanced user (the target of this application).  The
    // normal errors might be a missing or ejected disk, or a sector
    // size that isn't 512 bytes, such as very recent USB disks.
    read_disk_partitions(&partitions, 0, 0);
    read_disk_partitions(&partitions, 1, 0);
    output_partition_table_info(&partitions, listview, instance);
}

// This function may be moved to a shared library at some point if
// Listview_columns becomes a useful structure to share.
void add_listview_headers(
    _In_ HWND listview,
    _In_ HINSTANCE instance,
    _In_reads_(column_count) const Listview_columns* columns,
    _In_ unsigned int column_count)
{
    assert(INVALID_HANDLE_VALUE != listview);
    assert(DiskTools::is_listview_in_report_mode(listview));

    LVCOLUMN new_column = {};
    new_column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_ORDER | LVCF_FMT;

    for(unsigned int column_index = 0; column_index < column_count; ++column_index)
    {
        TCHAR label[32];
        PortableRuntime::verify(LoadString(instance, columns[column_index].name, label, ARRAYSIZE(label)) > 0);
        label[ARRAYSIZE(label) - 1] = TEXT('\0');    // Suggested by static analysis.

        new_column.fmt = columns[column_index].format;
        new_column.iOrder = column_index;
        new_column.cx = 1;  // Column size should be adjusted later.
        new_column.pszText = label;
        // Cast is safe as size is guaranteed to be less than ARRAYSIZE(label).
        new_column.cchTextMax = static_cast<int>(_tcslen(new_column.pszText));

        if(ListView_InsertColumn(listview, column_index, &new_column) == -1)
        {
            // The only likely failure case for insertion of valid data
            // is memory exhaustion.
            throw std::bad_alloc();
        }
    }
}

static BOOL on_command(_In_ HWND window, WORD id)
{
    BOOL message_processed = FALSE;

    if(IDCANCEL == id)
    {
        // Return 1 on success, as it makes the DialogBox* return code unambiguous.
        EndDialog(window, 1);
        message_processed = TRUE;
    }

    return message_processed;
}

static void on_paint(_In_ HWND window)
{
    RECT grip_rect = DiskTools::get_clientspace_grip_rect(window);

    PAINTSTRUCT paint_struct;

    // Ensure EndPaint is called at the end of the method.
    std::unique_ptr<HDC__, std::function<void (HDC)>> context(
        BeginPaint(window, &paint_struct),
        [window, &paint_struct](HDC)
        {
            EndPaint(window, &paint_struct);
        });

    // Paint the grip.
    DrawFrameControl(context.get(), &grip_rect, DFC_SCROLL, DFCS_SCROLLSIZEGRIP);
}

static std::tuple<BOOL, LONG> on_nc_hit_test(_In_ HWND window, int x_coord, int y_coord)
{
    BOOL message_processed = FALSE;
    LONG hit_location = HTERROR;

    RECT grip_rect = DiskTools::get_clientspace_grip_rect(window);

    POINT mouse_location;
    mouse_location.x = x_coord;
    mouse_location.y = y_coord;
    ScreenToClient(window, &mouse_location);

    // Hit test the mouse, and allow drag resizing if the mouse
    // is over any part of the grip.  (Resizing is typically only
    // allowed if the mouse is over the corner of the grip).
    if(PtInRect(&grip_rect, mouse_location))
    {
        hit_location = HTBOTTOMRIGHT;
        message_processed = TRUE;
    }

    return std::make_tuple(message_processed, hit_location);
}

class Partition_table_dialog
{
public:
    Partition_table_dialog();
    void show(_In_ HINSTANCE instance, _In_ PCTSTR dialog_id) const;

protected:
    static INT_PTR CALLBACK dialog_proc(_In_ HWND window, UINT message, WPARAM w_param, LPARAM l_param);
    void on_init_dialog(_In_ HWND window, _In_ HINSTANCE instance);
    void on_get_minmax_info(_In_ MINMAXINFO* minmax_info) const;
    void on_size(_In_ HWND window, int new_client_width, int new_client_height) const;

private:
    RECT m_original_client_rect;
    RECT m_original_clientspace_listview_rect;
    RECT m_original_clientspace_label_rect;
    SIZE m_minimum_dialog_size;

    // Not implemented to prevent accidental copying.
    Partition_table_dialog(const Partition_table_dialog&);
    Partition_table_dialog& operator=(const Partition_table_dialog&);
};

Partition_table_dialog::Partition_table_dialog()
{
    ZeroMemory(&m_original_client_rect, sizeof(m_original_client_rect));
    ZeroMemory(&m_original_clientspace_listview_rect, sizeof(m_original_clientspace_listview_rect));
    ZeroMemory(&m_original_clientspace_label_rect, sizeof(m_original_clientspace_label_rect));
    ZeroMemory(&m_minimum_dialog_size, sizeof(m_minimum_dialog_size));
}

void Partition_table_dialog::show(_In_ HINSTANCE instance, _In_ PCTSTR dialog_id) const
{
    DialogBoxParam(instance,
                   dialog_id,
                   nullptr,
                   dialog_proc,
                   reinterpret_cast<LPARAM>(this));
}

INT_PTR CALLBACK Partition_table_dialog::dialog_proc(
    _In_ HWND window,
    UINT message,
    WPARAM w_param,
    LPARAM l_param)
{
    BOOL message_processed = FALSE;

    try
    {
        switch(message)
        {
            case WM_INITDIALOG:
            {
                // TRUE means to set the keyboard focus to the control in w_param,
                // which is the first control in the dialog.
                // Set message_processed first before any exception is thrown.
                message_processed = TRUE;

                SetWindowLongPtr(window, DWLP_USER, l_param);

                auto dialog = reinterpret_cast<Partition_table_dialog*>(l_param);
                HINSTANCE instance = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(window, GWLP_HINSTANCE));
                dialog->on_init_dialog(window, instance);
                break;
            }

            case WM_COMMAND:
            {
                message_processed = on_command(window, GET_WM_COMMAND_ID(w_param, l_param));
                break;
            }

            case WM_PAINT:
            {
                message_processed = TRUE;
                on_paint(window);
                break;
            }

            case WM_NCHITTEST:
            {
                LONG hit_location;
                std::tie<BOOL, LONG>(message_processed, hit_location) = on_nc_hit_test(window, GET_X_LPARAM(l_param), GET_Y_LPARAM(l_param));

                if(message_processed)
                {
                    SetWindowLong(window, DWLP_MSGRESULT, hit_location);
                }
                break;
            }

            case WM_GETMINMAXINFO:
            {
                message_processed = TRUE;

                // GetWindowLongPtr should never fail.
                // dialog is not valid until WM_INITDIALOG has been sent.
                auto dialog = reinterpret_cast<Partition_table_dialog*>(GetWindowLongPtr(window, DWLP_USER));
                dialog->on_get_minmax_info(reinterpret_cast<MINMAXINFO*>(l_param));
                break;
            }

            case WM_SIZE:
            {
                message_processed = TRUE;

                // GetWindowLongPtr should never fail.
                // dialog is not valid until WM_INITDIALOG has been sent.
                auto dialog = reinterpret_cast<Partition_table_dialog*>(GetWindowLongPtr(window, DWLP_USER));
                dialog->on_size(window, GET_X_LPARAM(l_param), GET_Y_LPARAM(l_param));
                break;
            }
        }
    }
    // Don't allow std::bad_alloc to propagate across ABI boundaries.
    catch(const std::bad_alloc& ex)
    {
        UNREFERENCED_PARAMETER(ex);
        HINSTANCE instance = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(window, GWLP_HINSTANCE));
        DiskTools::display_localized_error_dialog(window, instance, IDS_OOMCAPTION, IDS_OOMMESSAGE);
    }

    // If message_processed is TRUE, then DWLP_MSGRESULT is either set or is implicitly zero.
    return message_processed;
}

void Partition_table_dialog::on_init_dialog(_In_ HWND window, _In_ HINSTANCE instance)
{
    DiskTools::set_window_icon(window, instance, MAKEINTRESOURCE(IDI_HARDDISK));

    // Optional:
    // center_window_on_parent(window);

    // Use the original dialog size as the minimum window size.
    RECT window_rect;
    GetWindowRect(window, &window_rect);
    m_minimum_dialog_size.cx = window_rect.right - window_rect.left;
    m_minimum_dialog_size.cy = window_rect.bottom - window_rect.top;

    // Save window rectangles for use during resize.
    GetClientRect(window, &m_original_client_rect);
    m_original_clientspace_listview_rect = DiskTools::get_clientspace_control_rect(window, IDC_PARTITIONS);

    HWND listview = GetDlgItem(window, IDC_PARTITIONS);
    add_listview_headers(listview, instance, listview_columns, ARRAYSIZE(listview_columns));
    populate_listview(listview, instance);
    DiskTools::adjust_listview_column_widths(listview, 0);
}

void Partition_table_dialog::on_get_minmax_info(_In_ MINMAXINFO* minmax_info) const
{
    minmax_info->ptMinTrackSize.x = m_minimum_dialog_size.cx;
    minmax_info->ptMinTrackSize.y = m_minimum_dialog_size.cy;
}

void Partition_table_dialog::on_size(_In_ HWND window, int new_client_width, int new_client_height) const
{
    int offset_width = new_client_width - m_original_client_rect.right;
    int offset_height = new_client_height - m_original_client_rect.bottom;

    POINT position_offset = {};
    SIZE size_offset = { offset_width, offset_height };

    DiskTools::reposition_control_by_offset(window, IDC_PARTITIONS, m_original_clientspace_listview_rect, position_offset, size_offset);

    // Because a resize grip is displayed, invalidate the client rect.  Since
    // the whole client area is redrawn, there is no need for DeferWindowPos,
    // since the window will flicker anyway.
    InvalidateRect(window, nullptr, TRUE);
}

}

int WINAPI _tWinMain(_In_ HINSTANCE instance,   // Handle to the program instance.
                     _In_opt_ HINSTANCE,        // hInstPrev - Unused in Win32.
                     _In_ PTSTR command_line,   // Command line.
                     _In_ int show_command)     // How the window is to be displayed.
{
    // Prevent unreferenced parameter.
    (command_line);
    (show_command);

    INITCOMMONCONTROLSEX init_controls;
    init_controls.dwSize = sizeof(init_controls);
    init_controls.dwICC  = ICC_LISTVIEW_CLASSES;

    // Load the ListView control from the common controls.
    if(InitCommonControlsEx(&init_controls))
    {
        WinPartitionInfo::Partition_table_dialog dialog;
        dialog.show(instance, MAKEINTRESOURCE(IDD_PARTITION_TABLE_DIALOG));
    }
    else
    {
        DiskTools::display_localized_error_dialog(nullptr, instance, IDS_ERRORLISTVIEWCAPTION, IDS_ERRORLISTVIEWMESSAGE);
    }

    return 0;
}

