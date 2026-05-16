/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     Windows installer or deployment component
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



#ifndef VERBOSE_BOOT_H
#define VERBOSE_BOOT_H

#include <string>
#include <vector>

class VerboseBoot {
public:
    static void Show(const std::vector<std::string>& messages, bool* p_open);
    static void AddMessage(std::vector<std::string>& messages, const std::string& msg);
    
private:
    static ImVec4 GetMessageColor(const std::string& message);
};

#endif // VERBOSE_BOOT_H