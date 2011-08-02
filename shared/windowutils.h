/*
Copyright (C) 2000-2011 by Toby Jones.

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

#ifndef WINDOWUTILS_H
#define WINDOWUTILS_H

void set_window_icon(_In_ HWND window, _In_ HINSTANCE instance, _In_ PCTSTR icon_resource);
bool is_listview_in_report_mode(_In_ HWND listview);
bool adjust_listview_column_widths(_In_ HWND listview, unsigned int image_index);
void get_clientspace_grip_rect(_In_ HWND window, _Out_ RECT* grip_rect);
void get_clientspace_control_rect(_In_ HWND window, _In_ int control_id, _Out_ RECT* clientspace_rect);
void get_repositioned_rect_by_offset(const RECT& original_control_rect, const POINT& position_offset, const SIZE& size_offset, _Out_ RECT* window_rect);
void reposition_control_by_offset(_In_ HWND parent_window, _In_ int control_id, const RECT& original_control_rect, const POINT& position_offset, const SIZE& size_offset);
void display_localized_error_dialog(_In_opt_ HWND parent_window, _In_ HINSTANCE instance, UINT caption_id, UINT message_id);

#endif

