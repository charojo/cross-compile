#include "Cache.h"

std::string Cache::Get(int userId, const std::string& field) {
    auto userIt = data_.find(userId);
    if (userIt == data_.end()) {
        return "";
    }
    auto fieldIt = userIt->second.find(field);
    if (fieldIt == userIt->second.end()) {
        return "";
    }
    return fieldIt->second;
}

void Cache::Set(int userId, const std::string& field, const std::string& value) {
    data_[userId][field] = value;
}
