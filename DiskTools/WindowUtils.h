#pragma once

void set_window_icon(_In_ HWND window, _In_ HINSTANCE instance, _In_ PCTSTR icon_resource);
bool is_listview_in_report_mode(_In_ HWND listview);
bool adjust_listview_column_widths(_In_ HWND listview, unsigned int image_index);
void get_clientspace_grip_rect(_In_ HWND window, _Out_ RECT* grip_rect);
void get_clientspace_control_rect(_In_ HWND window, _In_ int control_id, _Out_ RECT* clientspace_rect);
void get_repositioned_rect_by_offset(const RECT& original_control_rect, const POINT& position_offset, const SIZE& size_offset, _Out_ RECT* window_rect);
void reposition_control_by_offset(_In_ HWND parent_window, _In_ int control_id, const RECT& original_control_rect, const POINT& position_offset, const SIZE& size_offset);
void display_localized_error_dialog(_In_opt_ HWND parent_window, _In_ HINSTANCE instance, UINT caption_id, UINT message_id);

