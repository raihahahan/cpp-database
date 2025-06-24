#pragma once
#include "../../engine.hpp"
#include "../../wal/wal.hpp"
#include "../memtable/memtable.hpp"
#include "../sstable/segment_manager.hpp"
#include "../../../config.hpp"
#include <thread>
#include <atomic>
#include <chrono>

class LSMEngine : public StorageEngine {
public:
    LSMEngine(std::optional<std::filesystem::path> walPath = std::nullopt, 
                const size_t threshold = LSM_FLUSH_THRESHOLD,
                const int compactionInterval = LSM_COMPACTION_INTERVAL_MS,
                const std::string ssTableDir = SSTABLE_DIR);
    ~LSMEngine();

    void put(const std::string& key, const std::string& value) override;
    std::optional<std::string> get(const std::string& key) override;
    std::vector<std::pair<std::string, std::string>> getRange(int limit = -1) override;
    void remove(const std::string& key) override;
    void startCompactionThread();

private:
    size_t FLUSH_THRESHOLD;
    size_t entryCount = 0;
    int COMPACTION_INTERVAL_MS;

    WAL wal;
    Memtable memTable;
    SegmentManager segmentManager;
    std::thread compactionThread;
    std::atomic<bool> stopCompaction{false};


    void maybeFlush();
};