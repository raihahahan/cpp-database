#include "database.hpp"

Database::Database(std::unique_ptr<StorageEngine> engine)
    : engine_(std::move(engine)) {}

void Database::put(const std::string& key, const std::string& value) {
    engine_->put(key, value);
}

std::optional<std::string> Database::get(const std::string& key) {
    return engine_->get(key);
}

void Database::remove(const std::string& key) {
    engine_->remove(key);
}

std::vector<std::pair<std::string, std::string>> Database::getRange(int limit) {
    return engine_->getRange(limit);
}
