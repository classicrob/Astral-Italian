#pragma once
#include "pipeline_cache_build_identity.hpp"
#include <sqlite3.h>
#include <cstdio>
#include <cstring>
#include <string>

namespace PetariPipelineCache {
inline std::string filename(const char* stem) {
    return std::string(stem) + "." + PETARI_PIPELINE_BUILD_KEY + ".db";
}
// Existing databases without an identity are foreign, even if renamed to this
// build's filename. Only a new/empty database may acquire the compiled identity.
inline bool identity(sqlite3* db, bool create = false) {
    sqlite3_stmt* statement = nullptr;
    bool match = false;
    if (sqlite3_prepare_v2(db, "SELECT build_key FROM petari_cache_build", -1, &statement, nullptr) == SQLITE_OK) {
        if (sqlite3_step(statement) == SQLITE_ROW) {
            const auto* key = sqlite3_column_text(statement, 0);
            match = key && std::strcmp(reinterpret_cast<const char*>(key), PETARI_PIPELINE_BUILD_KEY) == 0;
            match = sqlite3_step(statement) == SQLITE_DONE && match;
        }
        sqlite3_finalize(statement);
        return match;
    }
    if (statement) sqlite3_finalize(statement);
    if (!create) return false;
    int tables = -1;
    if (sqlite3_prepare_v2(db, "SELECT count(*) FROM sqlite_master WHERE type='table'", -1, &statement, nullptr) == SQLITE_OK &&
        sqlite3_step(statement) == SQLITE_ROW) tables = sqlite3_column_int(statement, 0);
    if (statement) sqlite3_finalize(statement);
    if (tables != 0) return false;
    if (sqlite3_exec(db, "CREATE TABLE petari_cache_build(build_key TEXT NOT NULL)", nullptr, nullptr, nullptr) != SQLITE_OK) return false;
    if (sqlite3_prepare_v2(db, "INSERT INTO petari_cache_build VALUES(?)", -1, &statement, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(statement, 1, PETARI_PIPELINE_BUILD_KEY, -1, SQLITE_STATIC);
    match = sqlite3_step(statement) == SQLITE_DONE;
    sqlite3_finalize(statement);
    return match;
}
inline bool sameBuildFile(const char* path) {
    sqlite3* db = nullptr;
    const bool opened = sqlite3_open_v2(path, &db, SQLITE_OPEN_READONLY, nullptr) == SQLITE_OK;
    const bool match = opened && identity(db);
    if (db) sqlite3_close(db);
    return match;
}
} // namespace PetariPipelineCache
