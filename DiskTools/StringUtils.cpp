#include "PreCompile.h"
#include "StringUtils.h"    // Pick up forward declarations to ensure correctness.

//---------------------------------------------------------------------------
static int get_locale_info_number(
    LCID locale,
    LCTYPE type,
    _Out_ UINT* number)
{
    // The size of the buffer is in TCHARS, even if this function is requesting
    // a number returned.
    // http://msdn.microsoft.com/en-us/library/dd318101.aspx
    return ::GetLocaleInfo(locale,
                           type | LOCALE_RETURN_NUMBER,
                           reinterpret_cast<PTSTR>(number),
                           sizeof(*number) / sizeof(TCHAR));
}

//---------------------------------------------------------------------------
// Wrapper for NUMBERFMT.
class Number_format
{
    NUMBERFMT m_number_format;
    TCHAR m_decimal_separator[8];
    TCHAR m_thousands_separator[8];

public:
    Number_format();

    void init_by_LCID(LCID lcid);
    void strip_decimal();

    void get_number_format(_Out_ NUMBERFMT* pnf);
};

//---------------------------------------------------------------------------
Number_format::Number_format()
{
    ::ZeroMemory(&m_number_format, sizeof(m_number_format));
}

//---------------------------------------------------------------------------
void Number_format::init_by_LCID(LCID lcid)
{
    // No need to check for return codes, as failure only occurs for invalid parameters.
    get_locale_info_number(lcid, LOCALE_IDIGITS, &m_number_format.NumDigits);
    get_locale_info_number(lcid, LOCALE_ILZERO, &m_number_format.LeadingZero);
    get_locale_info_number(lcid, LOCALE_INEGNUMBER, &m_number_format.NegativeOrder);

    TCHAR buffer[16];

    ::GetLocaleInfo(lcid, LOCALE_SGROUPING, buffer, ARRAYSIZE(buffer));
    PTSTR string_pointer = buffer;
    while(*string_pointer)
    {
        // This works for ASCII based character sets only.
        if((*string_pointer >= TEXT('1')) && (*string_pointer <= TEXT('9')))
        {
            m_number_format.Grouping = m_number_format.Grouping * 10 + (*string_pointer - TEXT('0'));
        }
        ++string_pointer;
    }

    ::GetLocaleInfo(lcid, LOCALE_SDECIMAL, m_decimal_separator, ARRAYSIZE(m_decimal_separator));
    m_number_format.lpDecimalSep = m_decimal_separator;

    ::GetLocaleInfo(lcid, LOCALE_SMONTHOUSANDSEP, m_thousands_separator, ARRAYSIZE(m_thousands_separator));
    m_number_format.lpThousandSep = m_thousands_separator;
}

//---------------------------------------------------------------------------
void Number_format::strip_decimal()
{
    m_number_format.NumDigits = 0;
}

//---------------------------------------------------------------------------
void Number_format::get_number_format(_Out_ NUMBERFMT* pnf)
{
    ::CopyMemory(pnf, &m_number_format, sizeof(m_number_format));
}

//---------------------------------------------------------------------------
static void output_formatted_number(
    PTSTR number_string,
    _Out_z_cap_(size_in_chars) PTSTR output_string,
    _In_range_(0, INT_MAX) size_t size_in_chars)
{
    const LCID locale = LOCALE_USER_DEFAULT;

    Number_format number_format;
    number_format.init_by_LCID(locale);
    number_format.strip_decimal();

    NUMBERFMT number_format_no_decimal;
    number_format.get_number_format(&number_format_no_decimal);

    if(0 == ::GetNumberFormat(locale, 0, number_string, &number_format_no_decimal, output_string, static_cast<int>(size_in_chars)))
    {
        // Output the string without formatting.
        if(FAILED(::StringCchCopy(output_string, size_in_chars, number_string)))
        {
            assert(!"Input buffer not large enough for StringCchCopy.");
        }
    }
}

//---------------------------------------------------------------------------
void pretty_print32(
    uint32_t value,
    _Out_z_cap_(size_in_chars) PTSTR output_string,
    _In_range_(0, INT_MAX) size_t size_in_chars)
{
    TCHAR temp_buffer[11]; // Enough space to hold 2^32, including null, but not including commas.
    _ultot_s(value, temp_buffer, ARRAYSIZE(temp_buffer), 10);

    output_formatted_number(temp_buffer, output_string, size_in_chars);
}

//---------------------------------------------------------------------------
void pretty_print64(
    uint64_t value,
    _Out_z_cap_(size_in_chars) PTSTR output_string,
    _In_range_(0, INT_MAX) size_t size_in_chars)
{
    TCHAR temp_buffer[21]; // Enough space to hold 2^64, including null, but not including commas.
    _ui64tot_s(value, temp_buffer, ARRAYSIZE(temp_buffer), 10);

    output_formatted_number(temp_buffer, output_string, size_in_chars);
}

