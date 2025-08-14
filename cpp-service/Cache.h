#pragma once

#include <string>
#include <unordered_map>

class Cache {
public:
    std::string Get(int userId, const std::string& field);
    void Set(int userId, const std::string& field, const std::string& value);
private:
    std::unordered_map<int, std::unordered_map<std::string, std::string>> data_;
};
