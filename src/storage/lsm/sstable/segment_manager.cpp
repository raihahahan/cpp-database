#include "segment_manager.hpp"
#include "../../../config.hpp"

#include <fstream>
#include <iostream>
#include <chrono>
#include <sstream>
#include <string>
#include <map>

std::string SegmentManager::generateSegmentFilename() const {
    auto now = std::chrono::system_clock::now().time_since_epoch();
    return "segment_" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(now).count()) + ".dat";
}

void SegmentManager::flush(const std::vector<std::pair<std::string, std::string>>& data) {
    std::filesystem::create_directories(segmentDir);
    std::string filename = generateSegmentFilename();
    auto filepath = segmentDir / filename;

    std::ofstream out(filepath, std::ios::binary);
    if (!out) {
        std::cerr << "Failed to open segment file for writing.\n";
        return;
    }

    // write each kv pair to the file and record its byte offset
    for (const auto& [key, value] : data) {
        std::streampos offset = out.tellp();

        uint32_t kSize = key.size();
        uint32_t vSize = value.size();

        out.write(reinterpret_cast<const char*>(&kSize), sizeof(kSize));
        out.write(key.data(), kSize);
        out.write(reinterpret_cast<const char*>(&vSize), sizeof(vSize));
        out.write(value.data(), vSize);

        indexMap[key] = { filepath.string(), offset };
    }

    out.close();
    std::cout << "[Flush] Wrote " << data.size() << " entries to " << filepath << "\n";
}

void SegmentManager::loadSegments(const std::filesystem::path& dir) {
    segmentDir = dir;
    // rebuild the in-memory index map
    indexMap.clear();

    std::filesystem::create_directories(dir);

    for (const auto& entry : std::filesystem::directory_iterator(dir)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".dat") continue;

        std::ifstream in(entry.path(), std::ios::binary);
        if (!in) continue;

        while (in.peek() != EOF) {
            std::streampos offset = in.tellg();

            uint32_t kSize, vSize;
            in.read(reinterpret_cast<char*>(&kSize), sizeof(kSize));
            if (in.eof()) break;

            std::string key(kSize, '\0');
            in.read(&key[0], kSize);

            in.read(reinterpret_cast<char*>(&vSize), sizeof(vSize));
            std::string value(vSize, '\0');
            in.read(&value[0], vSize);

            indexMap[key] = { entry.path().string(), offset };
        }

        in.close();
    }

    std::cout << "[Startup] Loaded " << indexMap.size() << " entries from segments.\n";
}

std::optional<std::string> SegmentManager::get(const std::string& key) const {
    auto it = indexMap.find(key);
    if (it == indexMap.end()) return std::nullopt;

    const auto& [filepath, offset] = it->second;

    std::ifstream in(filepath, std::ios::binary);
    if (!in) return std::nullopt;

    // seek to stored offset for the key inside the filepath
    in.seekg(offset);
    uint32_t kSize, vSize;

    in.read(reinterpret_cast<char*>(&kSize), sizeof(kSize));
    in.ignore(kSize);
    in.read(reinterpret_cast<char*>(&vSize), sizeof(vSize));

    std::string value(vSize, '\0');
    in.read(&value[0], vSize);

    return value;
}

std::vector<std::pair<std::string, std::string>> SegmentManager::getRange(int limit) const {
    std::vector<std::pair<std::string, std::string>> result;
    for (const auto& [key, loc] : indexMap) {
        if (limit != -1 && result.size() >= static_cast<size_t>(limit)) break;
        if (auto val = get(key)) result.emplace_back(key, *val);
    }
    return result;
}

void SegmentManager::compact() {
    std::cout << "[Compaction] Starting compaction...\n";

    // 1. read all entries from indexMap
    auto allEntries = getRange();

    // 2. sort and deduplicate by key, keeping latest (rightmost) entry
    std::map<std::string, std::string> latest;  // overwrites duplicates
    for (const auto& [key, value] : allEntries) {
        latest[key] = value;
    }

    // 3. write to new compacted segment
    std::string compactedFilename = generateSegmentFilename();
    auto compactedPath = segmentDir / compactedFilename;

    std::ofstream out(compactedPath, std::ios::binary);
    if (!out) {
        std::cerr << "[Compaction] Failed to open compacted segment file.\n";
        return;
    }

    // 4. rebuild new index map
    indexMap.clear(); 

    for (const auto& [key, value] : latest) {
        std::streampos offset = out.tellp();

        uint32_t kSize = key.size();
        uint32_t vSize = value.size();

        out.write(reinterpret_cast<const char*>(&kSize), sizeof(kSize));
        out.write(key.data(), kSize);
        out.write(reinterpret_cast<const char*>(&vSize), sizeof(vSize));
        out.write(value.data(), vSize);

        indexMap[key] = { compactedPath.string(), offset };
    }

    out.close();

    // 5. delete all old segment files
    for (const auto& entry : std::filesystem::directory_iterator(segmentDir)) {
        if (entry.path() != compactedPath && entry.path().extension() == ".dat") {
            std::filesystem::remove(entry.path());
        }
    }

    std::cout << "[Compaction] Finished. Compacted into " << compactedPath << " with "
              << latest.size() << " entries.\n";
}
