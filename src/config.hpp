#pragma once

constexpr const char* WAL_PATH = "data/db.wal";
constexpr const char* SSTABLE_DIR = "data/segments";
constexpr const int LSM_FLUSH_THRESHOLD = 2000;
constexpr const int LSM_COMPACTION_INTERVAL_MS = 10000;
constexpr const char* TOMBSTONE_MARKER = "\x1ETOMB";