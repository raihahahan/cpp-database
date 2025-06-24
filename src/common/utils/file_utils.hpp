#pragma once
#include <string>
#include <iostream>
#include "../../config.hpp"

inline bool isTombstone(const std::string& value) {
    return value.rfind(TOMBSTONE_MARKER, 0) == 0;
}