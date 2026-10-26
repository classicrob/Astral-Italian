#pragma once
#include "pipeline_cache_identity.hpp"
#include <bit>
#include <cmath>
#include <limits>

namespace PetariPipelineCache {
inline uint32_t word(const unsigned char* p,size_t at) {
    return uint32_t(p[at]) | uint32_t(p[at+1])<<8 | uint32_t(p[at+2])<<16 | uint32_t(p[at+3])<<24;
}
// Decode bytes, not bools/enums, until their object representations are proven.
// Fixed field ledger is asserted against the actual Aurora types by the adapter.
inline bool recipeShape(int type,int version,const void* data,size_t size) {
    if(!data || std::endian::native!=std::endian::little) return false;
    const auto* b=static_cast<const unsigned char*>(data);
    if(type==0) return version==PETARI_CACHE_CLEAR_CONFIG_VERSION && size==PETARI_CACHE_CLEAR_CONFIG_SIZE &&
        word(b,0)==static_cast<uint32_t>(version) && b[4]<=1 && b[5]<=1 && b[6]<=1 && b[7]==0;
    if(type!=1 || version!=PETARI_CACHE_GX_CONFIG_VERSION || size!=PETARI_CACHE_GX_CONFIG_SIZE || word(b,0)!=static_cast<uint32_t>(version)) return false;
    constexpr size_t s=8;
    if(word(b,s+2632+4)>255 || word(b,s+2632+16)>255) return false;
    if ((b[s+2]&0xf8) || b[s+3] || word(b,s+2388)==0 || word(b,s+2388)>16 || word(b,s+2716)>4) return false;
    for(size_t i=0;i<26;++i) { const auto a=s+4+i*8; if(b[a+6]>1 || b[a+7]>1 || b[a+5]>31) return false; }
    for(size_t i=0;i<16;++i) {
        const auto t=s+276+i*132;
        for(auto op : {t+32,t+52}) if(b[op+16]>1 || b[op+17] || b[op+18] || b[op+19]) return false;
        if(word(b,t+100)>3 || word(b,t+108)>7 || !((word(b,t+116)<=3) || (word(b,t+116)>=5 && word(b,t+116)<=7) || (word(b,t+116)>=9 && word(b,t+116)<=11))) return false;
        if(b[t+128]>1 || b[t+129]>1 || b[t+130] || b[t+131]) return false;
    }
    for(size_t i=0;i<4;++i) { const auto c=s+2392+i*20;
        if(word(b,c)>1 || word(b,c+4)>1 || word(b,c+8)>2 || word(b,c+12)>2 || b[c+16]>1 || b[c+17] || b[c+18] || b[c+19]) return false;
    }
    for(size_t i=0;i<8;++i) { const auto t=s+2472+i*20;
        if(word(b,t)>10 || b[t+16]>1 || b[t+17]>=8 || b[t+18] || b[t+19]) return false;
        const auto m=word(b,t+8),p=word(b,t+12);
        if (!(m==60 || (m>=30 && m<=57 && (m-30)%3==0)) || !(p==125 || (p>=64 && p<=121 && (p-64)%3==0))) return false;
    }
    if(word(b,2728)>7 || word(b,2732)>3 || word(b,2736)>3 || word(b,2740)>7 || word(b,2744)>7 || word(b,2748)>15 || (word(b,2752)>255 && word(b,2752)!=UINT32_MAX)) return false;
    for(size_t i=2756;i<=2764;i+=4) if(!std::isfinite(std::bit_cast<float>(word(b,i)))) return false;
    return b[2768]<=1 && b[2769]<=1 && b[2770]<=1 && b[2771]<=1;
}
inline std::string recipeDigest(int type,uint64_t hash,int version,const void* data,size_t size) {
    Sha256 h; constexpr char prefix[]="PCC2recipe"; h.update(prefix,sizeof(prefix));
    h.update(PETARI_PIPELINE_RECIPE_KEY); const char zero=0; h.update(&zero,1);
    hashNumber(h,static_cast<uint32_t>(type),4);hashNumber(h,hash,8);hashNumber(h,version,4);hashNumber(h,size,4);h.update(data,size);return h.finish();
}
inline bool recipeContainer(sqlite3* db,bool require=false) {
    Statement tables(db,"SELECT count(*) FROM sqlite_master WHERE type='table' AND name IN ('aurora_schema','pipeline_cache')");
    if(!tables || sqlite3_step(tables.p)!=SQLITE_ROW) return false;
    const auto count=sqlite3_column_int64(tables.p,0);
    if(count==0) return !require;
    if(require && !columns(db,"PRAGMA table_info(petari_recipe_integrity)",{"type","hash","digest"})) return false;
    if(count!=2 || !columns(db,"PRAGMA table_info(aurora_schema)",{"value"}) ||
        !columns(db,"PRAGMA table_info(pipeline_cache)",{"type","hash","config_version","config_size","config","first_frame_used"})) return false;
    Statement version(db,"SELECT value FROM aurora_schema");
    return version && sqlite3_step(version.p)==SQLITE_ROW && sqlite3_column_type(version.p,0)==SQLITE_INTEGER &&
        sqlite3_column_int64(version.p,0)==1 && sqlite3_step(version.p)==SQLITE_DONE;
}
inline bool ensureRecipeIntegrity(sqlite3* db) {
    return sqlite3_exec(db,"CREATE TABLE IF NOT EXISTS petari_recipe_integrity(type INTEGER NOT NULL,hash INTEGER NOT NULL,digest TEXT NOT NULL,PRIMARY KEY(type,hash))",nullptr,nullptr,nullptr)==SQLITE_OK &&
        columns(db,"PRAGMA table_info(petari_recipe_integrity)",{"type","hash","digest"});
}
inline bool sealRecipe(sqlite3* db,int type,uint64_t hash,int version,const void* data,size_t size) {
    if(!recipeShape(type,version,data,size)) return false;
    Statement s(db,"INSERT OR REPLACE INTO petari_recipe_integrity VALUES(?,?,?)"); if(!s) return false;
    const auto sum=recipeDigest(type,hash,version,data,size);
    sqlite3_bind_int(s.p,1,type);sqlite3_bind_int64(s.p,2,static_cast<sqlite3_int64>(hash));sqlite3_bind_text(s.p,3,sum.c_str(),-1,SQLITE_TRANSIENT);
    return sqlite3_step(s.p)==SQLITE_DONE;
}
class RecipeRows {
    Statement checksum_;
public:
    explicit RecipeRows(sqlite3* db):checksum_(db,"SELECT digest FROM petari_recipe_integrity WHERE type=? AND hash=?") {}
    bool valid(int type,uint64_t hash,int version,const void* data,size_t size) {
        if(!recipeShape(type,version,data,size) || !checksum_) return false;
        sqlite3_reset(checksum_.p); sqlite3_clear_bindings(checksum_.p);
        sqlite3_bind_int(checksum_.p,1,type);sqlite3_bind_int64(checksum_.p,2,static_cast<sqlite3_int64>(hash));
        const auto sum=recipeDigest(type,hash,version,data,size);
        return sqlite3_step(checksum_.p)==SQLITE_ROW && textEquals(checksum_.p,0,sum) && sqlite3_step(checksum_.p)==SQLITE_DONE;
    }
};
// Common SELECT envelope order; preserves SQLite types instead of coercing TEXT
// or truncating a 64-bit rank/version into a valid-looking unsigned value.
inline constexpr const char* RecipeSelect = "type,hash,config_version,config_size,config,first_frame_used";
inline bool rowEnvelope(sqlite3_stmt* s,int first=0) {
    for(int i : {0,1,2,3,5}) if(sqlite3_column_type(s,first+i)!=SQLITE_INTEGER) return false;
    if(sqlite3_column_type(s,first+4)!=SQLITE_BLOB) return false;
    const auto type=sqlite3_column_int64(s,first),version=sqlite3_column_int64(s,first+2),size=sqlite3_column_int64(s,first+3),rank=sqlite3_column_int64(s,first+5);
    return (type==0 || type==1) && version>0 && version<=UINT32_MAX && size>0 && size<=2772 &&
        size==sqlite3_column_bytes(s,first+4) && rank>=0 && rank<=UINT32_MAX;
}
} // namespace PetariPipelineCache
