#ifdef PRECOMP_H
#error Precompiled header must be included only once.
#endif
#define PRECOMP_H

// Disable warnings that are informational.
#pragma warning(disable:4711)  // Function selected for inline expansion.
#pragma warning(disable:4820)  // Padding after data member.

// Disable warnings due to system headers not being warning clean.
#pragma warning(disable:4365)  // assert() has signed/unsigned mismatch.
#pragma warning(disable:4514)  // Unreferenced inline function.
#pragma warning(disable:4710)  // Function not inlined.

#pragma warning(push)
#pragma warning(disable:4668)  // Preprocessor macro not defined.
#pragma warning(disable:4986)  // operator new[]': exception specification does not match previous declaration.

#include <cassert>
#include <cstdint>
#include <algorithm>
#include <functional>
#include <memory>
#include <tchar.h>
#include <windows.h>
#include <commctrl.h>
#include <strsafe.h>

#pragma warning(pop)

