#pragma once

namespace WindowsCommon
{

// TODO: There is an argument for not putting this in WindowsCommon.
// It is MSVC specific at this point, and assumes that the FPU is preserved across a context switch, and
// there may be a better abstraction when there are multiple FPUs on a system (e.g. x87 vs SSE).
class Scoped_FPU_exception_control
{
    unsigned int m_original_control;
    unsigned int m_exception_mask;

    // Prevent copy.
    Scoped_FPU_exception_control& operator=(const Scoped_FPU_exception_control&) = delete;
    Scoped_FPU_exception_control(const Scoped_FPU_exception_control&) = delete;

public:
    Scoped_FPU_exception_control(unsigned int exception_mask);
    ~Scoped_FPU_exception_control();
    void enable(unsigned int fpu_exceptions);
    void disable(unsigned int fpu_exceptions);

    unsigned int current_control() const;
};

}

