#include "lsm_engine.hpp"
#include <iostream>

LSMEngine::LSMEngine() {
    std::cout << "LSMEngine created\n";
}

LSMEngine::~LSMEngine() {
    std::cout << "LSMEngine destroyed\n";
}

void LSMEngine::put(const std::string& key, const std::string& value) {
    std::cout << "Put: " << key << " -> " << value << "\n";
}

std::optional<std::string> LSMEngine::get(const std::string& key) {
    std::cout << "Get: " << key << "\n";
    return std::nullopt;
}

void LSMEngine::remove(const std::string& key) {
    std::cout << "Remove: " << key << "\n";
}
