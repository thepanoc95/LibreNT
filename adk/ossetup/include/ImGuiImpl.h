/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     Windows installer or deployment component
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



#ifndef IMGUI_IMPL_H
#define IMGUI_IMPL_H

#include <windows.h>
#include <d3d11.h>
#include <tchar.h>
#include <string>
#include <vector>

class ImGuiImpl {
public:
    ImGuiImpl();
    ~ImGuiImpl();

    bool CreateDeviceD3D(HWND hWnd);
    void CleanupDeviceD3D();
    void CreateRenderTarget();
    void CleanupRenderTarget();
    LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    ID3D11Device* GetDevice() const { return g_pd3dDevice; }
    ID3D11DeviceContext* GetDeviceContext() const { return g_pd3dDeviceContext; }
    IDXGISwapChain* GetSwapChain() const { return g_pSwapChain; }
    ID3D11RenderTargetView* GetRenderTarget() const { return g_mainRenderTargetView; }

private:
    ID3D11Device* g_pd3dDevice = nullptr;
    ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
    IDXGISwapChain* g_pSwapChain = nullptr;
    ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

    // Win32 state
    HWND g_hWnd = nullptr;
    bool g_SwapChainOccluded = false;
};

// Global state for ImGui
extern ImGuiImpl* g_ImGuiImpl;

#endif // IMGUI_IMPL_H