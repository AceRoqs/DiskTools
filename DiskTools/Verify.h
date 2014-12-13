#pragma once

inline void verify(DWORD dw)
{
    (dw);   // Prevent unreferenced parameter in Release build.
    assert(dw != 0);
}

inline void verify_hr(HRESULT hr)
{
    (hr);   // Prevent unreferenced parameter in Release build.
    assert(SUCCEEDED(hr));
}

// This macro should only be used to work around static analysis warnings.
#define VERIFY(expr) { verify(expr); __analysis_assume(expr); }

