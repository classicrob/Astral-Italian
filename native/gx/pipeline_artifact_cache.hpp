#pragma once
#include "pipeline_cache_identity.hpp"
#include <limits>
#include <vector>
#if defined(AURORA_CACHE_USE_ZSTD)
#include <zstd.h>
#endif

namespace PetariPipelineCache {
// The actual Dawn callback implementation and its native tests share this class.
// The caller serializes callbacks (Aurora's existing cache mutex). No game locks.
class ArtifactCache {
    sqlite3* db_ = nullptr;
    std::string context_;
    bool disabled_ = false;
    bool finished_ = false;
public:
    static constexpr size_t MaxKey = 1024*1024, MaxValue = 64*1024*1024;
    uint64_t hits=0, misses=0, rejected=0, stores=0;
    static std::string entryDigest(std::string_view context,const void* key,size_t keySize,const void* value,size_t size) {
        Sha256 h;h.update("PCC2artifact-entry");hashNumber(h,2,4);hashNumber(h,context.size(),8);h.update(context);
        hashNumber(h,keySize,8);h.update(key,keySize);hashNumber(h,size,8);h.update(value,size);return h.finish();
    }
    ~ArtifactCache() { close(); }
    int close() {
        if (!db_) return SQLITE_OK;
        const int rc=sqlite3_close(db_);
        if (rc==SQLITE_OK) db_=nullptr;
        return rc;
    }
    // Terminal process-exit operation, serialized with callbacks by their mutex.
    // Aurora shutdown and Host::_Exit may both call this. Late Dawn callbacks
    // must never reopen the DB or change the final counters after the report.
    bool finish(FILE* report = stderr) {
        if (finished_) return false;
        finished_ = true;
        // Some system SQLite builds keep WAL files even after a clean close.
        // Ask SQLite (never unlink a live WAL ourselves) for a final checkpoint
        // and normal WAL cleanup, and expose any failure instead of hiding it.
        int persist=0;
        const int persistence=db_?sqlite3_file_control(db_,nullptr,SQLITE_FCNTL_PERSIST_WAL,&persist):SQLITE_OK;
        const int checkpoint=db_?sqlite3_wal_checkpoint_v2(db_,nullptr,SQLITE_CHECKPOINT_TRUNCATE,nullptr,nullptr):SQLITE_OK;
        const int closed=close();
        std::fprintf(report,"[pipeline artifacts] hits=%llu misses=%llu rejected=%llu stored=%llu\n",
            (unsigned long long)hits,(unsigned long long)misses,
            (unsigned long long)rejected,(unsigned long long)stores);
        std::fprintf(report,"[pipeline artifacts close] persistence=%d checkpoint=%d close=%d\n",persistence,checkpoint,closed);
        std::fflush(report);
        return true;
    }
    bool open(const std::string& path, const std::string& context) {
        if (finished_ || disabled_ || context.empty()) return false;
        if (db_) return context_==context;
        context_=context;
        if (sqlite3_open_v2(path.c_str(),&db_,SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE,nullptr)!=SQLITE_OK ||
            !identity(db_,true,"artifact",context)) {
            std::fprintf(stderr,"[pipeline artifacts reject] path=%s reason=container-identity-or-open\n",path.c_str());
            close(); disabled_=true; return false;
        }
        // No destructive schema upgrade. A malformed existing schema disables reuse.
        if (sqlite3_exec(db_,"CREATE TABLE IF NOT EXISTS artifacts(key_hash TEXT PRIMARY KEY NOT NULL,full_key BLOB NOT NULL,context TEXT NOT NULL,version INTEGER NOT NULL,value BLOB NOT NULL,size INTEGER NOT NULL,encoding INTEGER NOT NULL,digest TEXT NOT NULL)",nullptr,nullptr,nullptr)!=SQLITE_OK ||
            !columns(db_,"PRAGMA table_info(artifacts)",{"key_hash","full_key","context","version","value","size","encoding","digest"})) {
            std::fprintf(stderr,"[pipeline artifacts reject] path=%s reason=unknown-schema\n",path.c_str());
            close(); disabled_=true; return false;
        }
        std::fprintf(stderr,"[pipeline artifacts open] path=%s namespace=%s legacy=not-adopted\n",path.c_str(),context.c_str());
        sqlite3_exec(db_,"PRAGMA journal_mode=WAL; PRAGMA synchronous=NORMAL",nullptr,nullptr,nullptr);
        return true;
    }
    // Every envelope, byte count and digest is checked on BOTH API calls. Never
    // return a trusted logical size before inspecting the actual stored blob.
    size_t load(const void* key, size_t keySize, void* value, size_t valueSize) {
        if (finished_) return 0;
        if (!db_ || !key || !keySize || keySize>MaxKey) { ++misses; return 0; }
        Statement s(db_,"SELECT full_key,context,version,value,size,encoding,digest FROM artifacts WHERE key_hash=?");
        if (!s) { ++misses; return 0; }
        const auto index=digest(key,keySize);
        sqlite3_bind_text(s.p,1,index.c_str(),-1,SQLITE_TRANSIENT);
        if (sqlite3_step(s.p)!=SQLITE_ROW) { ++misses; return 0; }
        auto reject=[&]() -> size_t { ++rejected; ++misses; return 0; };
        if (sqlite3_column_type(s.p,0)!=SQLITE_BLOB || sqlite3_column_bytes(s.p,0)!=static_cast<int>(keySize) ||
            std::memcmp(sqlite3_column_blob(s.p,0),key,keySize)!=0 || !textEquals(s.p,1,context_) ||
            sqlite3_column_type(s.p,2)!=SQLITE_INTEGER || sqlite3_column_int64(s.p,2)!=2 ||
            sqlite3_column_type(s.p,3)!=SQLITE_BLOB || sqlite3_column_type(s.p,4)!=SQLITE_INTEGER ||
            sqlite3_column_type(s.p,5)!=SQLITE_INTEGER) return reject();
        const auto size=sqlite3_column_int64(s.p,4), encoding=sqlite3_column_int64(s.p,5);
        const auto bytes=sqlite3_column_bytes(s.p,3);
        const auto* blob=sqlite3_column_blob(s.p,3);
        if(size<=0 || static_cast<uint64_t>(size)>MaxValue || bytes<=0 || static_cast<size_t>(bytes)>MaxValue || !blob) return reject();
        std::vector<uint8_t> decoded;
        const void* raw=blob;
        if(encoding==0) { if(size!=bytes) return reject(); }
        else if(encoding==1) {
#if defined(AURORA_CACHE_USE_ZSTD)
            if(ZSTD_getFrameContentSize(blob,bytes)!=static_cast<uint64_t>(size)) return reject();
            decoded.resize(static_cast<size_t>(size));
            const auto n=ZSTD_decompress(decoded.data(),decoded.size(),blob,static_cast<size_t>(bytes));
            if(ZSTD_isError(n) || n!=static_cast<uint64_t>(size)) return reject();
            raw=decoded.data();
#else
            return reject();
#endif
        } else return reject();
        const auto sum=entryDigest(context_,key,keySize,raw,static_cast<size_t>(size));
        if (!textEquals(s.p,6,sum)) return reject();
        if (value) {
            if(valueSize!=static_cast<uint64_t>(size)) return reject();
            std::memcpy(value,raw,static_cast<size_t>(size)); ++hits;
        }
        return static_cast<size_t>(size);
    }
    bool store(const void* key,size_t keySize,const void* value,size_t valueSize) {
        if (finished_) return false;
        if(!db_ || !key || !keySize || keySize>MaxKey || !value || !valueSize || valueSize>MaxValue) return false;
        const auto index=digest(key,keySize), sum=entryDigest(context_,key,keySize,value,valueSize);
        const void* blob=value; size_t bytes=valueSize; int encoding=0;
#if defined(AURORA_CACHE_USE_ZSTD)
        std::vector<uint8_t> packed(ZSTD_compressBound(valueSize));
        const auto n=ZSTD_compress(packed.data(),packed.size(),value,valueSize,0);
        if(!ZSTD_isError(n) && n<valueSize) { blob=packed.data(); bytes=n; encoding=1; }
#endif
        Statement s(db_,"INSERT OR REPLACE INTO artifacts VALUES(?,?,?,2,?,?,?,?)");
        if(!s) return false;
        if(sqlite3_bind_text(s.p,1,index.c_str(),-1,SQLITE_TRANSIENT)!=SQLITE_OK ||
           sqlite3_bind_blob64(s.p,2,key,keySize,SQLITE_TRANSIENT)!=SQLITE_OK ||
           sqlite3_bind_text(s.p,3,context_.c_str(),-1,SQLITE_TRANSIENT)!=SQLITE_OK ||
           sqlite3_bind_blob64(s.p,4,blob,bytes,SQLITE_TRANSIENT)!=SQLITE_OK ||
           sqlite3_bind_int64(s.p,5,static_cast<sqlite3_int64>(valueSize))!=SQLITE_OK ||
           sqlite3_bind_int(s.p,6,encoding)!=SQLITE_OK ||
           sqlite3_bind_text(s.p,7,sum.c_str(),-1,SQLITE_TRANSIENT)!=SQLITE_OK || sqlite3_step(s.p)!=SQLITE_DONE) return false;
        ++stores; return true;
    }
};
} // namespace PetariPipelineCache
