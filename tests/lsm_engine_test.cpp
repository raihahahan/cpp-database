#include "catch2/catch_test_macros.hpp"
#include "../src/storage/lsm/engine/lsm_engine.hpp"
#include "../src/storage/lsm/wal/wal.hpp"
#include "../src/config.hpp"

#include <filesystem>
#include <cstdio>

TEST_CASE("[lsm_engine]: WAL is correctly written by put/remove") {
    using namespace std;
    using namespace std::filesystem;

    // Setup
    path walPath = "data-engine/db.wal";
    remove(walPath);

    {
        LSMEngine engine(walPath);
        engine.put("a", "apple");
        engine.put("b", "banana");
        engine.remove("a");
    }

    // read back WAL manually
    FILE* fp = std::fopen(walPath.string().c_str(), "rb");
    REQUIRE(fp != nullptr);

    WalRecord r1 = WalRecord::deserialize(fp).value();
    REQUIRE(r1.opType == OpType::CREATE);
    REQUIRE(r1.key == "a");
    REQUIRE(r1.value == "apple");

    WalRecord r2 = WalRecord::deserialize(fp).value();
    REQUIRE(r2.opType == OpType::CREATE);
    REQUIRE(r2.key == "b");
    REQUIRE(r2.value == "banana");

    WalRecord r3 = WalRecord::deserialize(fp).value();
    REQUIRE(r3.opType == OpType::DELETE);
    REQUIRE(r3.key == "a");
    REQUIRE(r3.value == "");

    std::fclose(fp);
}

TEST_CASE("[lsm_engine]: WAL replay on recovery") {
    using namespace std;
    using namespace std::filesystem;

    // clean up old WAL
    path walPath = "data-recovery/db.wal";
    remove_all(walPath.parent_path());

    {
        // first engine: write to WAL and memtable
        LSMEngine engine(walPath);
        engine.put("a", "apple");
        engine.put("b", "banana");
        engine.remove("a");
    }

    {
        // second engine: should recover from WAL
        LSMEngine engine(walPath);

        // check that "a" was deleted, and "b" was recovered
        auto valA = engine.get("a");
        auto valB = engine.get("b");

        REQUIRE(!valA.has_value());
        REQUIRE(valB.has_value());
        REQUIRE(valB.value() == "banana");
    }
}