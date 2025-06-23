#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include "../src/storage/lsm/sstable/segment_manager.hpp"

namespace fs = std::filesystem;

void cleanDir(const std::string& dir) {
    fs::remove_all(dir);
    fs::create_directories(dir);
}

TEST_CASE("[SegmentManager]: flush and get") {
    cleanDir("data/segments");

    SegmentManager sm;
    sm.loadSegments("data/segments");

    std::vector<std::pair<std::string, std::string>> data = {
        {"apple", "fruit"},
        {"carrot", "veg"},
        {"banana", "fruit"}
    };

    sm.flush(data);

    REQUIRE(sm.get("apple").value() == "fruit");
    REQUIRE(sm.get("carrot").value() == "veg");
    REQUIRE_FALSE(sm.get("not_found").has_value());
}

TEST_CASE("[SegmentManager]: persistence and reload") {
    SegmentManager sm;
    sm.loadSegments("data/segments");

    REQUIRE(sm.get("banana").value() == "fruit");
    REQUIRE(sm.get("carrot").value() == "veg");
}

TEST_CASE("[SegmentManager]: getRange") {
    SegmentManager sm;
    sm.loadSegments("data/segments");

    auto full = sm.getRange();
    REQUIRE(full.size() >= 3);

    auto limited = sm.getRange(2);
    REQUIRE(limited.size() == 2);
}
