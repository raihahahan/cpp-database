#include "memtable.hpp"
#include "../../../config.hpp"
#include "../../../common/utils/file_utils.hpp"

void Memtable::put(const std::string& key, const std::string& value) {
    kv.insert(key, value);
}

void Memtable::remove(const std::string& key) {
    kv.insert(key, TOMBSTONE_MARKER);
}

std::optional<std::string> Memtable::get(const std::string& key) const {
    return kv.get(key);
}

std::vector<std::pair<std::string, std::string>> Memtable::getRange(int limit) const {
    std::vector<std::pair<std::string, std::string>> result;
    for (const auto& [key, loc] : kv.entries()) {
        if (limit != -1 && result.size() >= static_cast<size_t>(limit)) break;

        if (auto val = get(key)) {
            result.emplace_back(key, *val);
        }
    }
    return result;
}

void Memtable::clear() {
    kv.clear();
}