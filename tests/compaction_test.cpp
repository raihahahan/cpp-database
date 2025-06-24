#include "catch2/catch_test_macros.hpp"
#include "../src/storage/lsm/engine/lsm_engine.hpp"
#include "../src/storage/wal/wal.hpp"
#include "../src/config.hpp"

#include <filesystem>
#include <cstdio>

TEST_CASE("[Compaction]: works in background thread") {
    std::filesystem::remove_all(SSTABLE_DIR);

    LSMEngine engine("./wal", 5, 5000);

    // insert enough keys to flush multiple segments
    for (int i = 0; i < 50; ++i) {
        engine.put("key" + std::to_string(i % 10), "val" + std::to_string(i));
    }

    // wait for compaction thread to run
    std::this_thread::sleep_for(std::chrono::milliseconds(6000));

    // expect only 1 segment file to remain after compaction
    int datFiles = 0;
    for (const auto& entry : std::filesystem::directory_iterator(SSTABLE_DIR)) {
        if (entry.path().extension() == ".dat") datFiles++;
    }

    REQUIRE(datFiles == 1); // all merged into 1 file
}