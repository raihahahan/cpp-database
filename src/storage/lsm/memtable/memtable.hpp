#pragma once
#include <string>
#include <optional>
#include "../../../common/containers/skiplist.hpp"

class Memtable {
public:
    void put(const std::string& key, const std::string& value);
    void remove(const std::string& key);
    std::optional<std::string> get(const std::string& key) const;
    std::vector<std::pair<std::string, std::string>> getRange(int limit = -1) const;

private:
    SkipList<std::string, std::string> kv;
};