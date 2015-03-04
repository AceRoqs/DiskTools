#pragma once

#include <cassert>

// C++ Standard Library.
#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <vector>

// Windows API.
#ifdef WIN32

// Defines to decrease build times:
// http://support.microsoft.com/kb/166474
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <shellapi.h>

#endif

// PreCPP11.h, being a non-system header, always goes last.
#include <PortableRuntime/PreCPP11.h>

