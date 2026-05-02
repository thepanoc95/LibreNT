#include <windows.h>
#include <shellapi.h>
#include <stdio.h>

// #include "dock.h"

#define ID_BTN_TERMINAL 1
#define ID_BTN_EXPLORER 2
#define ID_BTN_RUN      3

char buildNumber[64] = "Unknown";

void GetBuildNumber() {
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
        0, KEY_READ, &hKey) == ERROR_SUCCESS) {

        DWORD size = sizeof(buildNumber);
        RegQueryValueEx(hKey, "CurrentBuildNumber", NULL, NULL,
            (LPBYTE)buildNumber, &size);

        RegCloseKey(hKey);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {

    case WM_CREATE:
        GetBuildNumber();

        CreateWindow("BUTTON", "Open cmd",
            WS_VISIBLE | WS_CHILD,
            20, 100, 150, 30,
            hwnd, (HMENU)ID_BTN_TERMINAL, NULL, NULL);

        CreateWindow("BUTTON", "Run Notepad",
            WS_VISIBLE | WS_CHILD,
            20, 180, 150, 30,
            hwnd, (HMENU)ID_BTN_RUN, NULL, NULL);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_BTN_TERMINAL:
            ShellExecute(NULL, "open", "cmd.exe", NULL, NULL, SW_SHOWNORMAL);
            break;

        case ID_BTN_RUN:
            ShellExecute(NULL, "open", "notepad.exe", NULL, NULL, SW_SHOWNORMAL);
            break;
        }
        break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        char line1[] = "LibreNT, free your WindowsNT";
        char line2[128];
        char line3[] = "LibreNT 0.1.000 testing";

        sprintf(line2, "Based on build %s", buildNumber);

        TextOut(hdc, 20, 20, line1, lstrlen(line1));
        TextOut(hdc, 20, 40, line2, lstrlen(line2));
        TextOut(hdc, 20, 60, line3, lstrlen(line3));

        EndPaint(hwnd, &ps);
        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    const char CLASS_NAME[] = "LibreNTFallback";

    SetProcessDPIAware();

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    int winWidth = 400;
    int winHeight = 280;

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    int posX = (screenWidth - winWidth) / 2;
    int posY = (screenHeight - winHeight) / 2;

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "Welcome to LibreNT",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        posX, posY, winWidth, winHeight,
        NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);

  //  CreateSidebar(hInstance);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}