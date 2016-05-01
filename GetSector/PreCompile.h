#pragma once

#include <cassert>
#include <array>
#include <cstdint>
#include <fstream>
#include <memory>
#include <vector>

#ifdef _MSC_VER

// TODO: 2016: Remove all instances of tchar.h.
#include <tchar.h>
#include <windows.h>

// APIs for MSVCRT UTF-8 output.
#include <fcntl.h>
#include <io.h>

#endif

