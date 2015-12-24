#include "PreCompile.h"
#include "ScopedWindowsTypes.h"     // Pick up forward declarations to ensure correctness.
#include "CheckHR.h"

namespace WindowsCommon
{

static void unregister_atom(_In_ ATOM atom, _In_ HINSTANCE instance) noexcept
{
    BOOL result = UnregisterClassW(MAKEINTATOM(atom), instance);
    if(!result)
    {
        auto hr = hresult_from_last_error();
        (hr);
        assert(SUCCEEDED(hr));
    }
}

static std::function<void (ATOM)> unregister_class_functor(_In_ HINSTANCE instance) noexcept
{
    return [=](ATOM atom)
    {
        unregister_atom(atom, instance);
    };
}

Scoped_atom make_scoped_window_class(_In_ ATOM atom, _In_ HINSTANCE instance)
{
    return Scoped_atom(atom, unregister_class_functor(instance));
}

static void destroy_window(_In_ HWND window) noexcept
{
    if(!DestroyWindow(window))
    {
        auto hr = hresult_from_last_error();
        (hr);
        assert(SUCCEEDED(hr));
    }
}

Scoped_window make_scoped_window(_In_ HWND window)
{
    return Scoped_window(window, std::function<void (HWND)>(destroy_window));
}

static void release_device_context(_In_ HDC device_context, _In_ HWND window) noexcept
{
    if(!ReleaseDC(window, device_context))
    {
        assert(false);
    }
}

std::function<void (HDC)> release_device_context_functor(_In_ HWND window) noexcept
{
    return [=](HDC device_context)
    {
        release_device_context(device_context, window);
    };
}

std::function<void (HDC)> end_paint_functor(_In_ HWND window, _In_ PAINTSTRUCT* paint_struct) noexcept
{
    return [=](HDC device_context)
    {
        UNREFERENCED_PARAMETER(device_context);
        EndPaint(window, paint_struct);
    };
}

Scoped_device_context make_scoped_device_context(_In_ HDC device_context, std::function<void (HDC)> deleter)
{
    return Scoped_device_context(device_context, std::move(deleter));
}

static void close_handle(_In_ HANDLE handle) noexcept
{
    if(!CloseHandle(handle))
    {
        auto hr = hresult_from_last_error();
        (hr);
        assert(SUCCEEDED(hr));
    }
}

Scoped_handle make_scoped_handle(_In_ HANDLE handle)
{
    return Scoped_handle(handle, std::function<void (HANDLE)>(close_handle));
}

static void select_object(_In_ HDC device_context, HGDIOBJ gdi_object) noexcept
{
    auto result = SelectObject(device_context, gdi_object);
    (result);
    assert(result != nullptr);
}

std::function<void (HFONT)> select_object_functor(_In_ HDC device_context) noexcept
{
    return [=](HFONT font)
    {
        select_object(device_context, font);
    };
}

void delete_object(_In_ HFONT font) noexcept
{
    auto result = DeleteObject(font);
    (result);
    assert(result != 0);
}

Scoped_font make_scoped_font(_In_ HFONT font, std::function<void (HFONT)> deleter)
{
    return Scoped_font(font, std::move(deleter));
}

static void local_free(_In_ HLOCAL local) noexcept
{
    if(LocalFree(local) != nullptr)
    {
        auto hr = hresult_from_last_error();
        (hr);
        assert(SUCCEEDED(hr));
    }
}

Scoped_local make_scoped_local(_In_ HLOCAL local)
{
    return std::move(Scoped_local(local, std::function<void (HLOCAL)>(local_free)));
}

}

