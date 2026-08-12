#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <ctype.h>

#include "webm_channel_dialog.h"

#define WEBM_CHANNEL_DIALOG_CLASS "NC_TK17_WEBM_CHANNEL_DIALOG"
#define WEBM_CHANNEL_EDIT_ID 4101

typedef struct {
    char *output;
    size_t output_size;
    const char *initial_channel;
    HWND edit;
    int accepted;
} webm_channel_dialog_state_t;

typedef struct {
    DWORD process_id;
    HWND window;
} webm_window_search_t;

static void set_control_font(HWND control)
{
    SendMessageA(control, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
}

static void copy_trimmed_channel(HWND edit, char *output, size_t output_size)
{
    char buffer[2048];
    char *start;
    char *end;
    size_t length;
    if (!output || !output_size) return;
    output[0] = 0;
    GetWindowTextA(edit, buffer, sizeof(buffer));
    start = buffer;
    while (*start && isspace((unsigned char)*start)) start++;
    end = start + strlen(start);
    while (end > start && isspace((unsigned char)end[-1])) end--;
    length = (size_t)(end - start);
    if (length >= output_size) length = output_size - 1;
    memcpy(output, start, length);
    output[length] = 0;
}

static LRESULT CALLBACK webm_channel_dialog_proc(HWND window, UINT message,
                                                  WPARAM wparam, LPARAM lparam)
{
    webm_channel_dialog_state_t *state =
        (webm_channel_dialog_state_t*)GetWindowLongPtrA(window, GWLP_USERDATA);
    if (message == WM_NCCREATE) {
        CREATESTRUCTA *create = (CREATESTRUCTA*)lparam;
        state = (webm_channel_dialog_state_t*)create->lpCreateParams;
        SetWindowLongPtrA(window, GWLP_USERDATA, (LONG_PTR)state);
    }
    switch (message) {
    case WM_CREATE: {
        HWND control;
        control = CreateWindowExA(0, "STATIC",
            "Enter a channel name, full Twitch URL, or comma-separated channel list.",
            WS_CHILD | WS_VISIBLE, 18, 16, 444, 18, window, NULL, NULL, NULL);
        set_control_font(control);
        state->edit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT",
            state->initial_channel ? state->initial_channel : "",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
            18, 42, 444, 24, window, (HMENU)WEBM_CHANNEL_EDIT_ID, NULL, NULL);
        SendMessageA(state->edit, EM_SETLIMITTEXT, 2000, 0);
        set_control_font(state->edit);
        control = CreateWindowExA(0, "BUTTON", "Apply",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
            292, 82, 80, 26, window, (HMENU)IDOK, NULL, NULL);
        set_control_font(control);
        control = CreateWindowExA(0, "BUTTON", "Cancel",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
            382, 82, 80, 26, window, (HMENU)IDCANCEL, NULL, NULL);
        set_control_font(control);
        SetFocus(state->edit);
        SendMessageA(state->edit, EM_SETSEL, 0, -1);
        return 0;
    }
    case WM_COMMAND:
        if (LOWORD(wparam) == IDOK) {
            copy_trimmed_channel(state->edit, state->output, state->output_size);
            state->accepted = 1;
            DestroyWindow(window);
            return 0;
        }
        if (LOWORD(wparam) == IDCANCEL) {
            DestroyWindow(window);
            return 0;
        }
        break;
    case WM_CLOSE:
        DestroyWindow(window);
        return 0;
    }
    return DefWindowProcA(window, message, wparam, lparam);
}

static BOOL CALLBACK find_process_window(HWND window, LPARAM parameter)
{
    webm_window_search_t *search = (webm_window_search_t*)parameter;
    DWORD process_id = 0;
    GetWindowThreadProcessId(window, &process_id);
    if (process_id == search->process_id && IsWindowVisible(window) &&
        GetWindow(window, GW_OWNER) == NULL) {
        search->window = window;
        return FALSE;
    }
    return TRUE;
}

static HWND find_parent_window(void)
{
    webm_window_search_t search;
    HWND window = GetForegroundWindow();
    DWORD process_id = 0;
    if (window) GetWindowThreadProcessId(window, &process_id);
    if (process_id == GetCurrentProcessId()) return window;
    memset(&search, 0, sizeof(search));
    search.process_id = GetCurrentProcessId();
    EnumWindows(find_process_window, (LPARAM)&search);
    return search.window;
}

static int register_dialog_class(HINSTANCE instance)
{
    WNDCLASSEXA cls;
    memset(&cls, 0, sizeof(cls));
    cls.cbSize = sizeof(cls);
    cls.lpfnWndProc = webm_channel_dialog_proc;
    cls.hInstance = instance;
    cls.hCursor = LoadCursorA(NULL, IDC_ARROW);
    cls.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    cls.lpszClassName = WEBM_CHANNEL_DIALOG_CLASS;
    if (RegisterClassExA(&cls)) return 1;
    return GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

int webm_channel_dialog_show(HINSTANCE instance, const char *initial_channel,
                             char *out_channel, size_t out_size)
{
    webm_channel_dialog_state_t state;
    HWND parent;
    HWND window;
    RECT parent_rect;
    RECT dialog_rect = {0, 0, 488, 148};
    MSG message;
    int x;
    int y;
    int get_message_result = 1;
    if (!instance || !out_channel || !out_size || !register_dialog_class(instance)) return 0;
    memset(&state, 0, sizeof(state));
    state.output = out_channel;
    state.output_size = out_size;
    state.initial_channel = initial_channel;
    parent = find_parent_window();
    AdjustWindowRectEx(&dialog_rect, WS_CAPTION | WS_SYSMENU, FALSE, WS_EX_DLGMODALFRAME);
    if (parent && GetWindowRect(parent, &parent_rect)) {
        x = parent_rect.left + ((parent_rect.right - parent_rect.left) -
            (dialog_rect.right - dialog_rect.left)) / 2;
        y = parent_rect.top + ((parent_rect.bottom - parent_rect.top) -
            (dialog_rect.bottom - dialog_rect.top)) / 2;
    } else {
        x = (GetSystemMetrics(SM_CXSCREEN) - (dialog_rect.right - dialog_rect.left)) / 2;
        y = (GetSystemMetrics(SM_CYSCREEN) - (dialog_rect.bottom - dialog_rect.top)) / 2;
    }
    window = CreateWindowExA(WS_EX_DLGMODALFRAME, WEBM_CHANNEL_DIALOG_CLASS,
        "NC-TK17-WebM - Twitch Channel", WS_POPUP | WS_CAPTION | WS_SYSMENU,
        x, y, dialog_rect.right - dialog_rect.left, dialog_rect.bottom - dialog_rect.top,
        parent, NULL, instance, &state);
    if (!window) return 0;
    if (parent) EnableWindow(parent, FALSE);
    ShowWindow(window, SW_SHOW);
    UpdateWindow(window);
    while (IsWindow(window) && (get_message_result = GetMessageA(&message, NULL, 0, 0)) > 0) {
        if (!IsDialogMessageA(window, &message)) {
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
    }
    if (parent && IsWindow(parent)) {
        EnableWindow(parent, TRUE);
        SetForegroundWindow(parent);
    }
    if (get_message_result == 0) PostQuitMessage((int)message.wParam);
    return state.accepted;
}
