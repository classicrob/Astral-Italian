#pragma once
#include "pipeline_recipe_validation.hpp"
#include <filesystem>
#include <vector>

namespace PetariPipelineCache {
inline bool emptyRecipes(sqlite3* db) {
    Statement s(db,"SELECT count(*) FROM pipeline_cache");
    return s && sqlite3_step(s.p)==SQLITE_ROW && sqlite3_column_int64(s.p,0)==0;
}
// Invoked only inside the selected/private cache root after the machine lock is
// acquired. Legacy files are READONLY; no compiler blobs are imported or renamed.
// The transaction includes recipe rows, checksums, and the one-time receipt.
template<class Writer> bool migrateRecipes(sqlite3* target,const std::filesystem::path& root,Writer write) {
    if(sqlite3_exec(target,"CREATE TABLE IF NOT EXISTS petari_recipe_migration(version INTEGER NOT NULL,files INTEGER NOT NULL,accepted INTEGER NOT NULL,rejected INTEGER NOT NULL)",nullptr,nullptr,nullptr)!=SQLITE_OK) return false;
    if(!columns(target,"PRAGMA table_info(petari_recipe_migration)",{"version","files","accepted","rejected"})) return false;
    { Statement s(target,"SELECT version FROM petari_recipe_migration");
      if(!s) return false;
      const auto step=sqlite3_step(s.p);
      if(step==SQLITE_ROW) return sqlite3_column_type(s.p,0)==SQLITE_INTEGER && sqlite3_column_int64(s.p,0)==1 && sqlite3_step(s.p)==SQLITE_DONE;
      if(step!=SQLITE_DONE) return false;
    }
    if(sqlite3_exec(target,"BEGIN IMMEDIATE",nullptr,nullptr,nullptr)!=SQLITE_OK) return false;
    uint64_t files=0,accepted=0,rejected=0; bool ok=true;
    std::error_code ec;
    std::vector<std::filesystem::path> paths;
    for(std::filesystem::directory_iterator it(root,ec),end; !ec && it!=end; it.increment(ec)) {
        const auto name=it->path().filename().string();
        if(name.size()!=std::strlen("pipeline_cache.")+64+3 || !name.starts_with("pipeline_cache.") || !name.ends_with(".db") || it->is_symlink(ec) || !it->is_regular_file(ec)) continue;
        const auto key=name.substr(std::strlen("pipeline_cache."),64);
        if(key.find_first_not_of("0123456789abcdef")!=std::string::npos) continue;
        paths.push_back(it->path());
    }
    std::sort(paths.begin(),paths.end());
    // Bound unexpected directories rather than spending unbounded launch time.
    if(ec || paths.size()>256) ok=false;
    for(const auto& path:paths) {
        if(!ok) break;
        sqlite3* source=nullptr;
        if(sqlite3_open_v2(path.c_str(),&source,SQLITE_OPEN_READONLY,nullptr)!=SQLITE_OK) { if(source)sqlite3_close(source);++rejected;continue; }
        ++files;
        const auto key=path.filename().string().substr(std::strlen("pipeline_cache."),64);
        bool legacy=false;
        { Statement id(source,"SELECT build_key FROM petari_cache_build"); Statement check(source,"PRAGMA quick_check");
          legacy=columns(source,"PRAGMA table_info(petari_cache_build)",{"build_key"}) && id && sqlite3_step(id.p)==SQLITE_ROW && textEquals(id.p,0,key) && sqlite3_step(id.p)==SQLITE_DONE &&
              check && sqlite3_step(check.p)==SQLITE_ROW && textEquals(check.p,0,"ok") && sqlite3_step(check.p)==SQLITE_DONE;
        }
        if(legacy) {
            Statement s(source,"SELECT type,hash,config_version,config_size,config,first_frame_used FROM pipeline_cache ORDER BY first_frame_used,hash");
            if(!s) ++rejected;
            else { int step;
                while((step=sqlite3_step(s.p))==SQLITE_ROW) {
                    if(!rowEnvelope(s.p)) {++rejected;continue;}
                    const auto type=sqlite3_column_int(s.p,0),version=sqlite3_column_int(s.p,2);
                    const auto hash=static_cast<uint64_t>(sqlite3_column_int64(s.p,1));
                    const auto* data=sqlite3_column_blob(s.p,4);const auto size=static_cast<size_t>(sqlite3_column_bytes(s.p,4));
                    if(!recipeValid(type,hash,version,data,size)) {++rejected;continue;}
                    if(!write(type,hash,version,data,size,static_cast<uint32_t>(sqlite3_column_int64(s.p,5)))) {ok=false;break;}
                    ++accepted;
                }
                if(step!=SQLITE_DONE) ok=false;
            }
        } else ++rejected;
        sqlite3_close(source);
    }
    if(ok) { Statement s(target,"INSERT INTO petari_recipe_migration VALUES(1,?,?,?)");
        ok=bool(s); if(ok) {sqlite3_bind_int64(s.p,1,files);sqlite3_bind_int64(s.p,2,accepted);sqlite3_bind_int64(s.p,3,rejected);ok=sqlite3_step(s.p)==SQLITE_DONE;} }
    if(sqlite3_exec(target,ok?"COMMIT":"ROLLBACK",nullptr,nullptr,nullptr)!=SQLITE_OK) ok=false;
    std::fprintf(stderr,"[pipeline compatibility migration] files=%llu accepted=%llu rejected=%llu committed=%d legacy_dawn=not-adopted\n",(unsigned long long)files,(unsigned long long)accepted,(unsigned long long)rejected,ok);
    return ok;
}
} // namespace PetariPipelineCache
