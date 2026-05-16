#include "VerboseBoot.h"
#include "imgui.h"
#include <algorithm>

void VerboseBoot::Show(const std::vector<std::string>& messages, bool* p_open) {
    ImGuiIO& io = ImGui::GetIO();
    
    // Fullscreen window
    ImGui::SetNextWindowBgAlpha(1.0f);
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | 
                             ImGuiWindowFlags_NoResize | 
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_NoBringToFrontOnFocus;
    
    ImGui::Begin("VerboseBoot", p_open, flags);
    
    // Black background
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRectFilled(ImGui::GetWindowPos(), 
                              ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowWidth(),
                                     ImGui::GetWindowPos().y + ImGui::GetWindowHeight()),
                              IM_COL32(0, 0, 0, 255));
    
    // Calculate available height
    float lineHeight = ImGui::GetFontSize() * 1.2f;
    float startY = 20.0f;
    float maxHeight = io.DisplaySize.y - startY - 20.0f;
    
    // Show only recent messages that fit on screen
    size_t startIdx = 0;
    if (messages.size() * lineHeight > maxHeight) {
        startIdx = messages.size() - (size_t)(maxHeight / lineHeight);
    }
    
    // Draw messages
    float y = startY;
    for (size_t i = startIdx; i < messages.size(); ++i) {
        const std::string& msg = messages[i];
        ImVec4 color = GetMessageColor(msg);
        
        draw_list->AddText(ImVec2(20, y), 
            IM_COL32((int)(color.x * 255), (int)(color.y * 255), (int)(color.z * 255), 255),
            msg.c_str());
        
        y += lineHeight;
    }
    
    ImGui::End();
}

ImVec4 VerboseBoot::GetMessageColor(const std::string& message) {
    // Different colors for different message types (macOS verbose boot style)
    if (message.find("ERROR") != std::string::npos || 
        message.find("error") != std::string::npos) {
        return ImVec4(1.0f, 0.4f, 0.4f, 1.0f);  // Red for errors
    }
    if (message.find("WARNING") != std::string::npos || 
        message.find("warning") != std::string::npos) {
        return ImVec4(1.0f, 0.8f, 0.4f, 1.0f);  // Yellow for warnings
    }
    if (message.find("SUCCESS") != std::string::npos || 
        message.find("success") != std::string::npos) {
        return ImVec4(0.4f, 1.0f, 0.4f, 1.0f);  // Green for success
    }
    return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);  // White for normal messages
}

void VerboseBoot::AddMessage(std::vector<std::string>& messages, const std::string& msg) {
    messages.push_back(msg);
}