#include "catch2/catch_test_macros.hpp"
#include "../src/storage/lsm/memtable/memtable.hpp"
#include "../src/config.hpp"

TEST_CASE("[memtable]: Basic put/get/remove operations") {
    Memtable mem;

    SECTION("put and get") {
        mem.put("a", "apple");
        mem.put("b", "banana");

        auto a = mem.get("a");
        auto b = mem.get("b");

        REQUIRE(a.has_value());
        REQUIRE(a.value() == "apple");

        REQUIRE(b.has_value());
        REQUIRE(b.value() == "banana");
    }

    SECTION("get non-existing key") {
        auto x = mem.get("nonexistent");
        REQUIRE_FALSE(x.has_value());
    }

    SECTION("overwrite key") {
        mem.put("a", "apple");
        mem.put("a", "apricot");

        auto a = mem.get("a");
        REQUIRE(a.has_value());
        REQUIRE(a.value() == "apricot");
    }

    SECTION("remove key") {
        mem.put("a", "apple");
        mem.remove("a");

        auto a = mem.get("a");
        REQUIRE(a == TOMBSTONE_MARKER);
    }

    SECTION("entries are sorted") {
        mem.put("c", "carrot");
        mem.put("a", "apple");
        mem.put("b", "banana");

        auto entries = mem.getRange(-1);
        REQUIRE(entries.size() == 3);
        REQUIRE(entries[0].first == "a");
        REQUIRE(entries[1].first == "b");
        REQUIRE(entries[2].first == "c");
    }
}
