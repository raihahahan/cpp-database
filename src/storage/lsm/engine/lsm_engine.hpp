#pragma once
#include "../../engine.hpp"
#include "../wal/wal.hpp"
#include "../memtable/memtable.hpp"

class LSMEngine : public StorageEngine {
public:
    LSMEngine(std::optional<std::filesystem::path> walPath = std::nullopt);
    ~LSMEngine();

    void put(const std::string& key, const std::string& value) override;
    std::optional<std::string> get(const std::string& key) override;
    void remove(const std::string& key) override;

private:
    WAL wal;
    Memtable memTable;
};