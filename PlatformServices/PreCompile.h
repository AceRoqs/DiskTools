#pragma once

#include <cassert>

// C++ Standard Library.
#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <vector>

// Windows API.
#ifdef _WIN32

// Defines to decrease build times:
// http://support.microsoft.com/kb/166474
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <shellapi.h>

#endif

