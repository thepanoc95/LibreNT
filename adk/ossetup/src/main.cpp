/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     Windows installer or deployment component
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



#include "ImGuiImpl.h"
#include "InstallerUI.h"
#include "DeploymentEngine.h"
#include "RegistryPatcher.h"
#include "DriverInjector.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <windows.h>
#include <tchar.h>
#include <thread>
#include <chrono>

// Global ImGuiImpl instance
static ImGuiImpl* g_Impl = nullptr;

// Forward declarations
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    g_Impl = new ImGuiImpl();
    
    // Create application window
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL),
                     NULL, NULL, NULL, NULL, _T("LibreNT Installer"), NULL };
    RegisterClassEx(&wc);
    
    HWND hwnd = CreateWindow(wc.lpszClassName, _T("LibreNT Installer"), WS_OVERLAPPEDWINDOW,
                             100, 100, 800, 600, NULL, NULL, wc.hInstance, NULL);
    
    // Initialize DirectX
    if (!g_Impl->CreateDeviceD3D(hwnd)) {
        g_Impl->CleanupDeviceD3D();
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        delete g_Impl;
        return 1;
    }
    
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    
    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_Impl->GetDevice(), g_Impl->GetDeviceContext());
    
    // Custom style for LibreNT
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.12f, 1.0f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.2f, 0.4f, 0.8f, 1.0f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.3f, 0.5f, 0.9f, 1.0f);
    colors[ImGuiCol_Button] = ImVec4(0.25f, 0.55f, 0.9f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.35f, 0.65f, 1.0f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.15f, 0.45f, 0.8f, 1.0f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.2f, 0.22f, 1.0f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.3f, 0.3f, 0.32f, 1.0f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.4f, 0.4f, 0.42f, 1.0f);
    
    // Initialize modules
    InstallerUI installerUI;
    DeploymentEngine deploymentEngine;
    RegistryPatcher registryPatcher;
    DriverInjector driverInjector;
    driverInjector.Initialize("drivers");
    
    bool done = false;
    bool showInstaller = true;
    bool showVerboseBoot = false;
    
    // Main loop
    while (!done) {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        
        if (done)
            break;
        
        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        
        // Show installer or verbose boot window
        if (showInstaller) {
            installerUI.ShowInstallerWindow(&showInstaller);
            
            // Handle installation start
            if (installerUI.IsInstalling()) {
                // In a real app, this would run in a background thread
                static int step = 0;
                step++;
                
                // Simulate installation steps
                std::vector<std::string> messages = {
                    "[OK] Starting installation...",
                    "[OK] Mounting Windows image...",
                    "[OK] Installing BTRFS drivers...",
                    "[OK] Installing Quibble bootloader...",
                    "[OK] Applying registry patches...",
                    "[OK] Committing changes...",
                    "[OK] Installation complete!"
                };
                
                static size_t msgIdx = 0;
                if (msgIdx < messages.size()) {
                    installerUI.AddVerboseMessage(messages[msgIdx]);
                    msgIdx++;
                }
            }
        }
        
        if (showVerboseBoot) {
            installerUI.ShowVerboseBootWindow(&showVerboseBoot);
        }
        
        // Rendering
        ImGui::Render();
        const float clear_color[4] = { 0.1f, 0.1f, 0.12f, 1.00f };
        g_Impl->GetDeviceContext()->OMSetRenderTargets(1, &g_Impl->GetRenderTarget(), NULL);
        g_Impl->GetDeviceContext()->ClearRenderTargetView(g_Impl->GetRenderTarget(), clear_color);
        g_Impl->GetDeviceContext()->RSSetViewports(1, &(D3D11_VIEWPORT){0, 0, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y, 0.0f, 1.0f});
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        
        g_Impl->GetSwapChain()->Present(1, 0);
    }
    
    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    
    g_Impl->CleanupDeviceD3D();
    DestroyWindow(hwnd);
    UnregisterClass(wc.lpszClassName, wc.hInstance);
    delete g_Impl;
    
    return 0;
}

// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (g_Impl->WndProc(hWnd, msg, wParam, lParam))
        return true;
    
    switch (msg) {
    case WM_SIZE:
        if (g_Impl->GetDevice() != nullptr && wParam != SIZE_MINIMIZED) {
            g_Impl->CleanupRenderTarget();
            g_Impl->GetSwapChain()->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            g_Impl->CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}