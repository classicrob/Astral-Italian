#pragma once
// Fingerprint the bytes read from SQLite, before copying a config into a C++
// variant. Padding in those variants must never change the sidecar binding.
#include "pipeline_deadlines.hpp"
#include <sqlite3.h>

namespace PetariDeadlines {
inline Result loadManifest(sqlite3* db, const std::filesystem::path& sidecar, const std::string& stage) {
    if (!db) return {false, "manifest-unavailable", {}};
    sqlite3_stmt* query = nullptr;
    if (sqlite3_prepare_v2(db, "SELECT type,hash,config_version,config_size,config,first_frame_used FROM pipeline_cache",
                           -1, &query, nullptr) != SQLITE_OK)
        return {false, "manifest-query", {}};
    struct Finalize {
        sqlite3_stmt* query;
        ~Finalize() { sqlite3_finalize(query); }
    } finalize{query};
    std::vector<std::vector<unsigned char>> blobs;
    std::vector<Row> rows;
    int status;
    while ((status = sqlite3_step(query)) == SQLITE_ROW) {
        if (rows.size() >= 65536) return {false, "manifest-count", {}};
        for (int column : {0, 1, 2, 3, 5})
            if (sqlite3_column_type(query, column) != SQLITE_INTEGER) return {false, "manifest-integer", {}};
        const auto type = sqlite3_column_int64(query, 0), version = sqlite3_column_int64(query, 2);
        const auto size = sqlite3_column_int64(query, 3), rank = sqlite3_column_int64(query, 5);
        if (type < 0 || type > 1 || version < 0 || version > UINT16_MAX || size <= 0 || size > UINT16_MAX ||
            rank < 0 || rank > UINT32_MAX || sqlite3_column_type(query, 4) != SQLITE_BLOB ||
            sqlite3_column_bytes(query, 4) != size)
            return {false, "manifest-row", {}};
        const auto* data = static_cast<const unsigned char*>(sqlite3_column_blob(query, 4));
        if (!data) return {false, "manifest-blob", {}};
        blobs.emplace_back(data, data + size);
        rows.push_back({static_cast<std::uint8_t>(type), static_cast<std::uint64_t>(sqlite3_column_int64(query, 1)),
                        static_cast<std::uint16_t>(version), blobs.back(), static_cast<std::uint32_t>(rank)});
    }
    if (status != SQLITE_DONE || rows.empty()) return {false, "manifest-read", {}};
    return load(sidecar, stage, rows);
}
} // namespace PetariDeadlines
