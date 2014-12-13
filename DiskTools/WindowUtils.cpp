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

#include "PreCompile.h"
#include "WindowUtils.h"    // Pick up forward declarations to ensure correctness.
#include "Verify.h"

//---------------------------------------------------------------------------
void set_window_icon(_In_ HWND window, _In_ HINSTANCE instance, _In_ PCTSTR icon_resource)
{
    // 32 pixels is used as this icon is used as both the window icon
    // and the taskbar icon.
    HICON icon_handle = static_cast<HICON>(::LoadImage(instance,
                                                       icon_resource,
                                                       IMAGE_ICON,
                                                       32,
                                                       32,
                                                       LR_DEFAULTCOLOR | LR_SHARED));

    ::SendMessage(window, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(icon_handle));
}

//---------------------------------------------------------------------------
unsigned int get_imagelist_width_by_index(
    _In_opt_ HIMAGELIST image_list,
    unsigned int image_index)
{
    // Return zero if no image_list is passed.
    unsigned int image_width = 0;
    if(nullptr != image_list)
    {
        IMAGEINFO image_info;
        VERIFY((::ImageList_GetImageInfo(image_list, image_index, &image_info)) != 0);
        image_width = image_info.rcImage.right - image_info.rcImage.left;
    }

    return image_width;
}

//---------------------------------------------------------------------------
bool is_listview_in_report_mode(_In_ HWND listview)
{
    LONG style = ::GetWindowLong(listview, GWL_STYLE);
    return (style & LVS_TYPEMASK) == LVS_REPORT;
}

//---------------------------------------------------------------------------
bool adjust_listview_column_widths(
    _In_ HWND listview,
    unsigned int image_index)
{
    assert(INVALID_HANDLE_VALUE != listview);
    assert(is_listview_in_report_mode(listview));

    bool success = true;
    TCHAR text[32];

    LVCOLUMN column_data;
    ZeroMemory(&column_data, sizeof(column_data));
    column_data.mask       = LVCF_TEXT;
    column_data.pszText    = text;
    column_data.cchTextMax = ARRAYSIZE(text);

    // If there is an image list, use the image_index
    // icon size in the width calculations.
    unsigned int image_width = get_imagelist_width_by_index(ListView_GetImageList(listview, LVSIL_SMALL),
                                                            image_index);

    HWND header = ListView_GetHeader(listview);
    unsigned int column_count = Header_GetItemCount(header);
    unsigned int row_count = ListView_GetItemCount(listview);

    for(unsigned int column = 0; column < column_count; ++column)
    {
        // Use the width of the column header as the default column width.
        verify(ListView_GetColumn(listview, column, &column_data) != 0);
        int min_width = ListView_GetStringWidth(listview, column_data.pszText);

        // See if any of the rows require more width than the current minimum.
        for(unsigned int row = 0; row < row_count; ++row)
        {
            ListView_GetItemText(listview, row, column, text, ARRAYSIZE(text));
            int column_width = ListView_GetStringWidth(listview, text);

            // Special case the first column, because it may have an icon.
            if(0 == column)
            {
                column_width += image_width;
            }

            if(column_width > min_width)
            {
                min_width = column_width;
            }
        }

        // The docs state that padding must be added to the return value of
        // ListView_GetStringWidth.  After testing, six pixels on each side
        // appears to be adequate.
        // http://msdn.microsoft.com/en-us/library/bb775074
        min_width += 12;

        if(!ListView_SetColumnWidth(listview, column, min_width))
        {
            success = false;
            break;
        }
    }

    return success;
}

//---------------------------------------------------------------------------
void get_clientspace_grip_rect(_In_ HWND window, _Out_ RECT* grip_rect)
{
    ::GetClientRect(window, grip_rect);

    // Grip is a type of scroll bar control (DFCS_SCROLLSIZEGRIP).
    grip_rect->left = grip_rect->right  - GetSystemMetrics(SM_CXHSCROLL);
    grip_rect->top  = grip_rect->bottom - GetSystemMetrics(SM_CYVSCROLL);
}

//---------------------------------------------------------------------------
void get_clientspace_control_rect(
    _In_ HWND window,
    _In_ int control_id,
    _Out_ RECT* clientspace_rect)
{
    HWND control_window = ::GetDlgItem(window, control_id);

    RECT screenspace_rect;
    ::GetWindowRect(control_window, &screenspace_rect);

    POINT point = { screenspace_rect.left, screenspace_rect.top };
    ::ScreenToClient(window, &point);

    clientspace_rect->left   = point.x;
    clientspace_rect->top    = point.y;
    clientspace_rect->right  = point.x + screenspace_rect.right - screenspace_rect.left;
    clientspace_rect->bottom = point.y + screenspace_rect.bottom - screenspace_rect.top;
}

//---------------------------------------------------------------------------
void get_repositioned_rect_by_offset(
    const RECT& original_control_rect,
    const POINT& position_offset,
    const SIZE& size_offset,
    _Out_ RECT* window_rect)
{
    window_rect->left   = original_control_rect.left + position_offset.x;
    window_rect->top    = original_control_rect.top + position_offset.y;
    window_rect->right  = original_control_rect.right - original_control_rect.left + size_offset.cx;
    window_rect->bottom = original_control_rect.bottom - original_control_rect.top + size_offset.cy;
}

//---------------------------------------------------------------------------
void reposition_control_by_offset(
    _In_ HWND parent_window,
    _In_ int control_id,
    const RECT& original_control_rect,
    const POINT& position_offset,
    const SIZE& size_offset)
{
    HWND control_window = ::GetDlgItem(parent_window, control_id);

    RECT control_rect;
    get_repositioned_rect_by_offset(original_control_rect, position_offset, size_offset, &control_rect);

    ::MoveWindow(control_window,
                 control_rect.left,
                 control_rect.top,
                 control_rect.right,
                 control_rect.bottom,
                 FALSE);
}

//---------------------------------------------------------------------------
void display_localized_error_dialog(
    _In_opt_ HWND parent_window,
    _In_ HINSTANCE instance,
    UINT caption_id,
    UINT message_id)
{
    TCHAR caption_buffer[32];
    ::LoadString(instance, caption_id, caption_buffer, ARRAYSIZE(caption_buffer));
    caption_buffer[ARRAYSIZE(caption_buffer) - 1] = TEXT('\0'); // Suggested by static analysis.

    TCHAR message_buffer[256];
    ::LoadString(instance, message_id, message_buffer, ARRAYSIZE(message_buffer));
    message_buffer[ARRAYSIZE(message_buffer) - 1] = TEXT('\0'); // Suggested by static analysis.

    ::MessageBox(parent_window, message_buffer, caption_buffer, MB_OK | MB_ICONERROR);
}

