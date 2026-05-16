/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     Windows native helper component
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



/*
 * welcome_nt.c - Windows NT "Welcome" dialog clone
 * Compile: cl welcome_nt.c /link user32.lib gdi32.lib
 *   or: gcc welcome_nt.c -o welcome_nt.exe -lgdi32 -luser32 -lcomdlg32 -mwindows
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>

/* Control IDs */
#define IDC_WHATS_NEW     101
#define IDC_HELP_CONTENTS 102
#define IDC_NEXT_TIP      103
#define IDC_CLOSE         104
#define IDC_SHOW_NEXT     105
#define IDC_TIP_PANEL     106

/* Dimensions (matching the original ~480x200 dialog) */
#define DLG_W  480
#define DLG_H  220

/* Tips pool */
static const TCHAR *g_tips[] = {
    _T("To add a program to your Start menu, you can\ndrag the program's icon to the Start button."),
    _T("To quickly minimize all open windows, right-click\nthe taskbar and choose Minimize All Windows."),
    _T("You can resize the taskbar by dragging its edge\nup or down."),
    _T("To move the taskbar, drag an empty area of it\nto any edge of the screen."),
    _T("You can create shortcuts on the Desktop by\ndragging icons from Explorer while holding Alt."),
    _T("Press Ctrl+Esc or click the Start button to open\nthe Start menu."),
    _T("To find a file quickly, click Start, choose Find,\nthen Files or Folders."),
    _T("You can change your desktop background by right-\nclicking the desktop and choosing Properties."),
};
#define TIP_COUNT (sizeof(g_tips) / sizeof(g_tips[0]))

static int    g_tipIndex    = 0;
static BOOL   g_showOnStart = TRUE;

/* ── helpers ───────────────────────────────────────────────────── */

/* Draw the classic Win3x/NT "checkerboard / hatched" banner fill */
static void DrawCheckerBanner(HDC hdc, RECT *rc)
{
    /* yellow background */
    HBRUSH hYellow = CreateSolidBrush(RGB(255, 255, 128));
    FillRect(hdc, rc, hYellow);
    DeleteObject(hYellow);

    /* white squares on top to produce the dithered look */
    HBRUSH hWhite  = CreateSolidBrush(RGB(255, 255, 255));
    int sq = 8;
    for (int y = rc->top; y < rc->bottom; y += sq) {
        for (int x = rc->left; x < rc->right; x += sq) {
            /* checkerboard offset: paint white on even cells */
            if (((x / sq) + (y / sq)) % 2 == 0) {
                RECT cell = { x, y, x + sq, y + sq };
                FillRect(hdc, &cell, hWhite);
            }
        }
    }
    DeleteObject(hWhite);
}

/* Draw the little "info" smiley-lightbulb icon (approximation) */
static void DrawInfoIcon(HDC hdc, int cx, int cy)
{
    /* yellow circle */
    HBRUSH hYellow = CreateSolidBrush(RGB(255, 220, 0));
    HPEN   hBlack  = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
    SelectObject(hdc, hYellow);
    SelectObject(hdc, hBlack);
    Ellipse(hdc, cx - 14, cy - 14, cx + 14, cy + 14);

    /* exclamation mark body */
    HBRUSH hBlackB = CreateSolidBrush(RGB(0, 0, 0));
    SelectObject(hdc, hBlackB);
    SelectObject(hdc, GetStockObject(NULL_PEN));
    Rectangle(hdc, cx - 3, cy - 8, cx + 3, cy + 3);

    /* exclamation dot */
    Ellipse(hdc, cx - 3, cy + 5, cx + 3, cy + 11);

    DeleteObject(hYellow);
    DeleteObject(hBlack);
    DeleteObject(hBlackB);
}

