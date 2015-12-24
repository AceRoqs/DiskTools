#pragma once

#include <WindowsCommon/ScopedWindowsTypes.h>

namespace WindowsCommon
{

// Reference get_hyperlink_control_class when using CreateWindow to create
// a hyperlink control.
PCWSTR get_hyperlink_control_class() noexcept;
Scoped_atom register_hyperlink_class(_In_ HINSTANCE instance);

}

