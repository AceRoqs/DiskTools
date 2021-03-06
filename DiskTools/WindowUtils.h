#pragma once

namespace DiskTools
{

// TODO: Once the WindowsCommon library is exposed, these functions might move to that namespace.
void set_window_icon(_In_ HWND window, _In_ HINSTANCE instance, _In_ PCWSTR icon_resource);
bool is_listview_in_report_mode(_In_ HWND listview);
bool adjust_listview_column_widths(_In_ HWND listview, unsigned int image_index);
RECT get_clientspace_grip_rect(_In_ HWND window);
RECT get_clientspace_control_rect(_In_ HWND window, _In_ int control_id);
RECT get_repositioned_rect_by_offset(const RECT& original_control_rect, const POINT& position_offset, const SIZE& size_offset);
void reposition_control_by_offset(_In_ HWND parent_window, _In_ int control_id, const RECT& original_control_rect, const POINT& position_offset, const SIZE& size_offset);
void display_localized_error_dialog(_In_opt_ HWND parent_window, _In_ HINSTANCE instance, UINT caption_id, UINT message_id);

}

