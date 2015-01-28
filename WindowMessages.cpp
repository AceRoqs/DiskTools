#include "PreCompile.h"

namespace WindowsCommon
{

#define MESSAGE(message) { #message, message }

static const struct Message_map
{
    PCSTR text;
    UINT message;
} window_messages[] =
{
    // Message list generated from VC2010 SDK on 7/24/2014.
    MESSAGE(WM_NULL),
    MESSAGE(WM_CREATE),
    MESSAGE(WM_DESTROY),
    MESSAGE(WM_MOVE),
    MESSAGE(WM_SIZE),
    MESSAGE(WM_ACTIVATE),
    MESSAGE(WM_SETFOCUS),
    MESSAGE(WM_KILLFOCUS),
    MESSAGE(WM_ENABLE),
    MESSAGE(WM_SETREDRAW),
    MESSAGE(WM_SETTEXT),
    MESSAGE(WM_GETTEXT),
    MESSAGE(WM_GETTEXTLENGTH),
    MESSAGE(WM_PAINT),
    MESSAGE(WM_CLOSE),
    MESSAGE(WM_QUERYENDSESSION),
    MESSAGE(WM_QUIT),
    MESSAGE(WM_QUERYOPEN),
    MESSAGE(WM_ERASEBKGND),
    MESSAGE(WM_SYSCOLORCHANGE),
    MESSAGE(WM_ENDSESSION),
    MESSAGE(WM_SHOWWINDOW),
    MESSAGE(WM_SETTINGCHANGE),
    MESSAGE(WM_DEVMODECHANGE),
    MESSAGE(WM_ACTIVATEAPP),
    MESSAGE(WM_FONTCHANGE),
    MESSAGE(WM_TIMECHANGE),
    MESSAGE(WM_CANCELMODE),
    MESSAGE(WM_SETCURSOR),
    MESSAGE(WM_MOUSEACTIVATE),
    MESSAGE(WM_CHILDACTIVATE),
    MESSAGE(WM_QUEUESYNC),
    MESSAGE(WM_GETMINMAXINFO),
    MESSAGE(WM_PAINTICON),
    MESSAGE(WM_ICONERASEBKGND),
    MESSAGE(WM_NEXTDLGCTL),
    MESSAGE(WM_SPOOLERSTATUS),
    MESSAGE(WM_DRAWITEM),
    MESSAGE(WM_MEASUREITEM),
    MESSAGE(WM_DELETEITEM),
    MESSAGE(WM_VKEYTOITEM),
    MESSAGE(WM_CHARTOITEM),
    MESSAGE(WM_SETFONT),
    MESSAGE(WM_GETFONT),
    MESSAGE(WM_SETHOTKEY),
    MESSAGE(WM_GETHOTKEY),
    MESSAGE(WM_QUERYDRAGICON),
    MESSAGE(WM_COMPAREITEM),
    MESSAGE(WM_GETOBJECT),
    MESSAGE(WM_COMPACTING),
    MESSAGE(WM_COMMNOTIFY),
    MESSAGE(WM_WINDOWPOSCHANGING),
    MESSAGE(WM_WINDOWPOSCHANGED),
    MESSAGE(WM_POWER),
    MESSAGE(WM_COPYDATA),
    MESSAGE(WM_CANCELJOURNAL),
    MESSAGE(WM_NOTIFY),
    MESSAGE(WM_INPUTLANGCHANGEREQUEST),
    MESSAGE(WM_INPUTLANGCHANGE),
    MESSAGE(WM_TCARD),
    MESSAGE(WM_HELP),
    MESSAGE(WM_USERCHANGED),
    MESSAGE(WM_NOTIFYFORMAT),
    MESSAGE(WM_CONTEXTMENU),
    MESSAGE(WM_STYLECHANGING),
    MESSAGE(WM_STYLECHANGED),
    MESSAGE(WM_DISPLAYCHANGE),
    MESSAGE(WM_GETICON),
    MESSAGE(WM_SETICON),
    MESSAGE(WM_NCCREATE),
    MESSAGE(WM_NCDESTROY),
    MESSAGE(WM_NCCALCSIZE),
    MESSAGE(WM_NCHITTEST),
    MESSAGE(WM_NCPAINT),
    MESSAGE(WM_NCACTIVATE),
    MESSAGE(WM_GETDLGCODE),
    MESSAGE(WM_SYNCPAINT),
    MESSAGE(WM_NCMOUSEMOVE),
    MESSAGE(WM_NCLBUTTONDOWN),
    MESSAGE(WM_NCLBUTTONUP),
    MESSAGE(WM_NCLBUTTONDBLCLK),
    MESSAGE(WM_NCRBUTTONDOWN),
    MESSAGE(WM_NCRBUTTONUP),
    MESSAGE(WM_NCRBUTTONDBLCLK),
    MESSAGE(WM_NCMBUTTONDOWN),
    MESSAGE(WM_NCMBUTTONUP),
    MESSAGE(WM_NCMBUTTONDBLCLK),
    MESSAGE(WM_NCXBUTTONDOWN),
    MESSAGE(WM_NCXBUTTONUP),
    MESSAGE(WM_NCXBUTTONDBLCLK),
    MESSAGE(WM_INPUT_DEVICE_CHANGE),
    MESSAGE(WM_INPUT),
    MESSAGE(WM_KEYDOWN),            // WM_KEYFIRST.
    MESSAGE(WM_KEYUP),
    MESSAGE(WM_CHAR),
    MESSAGE(WM_DEADCHAR),
    MESSAGE(WM_SYSKEYDOWN),
    MESSAGE(WM_SYSKEYUP),
    MESSAGE(WM_SYSCHAR),
    MESSAGE(WM_SYSDEADCHAR),
    MESSAGE(WM_UNICHAR),            // WM_KEYLAST.
    MESSAGE(WM_IME_STARTCOMPOSITION),
    MESSAGE(WM_IME_ENDCOMPOSITION),
    MESSAGE(WM_IME_COMPOSITION),
    MESSAGE(WM_INITDIALOG),
    MESSAGE(WM_COMMAND),
    MESSAGE(WM_SYSCOMMAND),
    MESSAGE(WM_TIMER),
    MESSAGE(WM_HSCROLL),
    MESSAGE(WM_VSCROLL),
    MESSAGE(WM_INITMENU),
    MESSAGE(WM_INITMENUPOPUP),
    MESSAGE(WM_GESTURE),
    MESSAGE(WM_GESTURENOTIFY),
    MESSAGE(WM_MENUSELECT),
    MESSAGE(WM_MENUCHAR),
    MESSAGE(WM_ENTERIDLE),
    MESSAGE(WM_MENURBUTTONUP),
    MESSAGE(WM_MENUDRAG),
    MESSAGE(WM_MENUGETOBJECT),
    MESSAGE(WM_UNINITMENUPOPUP),
    MESSAGE(WM_MENUCOMMAND),
    MESSAGE(WM_CHANGEUISTATE),
    MESSAGE(WM_UPDATEUISTATE),
    MESSAGE(WM_QUERYUISTATE),
    MESSAGE(WM_CTLCOLORMSGBOX),
    MESSAGE(WM_CTLCOLOREDIT),
    MESSAGE(WM_CTLCOLORLISTBOX),
    MESSAGE(WM_CTLCOLORBTN),
    MESSAGE(WM_CTLCOLORDLG),
    MESSAGE(WM_CTLCOLORSCROLLBAR),
    MESSAGE(WM_CTLCOLORSTATIC),
    MESSAGE(MN_GETHMENU),
    MESSAGE(WM_MOUSEMOVE),          // WM_MOUSEFIRST.
    MESSAGE(WM_LBUTTONDOWN),
    MESSAGE(WM_LBUTTONUP),
    MESSAGE(WM_LBUTTONDBLCLK),
    MESSAGE(WM_RBUTTONDOWN),
    MESSAGE(WM_RBUTTONUP),
    MESSAGE(WM_RBUTTONDBLCLK),
    MESSAGE(WM_MBUTTONDOWN),
    MESSAGE(WM_MBUTTONUP),
    MESSAGE(WM_MBUTTONDBLCLK),
    MESSAGE(WM_MOUSEWHEEL),
    MESSAGE(WM_XBUTTONDOWN),
    MESSAGE(WM_XBUTTONUP),
    MESSAGE(WM_XBUTTONDBLCLK),      // WM_MOUSELAST.
    MESSAGE(WM_MOUSEHWHEEL),
    MESSAGE(WM_PARENTNOTIFY),
    MESSAGE(WM_ENTERMENULOOP),
    MESSAGE(WM_EXITMENULOOP),
    MESSAGE(WM_NEXTMENU),
    MESSAGE(WM_SIZING),
    MESSAGE(WM_CAPTURECHANGED),
    MESSAGE(WM_MOVING),
    MESSAGE(WM_POWERBROADCAST),
    MESSAGE(WM_DEVICECHANGE),
    MESSAGE(WM_MDICREATE),
    MESSAGE(WM_MDIDESTROY),
    MESSAGE(WM_MDIACTIVATE),
    MESSAGE(WM_MDIRESTORE),
    MESSAGE(WM_MDINEXT),
    MESSAGE(WM_MDIMAXIMIZE),
    MESSAGE(WM_MDITILE),
    MESSAGE(WM_MDICASCADE),
    MESSAGE(WM_MDIICONARRANGE),
    MESSAGE(WM_MDIGETACTIVE),
    MESSAGE(WM_MDISETMENU),
    MESSAGE(WM_ENTERSIZEMOVE),
    MESSAGE(WM_EXITSIZEMOVE),
    MESSAGE(WM_DROPFILES),
    MESSAGE(WM_MDIREFRESHMENU),
    MESSAGE(WM_TOUCH),
    MESSAGE(WM_IME_SETCONTEXT),
    MESSAGE(WM_IME_NOTIFY),
    MESSAGE(WM_IME_CONTROL),
    MESSAGE(WM_IME_COMPOSITIONFULL),
    MESSAGE(WM_IME_SELECT),
    MESSAGE(WM_IME_CHAR),
    MESSAGE(WM_IME_REQUEST),
    MESSAGE(WM_IME_KEYDOWN),
    MESSAGE(WM_IME_KEYUP),
    MESSAGE(WM_NCMOUSEHOVER),
    MESSAGE(WM_MOUSEHOVER),
    MESSAGE(WM_NCMOUSELEAVE),
    MESSAGE(WM_MOUSELEAVE),
    MESSAGE(WM_WTSSESSION_CHANGE),
    MESSAGE(WM_TABLET_FIRST),
    MESSAGE(WM_TABLET_LAST),
    MESSAGE(WM_CUT),
    MESSAGE(WM_COPY),
    MESSAGE(WM_PASTE),
    MESSAGE(WM_CLEAR),
    MESSAGE(WM_UNDO),
    MESSAGE(WM_RENDERFORMAT),
    MESSAGE(WM_RENDERALLFORMATS),
    MESSAGE(WM_DESTROYCLIPBOARD),
    MESSAGE(WM_DRAWCLIPBOARD),
    MESSAGE(WM_PAINTCLIPBOARD),
    MESSAGE(WM_VSCROLLCLIPBOARD),
    MESSAGE(WM_SIZECLIPBOARD),
    MESSAGE(WM_ASKCBFORMATNAME),
    MESSAGE(WM_CHANGECBCHAIN),
    MESSAGE(WM_HSCROLLCLIPBOARD),
    MESSAGE(WM_QUERYNEWPALETTE),
    MESSAGE(WM_PALETTEISCHANGING),
    MESSAGE(WM_PALETTECHANGED),
    MESSAGE(WM_HOTKEY),
    MESSAGE(WM_PRINT),
    MESSAGE(WM_PRINTCLIENT),
    MESSAGE(WM_APPCOMMAND),
    MESSAGE(WM_THEMECHANGED),
    MESSAGE(WM_CLIPBOARDUPDATE),
    MESSAGE(WM_DWMCOMPOSITIONCHANGED),
    MESSAGE(WM_DWMNCRENDERINGCHANGED),
    MESSAGE(WM_DWMCOLORIZATIONCOLORCHANGED),
    MESSAGE(WM_DWMWINDOWMAXIMIZEDCHANGE),
    MESSAGE(WM_DWMSENDICONICTHUMBNAIL),
    MESSAGE(WM_DWMSENDICONICLIVEPREVIEWBITMAP),
    MESSAGE(WM_GETTITLEBARINFOEX),
    MESSAGE(WM_HANDHELDFIRST),
    MESSAGE(WM_HANDHELDLAST),
    MESSAGE(WM_AFXFIRST),
    MESSAGE(WM_AFXLAST),
    MESSAGE(WM_PENWINFIRST),
    MESSAGE(WM_PENWINLAST),
};
#undef MESSAGE

static bool operator<(const Message_map& first, const Message_map& second)
{
    return first.message < second.message;
}

static _Success_(return) bool map_message_to_index(_In_reads_(map_size) const Message_map* message_map, size_t map_size, unsigned int message, _Out_ size_t* final_index) NOEXCEPT
{
    const Message_map value = { nullptr, message };
    const auto iterator = std::lower_bound(message_map, message_map + map_size, value);
    const bool found = (iterator != message_map + map_size) && (message == iterator->message);
    if(found)
    {
        *final_index = iterator - message_map;
    }

    return found;
}

#ifndef NDEBUG // These functions are not currently called in Release.  Avoid C4505: unreferenced local function has been removed.
static void validate_message_map_sorted(_In_reads_(map_size) const Message_map* message_map, size_t map_size) NOEXCEPT
{
    assert(std::is_sorted(message_map, message_map + map_size));
}

static void validate_message_map_all_entries_accessible(_In_reads_(map_size) const Message_map* message_map, size_t map_size) NOEXCEPT
{
    std::for_each(message_map, message_map + map_size, [&](const Message_map& entry){
        size_t message_index;
        assert(map_message_to_index(message_map, map_size, entry.message, &message_index));
        assert(entry.message == message_map[message_index].message);
    });
}
#endif

void debug_validate_message_map() NOEXCEPT
{
#ifndef NDEBUG
    validate_message_map_sorted(window_messages, ARRAYSIZE(window_messages));
    validate_message_map_all_entries_accessible(window_messages, ARRAYSIZE(window_messages));
    assert(window_messages[ARRAYSIZE(window_messages) - 1].message < WM_USER);
#endif
}

PCSTR string_from_window_message(UINT message) NOEXCEPT
{
    PCSTR text;

    // TODO: Add %04x to the strings below to track the message number.
    size_t index;
    if(message >= WM_APP)       // WM_APP = 0x8000.
    {
        text = "WM_APP private message";
    }
    else if(message >= WM_USER) // WM_USER = 0x0400.
    {
        text = "WM_USER private message";
    }
    else if(map_message_to_index(window_messages, ARRAYSIZE(window_messages), message, &index))
    {
        text = window_messages[index].text;
    }
    else
    {
        text = "Unknown message";
    }

    return text;
}

// Returns false if WM_QUIT was posted.
bool dispatch_all_windows_messages(_Out_ MSG* message) NOEXCEPT
{
    // Clear out all the messages before drawing a new frame.
    BOOL message_exists = PeekMessage(message, nullptr, 0, 0, PM_REMOVE);
    while(message_exists)
    {
        if(WM_QUIT == message->message)
        {
            break;
        }

        TranslateMessage(message);
        DispatchMessageW(message);

        message_exists = PeekMessageW(message, nullptr, 0, 0, PM_REMOVE);
    }

    return !message_exists || (WM_QUIT != message->message);
}

}

