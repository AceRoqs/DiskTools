#pragma once

namespace WindowsCommon
{

// TODO: Consider making this class private once get_error_string is removed.
class HRESULT_exception : public std::exception
{
public:
    HRESULT_exception(HRESULT hr) NOEXCEPT;
    HRESULT_exception(HRESULT hr, _In_opt_z_ const char* message) NOEXCEPT;
    HRESULT_exception(const HRESULT_exception& that) NOEXCEPT;
    HRESULT_exception& operator=(const HRESULT_exception& that) NOEXCEPT;

    // Define a method besides exception::what() that doesn't require heap memory allocation.
    virtual void get_error_string(_Out_writes_z_(size) PTSTR error_string, size_t size) const NOEXCEPT;
    virtual const char* what() const NOEXCEPT override;

    HRESULT m_hr;

private:
    std::unique_ptr<char[]> m_error_string;
};

HRESULT hresult_from_last_error() NOEXCEPT;
void check_hr(HRESULT hr);
void check_windows_error(BOOL result);
void check_with_custom_hr(BOOL result, HRESULT hr);

// These macros should only be used to work around static analysis warnings.
// TODO: Passing a function as expr is not valid.  Check this.
#define CHECK_WINDOWS_ERROR(expr) { WindowsCommon::check_windows_error(expr); _Analysis_assume_(expr); }
#define CHECK_WITH_CUSTOM_HR(expr, hr) { WindowsCommon::check_with_custom_hr(expr, hr); _Analysis_assume_(expr); }

} // namespace WindowsCommon

