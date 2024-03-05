#pragma once
#ifndef JOIN_HPP
#define JOIN_HPP

#include <vector>
#include <sstream>

std::string join(const std::vector<std::string>& vec, const std::string& delimiter) {
    std::stringstream ss;
    for (size_t i = 0; i < vec.size(); i++) {
        ss << vec[i];
        if (i != vec.size() - 1) {
            ss << delimiter;
        }
    }
    return ss.str();
}

#endif
