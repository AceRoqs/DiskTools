#pragma once

namespace WindowsCommon
{

void debug_validate_message_map() noexcept;
PCSTR string_from_window_message(UINT message) noexcept;
bool dispatch_all_windows_messages(_Out_ MSG* message) noexcept;

}

