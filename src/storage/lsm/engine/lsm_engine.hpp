#pragma once
#include "../../engine.hpp"

class LSMEngine : public StorageEngine {
public:
    LSMEngine();
    ~LSMEngine();

    void put(const std::string& key, const std::string& value) override;
    std::optional<std::string> get(const std::string& key) override;
    void remove(const std::string& key) override;
};