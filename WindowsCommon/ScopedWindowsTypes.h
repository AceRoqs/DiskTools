#pragma once

namespace WindowsCommon
{

template <typename RESOURCE, typename DELETER=std::function<void (RESOURCE)>>
class Scoped_resource
{
    DELETER m_deleter;
    RESOURCE m_resource;

    // Prevent copy.
    Scoped_resource& operator=(const Scoped_resource&) EQUALS_DELETE;
    Scoped_resource(const Scoped_resource&) EQUALS_DELETE;

public:
    explicit Scoped_resource() NOEXCEPT :
        m_resource(0)
    {
    }

    explicit Scoped_resource(RESOURCE resource, DELETER&& deleter) NOEXCEPT :
        m_deleter(std::move(deleter)),
        m_resource(resource)
    {
    }

    ~Scoped_resource()
    {
        invoke();
    }

    Scoped_resource(Scoped_resource&& other) NOEXCEPT :
        m_deleter(std::move(other.m_deleter)),
        m_resource(std::move(other.m_resource))
    {
        other.release();
    }

    Scoped_resource& operator=(Scoped_resource&& other) NOEXCEPT
    {
        // Handle A=A case.
        if(this != &other)
        {
            invoke();
            m_deleter = std::move(other.m_deleter);
            m_resource = std::move(other.m_resource);
            other.release();
        }

        return *this;
    }

    void invoke() NOEXCEPT
    {
        if(m_resource != 0)
        {
            m_deleter(m_resource);
            m_resource = 0;
        }
    }

    RESOURCE release() NOEXCEPT
    {
        RESOURCE resource = m_resource;
        m_resource = 0;

        return resource;
    }

    operator const RESOURCE&() const NOEXCEPT
    {
        return m_resource;
    }
};

typedef Scoped_resource<ATOM> Scoped_atom;
typedef Scoped_resource<HWND> Scoped_window;
typedef Scoped_resource<HDC> Scoped_device_context;
typedef Scoped_resource<HANDLE> Scoped_handle;
typedef Scoped_resource<HFONT> Scoped_font;
typedef Scoped_resource<HLOCAL> Scoped_local;

Scoped_atom make_scoped_window_class(_In_ ATOM atom, _In_ HINSTANCE instance);
Scoped_window make_scoped_window(_In_ HWND window);

std::function<void (HDC)> release_device_context_functor(_In_ HWND window) NOEXCEPT;
std::function<void (HDC)> end_paint_functor(_In_ HWND window, _In_ PAINTSTRUCT* paint_struct) NOEXCEPT;

Scoped_device_context make_scoped_device_context(_In_ HDC device_context, std::function<void (HDC)> deleter);
Scoped_handle make_scoped_handle(_In_ HANDLE handle);

std::function<void (HFONT)> select_object_functor(_In_ HDC device_context) NOEXCEPT;
void delete_object(_In_ HFONT font) NOEXCEPT;

Scoped_font make_scoped_font(_In_ HFONT font, std::function<void (HFONT)> deleter = delete_object);

Scoped_local make_scoped_local(_In_ HLOCAL local);

}