/* ── window procedure ──────────────────────────────────────────── */

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {

    case WM_CREATE: {
        HFONT hFontNormal = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

        /* ---------- Title label: "Welcome to Windows NT" ---------- */
        /* We'll paint this manually in WM_PAINT for font control.    */

        /* ---------- Right-side buttons ---------- */
        struct { TCHAR *label; int id; int y; } btns[] = {
            { _T("What's &New"),     IDC_WHATS_NEW,     60  },
            { _T("&Help Contents"),  IDC_HELP_CONTENTS, 92  },
            { _T("&Next Tip"),       IDC_NEXT_TIP,      124 },
        };
        for (int i = 0; i < 3; i++) {
            HWND hb = CreateWindow(_T("BUTTON"), btns[i].label,
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                380, btns[i].y, 88, 24,
                hwnd, (HMENU)(UINT_PTR)btns[i].id,
                ((LPCREATESTRUCT)lp)->hInstance, NULL);
            SendMessage(hb, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
        }

        /* ---------- Close button ---------- */
        HWND hClose = CreateWindow(_T("BUTTON"), _T("&Close"),
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            380, 183, 88, 24,
            hwnd, (HMENU)IDC_CLOSE,
            ((LPCREATESTRUCT)lp)->hInstance, NULL);
        SendMessage(hClose, WM_SETFONT, (WPARAM)hFontNormal, TRUE);

        /* ---------- Checkbox ---------- */
        HWND hChk = CreateWindow(_T("BUTTON"),
            _T("Show this Welcome Screen next time you start Windows NT"),
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            10, 188, 355, 18,
            hwnd, (HMENU)IDC_SHOW_NEXT,
            ((LPCREATESTRUCT)lp)->hInstance, NULL);
        SendMessage(hChk, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
        SendMessage(hChk, BM_SETCHECK, BST_CHECKED, 0);

        return 0;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT client;
        GetClientRect(hwnd, &client);

        /* ---- white background ---- */
        FillRect(hdc, &client, (HBRUSH)GetStockObject(WHITE_BRUSH));

        /* ---- title bar area (top ~48 px) ---- */
        RECT titleRect = { 0, 0, client.right, 48 };
        FillRect(hdc, &titleRect, (HBRUSH)GetStockObject(WHITE_BRUSH));

        /* Title text */
        SetBkMode(hdc, TRANSPARENT);

        /* "Welcome to " in normal weight */
        HFONT hBig   = CreateFont(28, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                          ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                          DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
        HFONT hBigB  = CreateFont(28, 0, 0, 0, FW_BOLD,   FALSE, FALSE, FALSE,
                          ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                          DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
        HFONT hBigI  = CreateFont(28, 0, 0, 0, FW_NORMAL, TRUE,  FALSE, FALSE,
                          ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                          DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));

        SetTextColor(hdc, RGB(0,0,0));

        /* measure "Welcome to " */
        SelectObject(hdc, hBig);
        SIZE sz1; GetTextExtentPoint32(hdc, _T("Welcome to "), 11, &sz1);

        /* measure "Windows" bold */
        SelectObject(hdc, hBigI);
        SIZE sz2; GetTextExtentPoint32(hdc, _T("Libre"), 7, &sz2);

        /* measure " NT" italic */
        SelectObject(hdc, hBigB);
        SIZE sz3; GetTextExtentPoint32(hdc, _T("NT"), 3, &sz3);

        int tx = 10, ty = 10;

        SelectObject(hdc, hBig);
        TextOut(hdc, tx, ty, _T("Welcome to "), 11);
        tx += sz1.cx;

        SelectObject(hdc, hBigI);
        TextOut(hdc, tx, ty, _T("Libre"), 7);
        tx += sz2.cx;

        SelectObject(hdc, hBigB);
        TextOut(hdc, tx, ty, _T("NT"), 3);

        DeleteObject(hBig);
        DeleteObject(hBigB);
        DeleteObject(hBigI);

        /* ---- separator line under title ---- */
        HPEN hGray = CreatePen(PS_SOLID, 1, RGB(128,128,128));
        SelectObject(hdc, hGray);
        MoveToEx(hdc, 0, 48, NULL);
        LineTo(hdc, client.right - 100, 48);
        DeleteObject(hGray);

        /* ---- checkerboard tip panel ---- */
        RECT checkRect = { 0, 49, client.right - 100, 178 };
        DrawCheckerBanner(hdc, &checkRect);

        /* ---- "Did you know..." header ---- */
        HFONT hBold = CreateFont(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                          ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                          DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
        HFONT hNorm = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                          ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                          DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));

        DrawInfoIcon(hdc, 26, 78);

        SelectObject(hdc, hBold);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(0,0,0));
        TextOut(hdc, 48, 62, _T("Did you know..."), 15);

        /* ---- tip text ---- */
        SelectObject(hdc, hNorm);
        RECT tipRect = { 48, 84, client.right - 108, 175 };
        DrawText(hdc, g_tips[g_tipIndex], -1, &tipRect,
                 DT_LEFT | DT_WORDBREAK | DT_NOPREFIX);

        DeleteObject(hBold);
        DeleteObject(hNorm);

        /* ---- separator above checkbox ---- */
        HPEN hGray2 = CreatePen(PS_SOLID, 1, RGB(128,128,128));
        SelectObject(hdc, hGray2);
        MoveToEx(hdc, 0, 180, NULL);
        LineTo(hdc, client.right, 180);
        DeleteObject(hGray2);

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case IDC_NEXT_TIP:
            g_tipIndex = (g_tipIndex + 1) % TIP_COUNT;
            /* Repaint the tip panel area */
            {
                RECT r = { 0, 49, DLG_W - 100, 178 };
                InvalidateRect(hwnd, &r, TRUE);
            }
            break;

        case IDC_WHATS_NEW:
            MessageBox(hwnd,
                _T("This feature is not implemented in this demo."),
                _T("What's New"), MB_OK | MB_ICONINFORMATION);
            break;

        case IDC_HELP_CONTENTS:
            MessageBox(hwnd,
                _T("Help is not available in this demo."),
                _T("Help"), MB_OK | MB_ICONINFORMATION);
            break;

        case IDC_CLOSE:
        case IDCANCEL:
            g_showOnStart = (SendDlgItemMessage(hwnd, IDC_SHOW_NEXT,
                                 BM_GETCHECK, 0, 0) == BST_CHECKED);
            DestroyWindow(hwnd);
            break;
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

/* ── WinMain ───────────────────────────────────────────────────── */

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev,
                   LPSTR lpCmd, int nShow)
{
    (void)hPrev; (void)lpCmd;

    WNDCLASSEX wc = {0};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = _T("WelcomeNT");
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    RegisterClassEx(&wc);

    /* Center on screen */
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);
    int wx = (sw - DLG_W) / 2;
    int wy = (sh - DLG_H) / 2;

    HWND hwnd = CreateWindowEx(
        WS_EX_DLGMODALFRAME,
        _T("WelcomeNT"),
        _T("Welcome"),
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        wx, wy, DLG_W, DLG_H,
        NULL, NULL, hInst, NULL);

    ShowWindow(hwnd, nShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}