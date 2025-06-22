#pragma once
#include <cstdio>
#include <string>
#include <filesystem>

enum OpType {
    CREATE,
    READ,
    UPDATE,
    DELETE
};

struct WalRecord {
    OpType opType;
    std::string key;
    std::string value;
    std::vector<uint8_t> serialize() const;
    static WalRecord deserialize(FILE* fp);
};

class WAL {
public:
    explicit WAL();
    ~WAL();

    WAL(const WAL&)= delete;
    WAL& operator=(const WAL&) = delete;

    WAL(WAL&& other) noexcept;
    WAL& operator=(WAL&& other) noexcept;

    void append(const WalRecord& record);

private:
    std::filesystem::path filepath;
    FILE* fp;
};