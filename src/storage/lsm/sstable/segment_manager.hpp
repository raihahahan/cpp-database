#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <utility>
#include <filesystem>
#include <optional>

class SegmentManager {
public:
    void loadSegments(const std::filesystem::path& dir);
    void flush(const std::vector<std::pair<std::string, std::string>>& data);
    std::optional<std::string> get(const std::string& key) const;
    std::vector<std::pair<std::string, std::string>> getRange(int limit = -1) const;


private:
    std::unordered_map<std::string, std::pair<std::string, std::streampos>> indexMap;
    std::filesystem::path segmentDir;
    std::string generateSegmentFilename() const;
};
