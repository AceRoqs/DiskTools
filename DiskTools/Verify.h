/*
Copyright (C) 2000-2014 by Toby Jones.

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

#ifndef VERIFY_H
#define VERIFY_H

//---------------------------------------------------------------------------
inline void verify(DWORD dw)
{
    (dw);   // Prevent unreferenced parameter in Release build.
    assert(dw != 0);
}

//---------------------------------------------------------------------------
inline void verify_hr(HRESULT hr)
{
    (hr);   // Prevent unreferenced parameter in Release build.
    assert(SUCCEEDED(hr));
}

// This macro should only be used to work around static analysis warnings.
#define VERIFY(expr) { verify(expr); __analysis_assume(expr); }

#endif

