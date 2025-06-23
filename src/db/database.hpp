#pragma once
#include "../storage/engine.hpp"
#include <utility>
#include <string>
#include <vector>

class Database {
public:
    Database(std::unique_ptr<StorageEngine> engine);

    void put(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key);
    std::vector<std::pair<std::string, std::string>> getRange(int limit = -1);
    void remove(const std::string& key);

private:
    std::unique_ptr<StorageEngine> engine_;
};