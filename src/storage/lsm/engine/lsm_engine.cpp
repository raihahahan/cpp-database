#include "lsm_engine.hpp"
#include "../../../config.hpp"
#include "../../../common/utils/file_utils.hpp"
#include <iostream>
#include <string>
#include <map>

LSMEngine::LSMEngine(std::optional<std::filesystem::path> walPath,
                        const size_t threshold,
                        const int compactionInterval,
                        std::string sstableDir) : 
                    wal(std::move(walPath)), 
                    FLUSH_THRESHOLD(threshold),
                    COMPACTION_INTERVAL_MS(compactionInterval) {
    segmentManager.loadSegments(sstableDir);
    wal.replay([this](const WalRecord& rec) {
        switch (rec.opType)
        {
        case OpType::CREATE:
            if (!memTable.get(rec.key).has_value()) {
                memTable.put(rec.key, rec.value);
                std::cout << "[WAL Replay]: Insert " << rec.key << ": " << rec.value << std::endl;
            }
            break;
        
        case OpType::UPDATE:
            if (memTable.get(rec.key).has_value()) {
                memTable.put(rec.key, rec.value);
                std::cout << "[WAL Replay]: Update " << rec.key << ": " << rec.value << std::endl;
            }
            break;
        
        case OpType::DELETE:
            memTable.remove(rec.key);
            std::cout << "[WAL Replay]: Delete " << rec.key << std::endl;
            break;

        default:
            break;
        }
    });
    startCompactionThread();

    std::cout << "LSMEngine created\n";
}

LSMEngine::~LSMEngine() {
    stopCompaction.store(true);
    if (compactionThread.joinable()) compactionThread.join();
    std::cout << "LSMEngine destroyed\n";
}

void LSMEngine::put(const std::string& key, const std::string& value) {
    std::cout << "Put: " << key << " -> " << value << "\n";
    wal.append(WalRecord{OpType::CREATE, key, value});
    memTable.put(key, value);
    ++entryCount;
    maybeFlush();
}

std::optional<std::string> LSMEngine::get(const std::string& key) {
    std::cout << "Get: " << key << "\n";
    if (auto val = memTable.get(key)) {
        if (val.has_value() && isTombstone(*val)) {
            return std::nullopt;
        }
        return val;
    }
    if (auto val = segmentManager.get(key)) {
        if (val.has_value() && isTombstone(*val)) {
            return std::nullopt;
        }
        return val;
    }
    return std::nullopt;
}

std::vector<std::pair<std::string, std::string>> LSMEngine::getRange(int limit) {
    auto mem = memTable.getRange(limit);
    auto seg = segmentManager.getRange(limit);

    // merge: latest entry wins (memtable should override segment)
    std::map<std::string, std::string> merged;
    for (const auto& [k, v] : seg) merged[k] = v;
    for (const auto& [k, v] : mem) merged[k] = v;

    std::vector<std::pair<std::string, std::string>> result;
    for (const auto& [k, v] : merged) {
        if (isTombstone(v)) continue;
        if (limit != -1 && result.size() >= static_cast<size_t>(limit)) break;
        result.emplace_back(k, v);
    }

    return result;
}

void LSMEngine::remove(const std::string& key) {
    std::cout << "Remove: " << key << "\n";
    wal.append(WalRecord{OpType::DELETE, key, ""});
    memTable.remove(key);
    ++entryCount;
    maybeFlush();
}

void LSMEngine::maybeFlush() {
    auto data = memTable.getRange();
    if (data.empty()) return;
    if (entryCount >= FLUSH_THRESHOLD) {
        segmentManager.flush(data);
        entryCount = 0;
        wal.clear();
        memTable.clear();
    }
}

void LSMEngine::startCompactionThread() {
    compactionThread = std::thread([this]() {
        while (!stopCompaction.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(COMPACTION_INTERVAL_MS));
            segmentManager.compact();
        }
    });
}
