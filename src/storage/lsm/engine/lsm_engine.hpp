#pragma once
#include "../../engine.hpp"
#include "../wal/wal.hpp"
#include "../memtable/memtable.hpp"
#include "../sstable/segment_manager.hpp"
#include "../../../config.hpp"

class LSMEngine : public StorageEngine {
public:
    LSMEngine(std::optional<std::filesystem::path> walPath = std::nullopt, const size_t threshold = LSM_FLUSH_THRESHOLD);
    ~LSMEngine();

    void put(const std::string& key, const std::string& value) override;
    std::optional<std::string> get(const std::string& key) override;
    std::vector<std::pair<std::string, std::string>> getRange(int limit = -1) override;
    void remove(const std::string& key) override;

private:
    size_t FLUSH_THRESHOLD;
    size_t entryCount = 0;

    WAL wal;
    Memtable memTable;
    SegmentManager segmentManager;

    void maybeFlush();
};