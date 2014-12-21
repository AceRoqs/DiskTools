#pragma once

namespace PortableRuntime
{

// TODO: Make this not inline, and verify that the compiler will inline during LTCG.
inline void verify(DWORD dw)
{
    (dw);   // Prevent unreferenced parameter in Release build.
    assert(dw != 0);
}

}

// TODO: Move this to a WindowsCommon specific header.
namespace WindowsCommon
{

// TODO: Make this not inline, and verify that the compiler will inline during LTCG.
inline void verify_hr(HRESULT hr)
{
    PortableRuntime::verify(SUCCEEDED(hr));
}

}

