#include "catch2/catch_test_macros.hpp"
#include "../src/common/containers/skiplist.hpp"
#include <string>

TEST_CASE("[skiplist] basic insert + get") {
    SkipList<std::string, std::string> map;

    map.insert("apple", "red");
    map.insert("banana", "yellow");

    REQUIRE(map.get("apple").has_value());
    REQUIRE(map.get("apple").value() == "red");

    REQUIRE(map.get("banana").has_value());
    REQUIRE(map.get("banana").value() == "yellow");

    REQUIRE_FALSE(map.get("cherry").has_value());
}

TEST_CASE("[skiplist] overwrite existing key") {
    SkipList<std::string, std::string> map;

    map.insert("apple", "red");
    map.insert("apple", "green");

    REQUIRE(map.get("apple").has_value());
    REQUIRE(map.get("apple").value() == "green");
}

TEST_CASE("[skiplist] remove entries") {
    SkipList<std::string, std::string> map;

    map.insert("a", "1");
    map.insert("b", "2");
    map.insert("c", "3");

    map.remove("b");

    REQUIRE(map.get("a").has_value());
    REQUIRE_FALSE(map.get("b").has_value());
    REQUIRE(map.get("c").has_value());

    REQUIRE(map.size() == 2);
}

TEST_CASE("[skiplist] entries in sorted order") {
    SkipList<std::string, int> map;

    map.insert("dog", 1);
    map.insert("cat", 2);
    map.insert("bird", 3);

    auto entries = map.entries();

    REQUIRE(entries.size() == 3);
    REQUIRE(entries[0].first == "bird");
    REQUIRE(entries[1].first == "cat");
    REQUIRE(entries[2].first == "dog");
}
