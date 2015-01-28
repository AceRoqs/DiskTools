#include "PreCompile.h"
#include "InputDevice.h"    // Pick up forward declarations to ensure correctness.
#include "CheckHR.h"

namespace WindowsCommon
{

Input_device::Input_device(_In_ HINSTANCE instance, _In_ HWND hwnd)
{
    // Create DirectInput keyboard device.
    ATL::CComPtr<IDirectInput8> direct_input;
    check_hr(DirectInput8Create(instance, DIRECTINPUT_VERSION, IID_IDirectInput8, reinterpret_cast<PVOID*>(&direct_input), nullptr));
    check_hr(direct_input->CreateDevice(GUID_SysKeyboard, &m_device, nullptr));

    assert(c_dfDIKeyboard.dwDataSize == keyboard_buffer_size);
    check_hr(m_device->SetDataFormat(&c_dfDIKeyboard));
    check_hr(m_device->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE));

    // Acquisition is done before input is read the first time.

    direct_input.Release();
}

Input_device::~Input_device()
{
    m_device->Unacquire();
}

void Input_device::get_input(_Out_ Keyboard_state* keyboard_state) const
{
    if(SUCCEEDED(m_device->Acquire()))
    {
        check_hr(m_device->GetDeviceState(static_cast<DWORD>(keyboard_state->size()), keyboard_state));
    }
    else
    {
        // Assume no state change if device cannot be acquired.
        keyboard_state->fill(0);
    }
}

}

