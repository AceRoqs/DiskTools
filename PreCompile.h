#pragma once

// Prevent <Windows.h> from defining min/max which is different from <algorithm>.
// Must be included before headers that include WinDef.h.
// http://support.microsoft.com/kb/143208
#define NOMINMAX

#include <cassert>

// C++ Standard Library.
#include <algorithm>
#include <array>
#include <cstdint>
#include <exception>
#include <functional>
#include <memory>
#include <string>
#include <vector>

// Windows API.
#ifdef _WIN32

// Define to prevent using namespace ATL by default.
// http://msdn.microsoft.com/en-us/library/a477k694.aspx
#define _ATL_NO_AUTOMATIC_NAMESPACE

// CComPtr is the only ATL component that is used.  Do not accidentally link to the ATL libs.
#define _ATL_NO_DEFAULT_LIBS

// Prevent CComPtr from throwing an exception on operator*.
#define _ATL_NO_EXCEPTIONS

// For ATL::CComPtr.  CComPtr is used (as opposed to _com_ptr_t) because it doesn't throw exceptions (unlike _com_ptr_t,
// which throws exceptions not rooted in std::exception), it returns HRESULTs which integrates well with code that uses
// DirectX, and doesn't release it's interface pointer in operator&.
#include <atlcomcli.h>

// Defines to decrease build times:
// http://support.microsoft.com/kb/166474
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <strsafe.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#endif  // _WIN32

#include <gl/GL.h>

