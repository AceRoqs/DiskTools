#pragma once

#include <WindowsCommon/ScopedWindowsTypes.h>
#include <WindowsCommon/Wrappers.h>

namespace WindowsCommon
{

typedef Scoped_resource<HGLRC> Scoped_gl_context;
typedef Scoped_resource<HGLRC> Scoped_current_context;

struct WGL_state
{
// http://stackoverflow.com/questions/70013/how-to-detect-if-im-compiling-code-with-visual-studio-2008
#if _MSC_VER <= 1800
    // Add default and move constructors as VS2010/2012/2013 doesn't default define a move constructor.
    WGL_state();
    WGL_state(WGL_state&& other);

    WGL_state& operator=(WGL_state&& other) NOEXCEPT;
#else
#error This compiler may autodefine the default move constructor.
#endif

    // The order of these fields matter, as destruction must happen in the opposite order.
    Scoped_atom atom;
    Scoped_window window;
    Scoped_device_context device_context;
    Scoped_gl_context gl_context;
    Scoped_current_context make_current_context;
};

class OpenGL_window : public Window_procedure
{
public:
    OpenGL_window(_In_ PCSTR window_title, _In_ HINSTANCE instance, bool windowed);
    ~OpenGL_window();

    WGL_state m_state;

protected:
    LRESULT window_proc(_In_ HWND window, UINT message, WPARAM w_param, LPARAM l_param) NOEXCEPT OVERRIDE;

private:
    bool m_windowed;
};

Scoped_gl_context create_gl_context(_In_ HDC device_context);
Scoped_current_context create_current_context(_In_ HDC device_context, _In_ HGLRC gl_context);
Scoped_gl_context make_scoped_gl_context(_In_ HGLRC gl_context);
Scoped_current_context make_scoped_current_context(_In_ HGLRC gl_context);

}

