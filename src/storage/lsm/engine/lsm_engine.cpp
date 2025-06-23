#include "lsm_engine.hpp"
#include <iostream>

LSMEngine::LSMEngine(std::optional<std::filesystem::path> walPath) : wal(std::move(walPath)) {
    wal.replay([this](const WalRecord& rec) {
        switch (rec.opType)
        {
        case OpType::CREATE:
            if (!memTable.get(rec.key).has_value()) {
                memTable.put(rec.key, rec.value);
                std::cout << "[WAL Replay]: Insert " << rec.key << ", " << rec.value << std::endl;
            }
            break;
        
        case OpType::UPDATE:
            if (memTable.get(rec.key).has_value()) {
                memTable.put(rec.key, rec.value);
                std::cout << "[WAL Replay]: Update " << rec.key << ", " << rec.value << std::endl;
            }
            break;
        
        case OpType::DELETE:
            memTable.remove(rec.key);
            std::cout << "[WAL Replay]: Delete " << rec.key << ", " << rec.value << std::endl;
            break;

        default:
            break;
        }
    });

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

std::vector<std::pair<std::string, std::string>> LSMEngine::getRange(int limit) {
    return memTable.getRange(limit);
}

void LSMEngine::remove(const std::string& key) {
    wal.append(WalRecord{OpType::DELETE, key, ""});
    memTable.remove(key);
    std::cout << "Remove: " << key << "\n";
}
