#include "wal.hpp"
#include <stdexcept>
#include <cstring>
#include <filesystem>
#include <fstream>
#include "../../../config.hpp"

namespace fs = std::filesystem;

WAL::WAL(std::optional<fs::path> pathOverride) : filepath(pathOverride.value_or(WAL_PATH)) {
    // ensure path exists
    fs::create_directories(filepath.parent_path());
    
    // open the file in append mode
    fp = std::fopen(filepath.string().c_str(), "ab");
    if (!fp) {
        throw std::runtime_error("Failed to open WAL file: " + filepath.string());
    }

};

WAL::~WAL() {
    if (fp) {
        std::fclose(fp);
        fp = nullptr;
    }
};

WAL::WAL(WAL&& other) noexcept : filepath(WAL_PATH), fp(other.fp) {
    other.fp = nullptr;
};

WAL& WAL::operator=(WAL&& other) noexcept {
    if (this != &other) {
        if (fp) std::fclose(fp);
        filepath = std::move(other.filepath);
        fp = other.fp;
        other.fp = nullptr;
    }

    return *this;
};

void WAL::append(WalRecord&& record) {
    auto data = record.serialize();
    std::fwrite(data.data(), 1, data.size(), fp);
    std::fflush(fp);
}


// serialize the record into a binary format
std::vector<uint8_t> WalRecord::serialize() const {
    /**
     * [1B  opType]
     * [4B  key size]
     * [key bytes]
     * [4B  value size]
     * [value bytes]
    */
    std::vector<uint8_t> buffer;

    uint8_t op = static_cast<uint8_t>(opType);
    uint32_t keySize = static_cast<uint32_t>(key.size());
    uint32_t valueSize = static_cast<uint32_t>(value.size());

    buffer.push_back(op);

    auto append_bytes = [&](auto val) {
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&val);
        buffer.insert(buffer.end(), bytes, bytes + sizeof(val));
    };

    append_bytes(keySize);
    buffer.insert(buffer.end(), key.begin(), key.end());
    append_bytes(valueSize);
    buffer.insert(buffer.end(), value.begin(), value.end());

    return buffer;
}

// read a record from a WAL file
WalRecord WalRecord::deserialize(FILE* fp) {
    WalRecord record;
    uint8_t op;

    if (std::fread(&op, sizeof(op), 1, fp) != 1) {
        throw std::runtime_error("Failed to read opType from WAL.");
    }
    record.opType = static_cast<OpType>(op);

    uint32_t keySize;
    if (std::fread(&keySize, sizeof(keySize), 1, fp) != 1) {
        throw std::runtime_error("Failed to read keySize from WAL.");
    }

    record.key.resize(keySize);
    if (std::fread(record.key.data(), 1, keySize, fp) != keySize) {
        throw std::runtime_error("Failed to read key from WAL.");
    }

    uint32_t valueSize;
    if (std::fread(&valueSize, sizeof(valueSize), 1, fp) != 1) {
        throw std::runtime_error("Failed to read valueSize from WAL.");
    }

    record.value.resize(valueSize);
    if (std::fread(record.value.data(), 1, valueSize, fp) != valueSize) {
        throw std::runtime_error("Failed to read value from WAL.");
    }

    return record;
}
