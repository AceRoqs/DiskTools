/*
Copyright (C) 2011 by Toby Jones.

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

