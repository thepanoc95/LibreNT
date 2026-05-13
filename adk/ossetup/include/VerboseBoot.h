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