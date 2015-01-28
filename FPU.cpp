#include "PreCompile.h"
#include "FPU.h"                // Pick up forward declarations to ensure correctness.
#include <PortableRuntime/CheckException.h>

namespace WindowsCommon
{

#if defined(_MSC_VER)
Scoped_FPU_exception_control::Scoped_FPU_exception_control(unsigned int exception_mask)
    : m_exception_mask(exception_mask & _MCW_EM)
{
    assert((exception_mask & ~_MCW_EM) == 0);

    errno_t err = _controlfp_s(&m_original_control, 0, 0);
    PortableRuntime::check_exception(err == 0);

    m_original_control &= m_exception_mask;
}
#else
Scoped_FPU_exception_control::Scoped_FPU_exception_control(unsigned int exception_mask)
{
#error No platform support for FPU exception control.
}
#endif

Scoped_FPU_exception_control::~Scoped_FPU_exception_control()
{
#if defined(_MSC_VER)
    // Clear pending FPU exceptions, so enabling won't trigger them.
    _clearfp();

    errno_t err = _controlfp_s(nullptr, m_original_control, m_exception_mask);
    (err);  // Prevent unreferenced parameter in Release build.
    assert(err == 0);
#else
#error No platform support for FPU exception control.
#endif
}

void Scoped_FPU_exception_control::enable(unsigned int fpu_exceptions)
{
#if defined(_MSC_VER)
    assert(((m_exception_mask | fpu_exceptions) & ~m_exception_mask) == 0);

    // Clear pending FPU exceptions, so enabling won't trigger them.
    _clearfp();

    // Clearing the bit enables exception.
    errno_t err = _controlfp_s(nullptr, ~fpu_exceptions, m_exception_mask & fpu_exceptions);
    PortableRuntime::check_exception(err == 0);
#else
#error No platform support for FPU exception control.
#endif
}

void Scoped_FPU_exception_control::disable(unsigned int fpu_exceptions)
{
#if defined(_MSC_VER)
    assert(((m_exception_mask | fpu_exceptions) & ~m_exception_mask) == 0);

    // Setting the bit enables masking of exception.
    errno_t err = _controlfp_s(nullptr, fpu_exceptions, m_exception_mask & fpu_exceptions);
    PortableRuntime::check_exception(err == 0);
#else
#error No platform support for FPU exception control.
#endif
}

unsigned int Scoped_FPU_exception_control::current_control() const
{
#if defined(_MSC_VER)
    unsigned int control;
    errno_t err = _controlfp_s(&control, 0, 0);
    PortableRuntime::check_exception(err == 0);

    return control &= m_exception_mask;
#else
#error No platform support for FPU exception control.
#endif
}

}

