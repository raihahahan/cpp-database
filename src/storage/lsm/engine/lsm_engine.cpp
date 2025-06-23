#include "lsm_engine.hpp"
#include <iostream>

LSMEngine::LSMEngine(std::optional<std::filesystem::path> walPath) : wal(std::move(walPath)) {
    std::cout << "LSMEngine created\n";
}

LSMEngine::~LSMEngine() {
    std::cout << "LSMEngine destroyed\n";
}

void LSMEngine::put(const std::string& key, const std::string& value) {
    wal.append(WalRecord{OpType::CREATE, key, value});
    memTable.put(key, value);
    std::cout << "Put: " << key << " -> " << value << "\n";
}

std::optional<std::string> LSMEngine::get(const std::string& key) {
    std::cout << "Get: " << key << "\n";
    return memTable.get(key);
}

void LSMEngine::remove(const std::string& key) {
    wal.append(WalRecord{OpType::DELETE, key, ""});
    memTable.remove(key);
    std::cout << "Remove: " << key << "\n";
}
