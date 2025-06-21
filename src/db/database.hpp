#pragma once
#include "../storage/engine.hpp"

class Database {
public:
    Database(std::unique_ptr<StorageEngine> engine);

    void put(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key);
    void remove(const std::string& key);

private:
    std::unique_ptr<StorageEngine> engine_;
};