#pragma once
#include "pipeline_cache_build_identity.hpp"
#include "cache_sha256.hpp"
#include <sqlite3.h>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <initializer_list>
#include <string>
#include <string_view>

namespace PetariPipelineCache {
inline constexpr std::string_view Protocol = "petari-pipeline-cache-v2";
struct Statement {
    sqlite3_stmt* p = nullptr;
    Statement(sqlite3* db, const char* sql) { sqlite3_prepare_v2(db, sql, -1, &p, nullptr); }
    ~Statement() { if (p) sqlite3_finalize(p); }
    Statement(const Statement&) = delete;
    explicit operator bool() const { return p != nullptr; }
};
inline bool textEquals(sqlite3_stmt* s, int col, std::string_view value) {
    return sqlite3_column_type(s,col) == SQLITE_TEXT &&
        sqlite3_column_bytes(s,col) == static_cast<int>(value.size()) &&
        std::memcmp(sqlite3_column_text(s,col),value.data(),value.size()) == 0;
}
inline bool columns(sqlite3* db, const char* pragma, std::initializer_list<std::string_view> names) {
    Statement s(db,pragma); if (!s) return false;
    for (auto name : names) if (sqlite3_step(s.p) != SQLITE_ROW || !textEquals(s.p,1,name)) return false;
    return sqlite3_step(s.p) == SQLITE_DONE;
}
// A nonempty foreign DB is never relabelled. Unknown fields and duplicate rows
// reject the container before any recipe or artifact can be exposed.
inline bool identity(sqlite3* db, bool create = false, std::string_view kind = "recipe",
                     std::string_view key = PETARI_PIPELINE_RECIPE_KEY) {
    if (!db || key.empty()) return false;
    if (columns(db,"PRAGMA table_info(petari_cache_compatibility)",{"protocol","kind","identity"})) {
        Statement s(db,"SELECT protocol,kind,identity FROM petari_cache_compatibility");
        return s && sqlite3_step(s.p) == SQLITE_ROW && textEquals(s.p,0,Protocol) &&
            textEquals(s.p,1,kind) && textEquals(s.p,2,key) && sqlite3_step(s.p) == SQLITE_DONE;
    }
    if (!create) return false;
    {
        Statement count(db,"SELECT count(*) FROM sqlite_master WHERE type='table'");
        if (!count || sqlite3_step(count.p)!=SQLITE_ROW || sqlite3_column_int64(count.p,0)!=0) return false;
    }
    if (sqlite3_exec(db,"BEGIN IMMEDIATE; CREATE TABLE petari_cache_compatibility(protocol TEXT NOT NULL,kind TEXT NOT NULL,identity TEXT NOT NULL)",nullptr,nullptr,nullptr)!=SQLITE_OK) return false;
    bool ok = false;
    { Statement s(db,"INSERT INTO petari_cache_compatibility VALUES(?,?,?)");
      if (s) { sqlite3_bind_text(s.p,1,Protocol.data(),static_cast<int>(Protocol.size()),SQLITE_TRANSIENT);
        sqlite3_bind_text(s.p,2,kind.data(),static_cast<int>(kind.size()),SQLITE_TRANSIENT);
        sqlite3_bind_text(s.p,3,key.data(),static_cast<int>(key.size()),SQLITE_TRANSIENT);
        ok = sqlite3_step(s.p)==SQLITE_DONE; } }
    if (sqlite3_exec(db,ok?"COMMIT":"ROLLBACK",nullptr,nullptr,nullptr)!=SQLITE_OK) ok=false;
    return ok;
}
inline std::string filename(const char* stem) {
    return std::string(stem) + ".v2." + PETARI_PIPELINE_RECIPE_KEY + ".db";
}
inline bool sameBuildFile(const char* path) { // historical API name; recipe ABI, never an app SHA
    sqlite3* db=nullptr;
    const bool ok=sqlite3_open_v2(path,&db,SQLITE_OPEN_READONLY,nullptr)==SQLITE_OK && identity(db);
    if (db) sqlite3_close(db); return ok;
}
struct DeviceContext {
    std::string backend, vendor, device, architecture, model, driver, os;
    static bool known(std::string_view value) { return !value.empty() && value!="Unknown" && value!="unknown"; }
    bool valid() const { return known(backend) && known(vendor) && known(device) &&
        known(architecture) && known(model) && known(driver) && known(os); }
    std::string key() const {
        if (!valid()) return {};
        Sha256 h; h.update("PCC2device");
        for (const auto* v : {&backend,&vendor,&device,&architecture,&model,&driver,&os}) {
            hashNumber(h,v->size(),8); h.update(*v);
        }
        return h.finish();
    }
};
inline std::string artifactIdentity(const DeviceContext& context) {
    const auto device=context.key(); if (device.empty()) return {};
    return digest(std::string("PCC2artifact\n")+PETARI_PIPELINE_RECIPE_KEY+"\n"+PETARI_PIPELINE_BACKEND_KEY+"\n"+device);
}
// Adapter selection initializes this before installing Dawn callbacks. An absent
// context disables disk blobs; it never opens an "Unknown" cache namespace.
inline std::mutex contextMutex;
inline std::string selectedArtifactIdentity;
inline void setDeviceContext(const DeviceContext& context) {
    std::lock_guard lock(contextMutex); selectedArtifactIdentity=artifactIdentity(context);
    std::fprintf(stderr,"[pipeline compatibility] recipe=%s backend=%s device=%s artifact=%s\n",
        PETARI_PIPELINE_RECIPE_KEY,PETARI_PIPELINE_BACKEND_KEY,context.key().c_str(),selectedArtifactIdentity.c_str());
}
inline std::string artifactContext() { std::lock_guard lock(contextMutex); return selectedArtifactIdentity; }
} // namespace PetariPipelineCache
