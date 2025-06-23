#include "memtable.hpp"

void Memtable::put(const std::string& key, const std::string& value) {
    kv.insert(key, value);
}

void Memtable::remove(const std::string& key) {
    kv.remove(key);
}

std::optional<std::string> Memtable::get(const std::string& key) const {
    return kv.get(key);
}

std::vector<std::pair<std::string, std::string>> Memtable::sortedEntries() const {
    return kv.entries();
}