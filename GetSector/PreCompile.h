#pragma once

#include <cassert>
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <unordered_map>
#include <memory>
#include <vector>

#ifdef _MSC_VER

// TODO: 2016: There should be no reason this is necessary once DiskTools are UTF-8 ready.
#include <windows.h>

// APIs for MSVCRT UTF-8 output.
#include <fcntl.h>
#include <io.h>

#endif

