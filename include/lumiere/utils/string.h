#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <utility>

namespace LuM {
    namespace Utils {
        namespace String {
            std::vector<std::string> Explode(const char* input, char delimiter) {
                std::vector<std::string> parts;
                std::istringstream iis(input);
                for (std::string token; std::getline(iis, token, delimiter);) {
                    parts.insert(parts.begin(), std::move(token));
                }
                return parts;
            }
        }
    }
}