#include "catch2/catch_test_macros.hpp"
#include "../src/storage/lsm/wal/wal.hpp"
#include <filesystem>
#include <fstream>
#include <cstdio>

TEST_CASE("[wal]: WAL append and deserialize") {
    using namespace std;
    using namespace std::filesystem;

    // 1. setup
    path testPath = "data/db.wal";
    create_directories(testPath.parent_path());
    remove(testPath);  // clean existing
    std::ofstream dummy(testPath); dummy.close();

    WalRecord r1{OpType::CREATE, "key1", "value1"};
    WalRecord r2{OpType::UPDATE, "key2", "value2"};

    {
        // write
        FILE* fp = std::fopen(testPath.string().c_str(), "ab");
        REQUIRE(fp != nullptr);
        std::vector<uint8_t> data1 = r1.serialize();
        std::fwrite(data1.data(), 1, data1.size(), fp);
        std::vector<uint8_t> data2 = r2.serialize();
        std::fwrite(data2.data(), 1, data2.size(), fp);
        std::fclose(fp);
    }

    {
        // read back
        FILE* fp = std::fopen(testPath.string().c_str(), "rb");
        REQUIRE(fp != nullptr);
        WalRecord out1 = WalRecord::deserialize(fp);
        WalRecord out2 = WalRecord::deserialize(fp);
        std::fclose(fp);

        REQUIRE(out1.opType == OpType::CREATE);
        REQUIRE(out1.key == "key1");
        REQUIRE(out1.value == "value1");

        REQUIRE(out2.opType == OpType::UPDATE);
        REQUIRE(out2.key == "key2");
        REQUIRE(out2.value == "value2");
    }
}
