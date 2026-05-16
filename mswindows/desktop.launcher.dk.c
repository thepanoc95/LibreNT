/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     Windows native helper component
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



#include <windows.h>
#include <shellapi.h>

#define SIDEBAR_WIDTH 120

// slot regions
#define SLOT_HEIGHT 80

static void launch(const char* app) {
    ShellExecute(NULL, "open", app, NULL, NULL, SW_SHOWNORMAL);
}

// --- icon helpers ---
HICON icon_terminal;
HICON icon_notepad;
HICON icon_explorer;

void load_icons() {
    icon_terminal = LoadIcon(NULL, IDI_APPLICATION);
    icon_notepad  = LoadIcon(NULL, IDI_APPLICATION);
    icon_explorer = LoadIcon(NULL, IDI_APPLICATION);
}

LRESULT CALLBACK SidebarProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {

    case WM_CREATE:
        load_icons();
        break;

    case WM_LBUTTONDOWN:
        ReleaseCapture();
        SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
        break;

    case WM_LBUTTONUP: {
        int y = HIWORD(lParam);

        if (y < SLOT_HEIGHT) {
            launch("cmd.exe");
        }
        else if (y < SLOT_HEIGHT * 2) {
            launch("notepad.exe");
        }
        else if (y < SLOT_HEIGHT * 3) {
            launch("explorer.exe");
        }
        break;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rect;
        GetClientRect(hwnd, &rect);

        HBRUSH bg = CreateSolidBrush(RGB(25, 25, 25));
        FillRect(hdc, &rect, bg);
        DeleteObject(bg);

        SetBkMode(hdc, TRANSPARENT);

        DrawIconEx(hdc, 20, 10, icon_terminal, 48, 48, 0, NULL, DI_NORMAL);
        TextOut(hdc, 20, 60, "Terminal", 8);

        DrawIconEx(hdc, 20, 90, icon_notepad, 48, 48, 0, NULL, DI_NORMAL);
        TextOut(hdc, 20, 140, "Notepad", 7);

        DrawIconEx(hdc, 20, 170, icon_explorer, 48, 48, 0, NULL, DI_NORMAL);
        TextOut(hdc, 20, 220, "Explorer", 8);

        EndPaint(hwnd, &ps);
        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}

// --- create sidebar ---
void CreateSidebar(HINSTANCE hInstance) {
    const char CLASS_NAME[] = "LibreNTSidebar";

    WNDCLASS wc = {0};
    wc.lpfnWndProc = SidebarProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    int screenH = GetSystemMetrics(SM_CYSCREEN);

    CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        CLASS_NAME,
        "LibreNT Sidebar",
        WS_POPUP | WS_VISIBLE,
        0, (screenH - 300) / 2,
        SIDEBAR_WIDTH, 300,
        NULL, NULL, hInstance, NULL
    );
}