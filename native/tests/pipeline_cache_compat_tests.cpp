// Production SQLite validators, with a fresh private HOME/root only.
#include "gx/gx.hpp"
#include "gx/pipeline.hpp"
#include "gfx/clear.hpp"
#include "gfx/hash.hpp"
#include "pipeline_cache_migration.hpp"
#include "pipeline_artifact_cache.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <unistd.h>

namespace fs=std::filesystem;
namespace C=PetariPipelineCache;
static void need(bool yes,const char* why) {if(!yes)throw std::runtime_error(why);}
static void sql(sqlite3* db,const std::string& text) {need(sqlite3_exec(db,text.c_str(),nullptr,nullptr,nullptr)==SQLITE_OK,text.c_str());}
static std::string fileHash(const fs::path& path) {std::ifstream f(path,std::ios::binary);const std::string s((std::istreambuf_iterator<char>(f)),std::istreambuf_iterator<char>());return C::digest(s);}
static C::DeviceContext context(){return {"Metal","Apple","fixture-gpu","arm64","fixture-model","fixture-driver","fixture-OS"};}
static const char* RecipeTable="CREATE TABLE pipeline_cache(type INTEGER NOT NULL,hash INTEGER NOT NULL,config_version INTEGER NOT NULL,config_size INTEGER NOT NULL,config BLOB NOT NULL,first_frame_used INTEGER NOT NULL,PRIMARY KEY(type,hash))";
static bool writeRecipe(sqlite3* db,int type,uint64_t hash,int version,const void* data,size_t size,uint32_t rank) {
    C::Statement s(db,"INSERT OR REPLACE INTO pipeline_cache VALUES(?,?,?,?,?,?)");if(!s)return false;
    sqlite3_bind_int(s.p,1,type);sqlite3_bind_int64(s.p,2,hash);sqlite3_bind_int(s.p,3,version);sqlite3_bind_int64(s.p,4,size);
    sqlite3_bind_blob(s.p,5,data,static_cast<int>(size),SQLITE_TRANSIENT);sqlite3_bind_int64(s.p,6,rank);
    return sqlite3_step(s.p)==SQLITE_DONE && C::sealRecipe(db,type,hash,version,data,size);
}
static void shaTests() {
    need(C::digest("")=="e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855","empty SHA256");
    need(C::digest("abc")=="ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad","abc SHA256");
    std::string million(1000000,'a');need(C::digest(million)=="cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0","million SHA256");
    C::Sha256 h;for(char a:std::string("abc"))h.update(&a,1);need(h.finish()==C::digest("abc"),"stream SHA256");
}
static void identityTests() {
    sqlite3* db=nullptr;need(sqlite3_open(":memory:",&db)==SQLITE_OK,"identity fixture");
    need(!C::identity(db),"missing identity");need(C::identity(db,true),"empty DB acquires identity");
    sql(db,"INSERT INTO petari_cache_compatibility SELECT * FROM petari_cache_compatibility");
    need(!C::identity(db,true),"duplicate identity cannot relabel");sqlite3_close(db);
    need(sqlite3_open(":memory:",&db)==SQLITE_OK,"foreign fixture");sql(db,"CREATE TABLE data(bytes BLOB)");
    need(!C::identity(db,true),"foreign nonempty DB cannot relabel");sqlite3_close(db);
    need(sqlite3_open(":memory:",&db)==SQLITE_OK,"unknown field fixture");need(C::identity(db,true),"identity initial");
    sql(db,"ALTER TABLE petari_cache_compatibility ADD COLUMN unknown TEXT");
    need(!C::identity(db,true),"unknown metadata fields reject");sqlite3_close(db);
}
static void artifactTests(const fs::path& root) {
    const auto ctx=C::artifactIdentity(context()),path=(root/"artifacts.db").string();
    const std::string key("full-Dawn-key\0binary",20); // the complete binary callback key, not its index
    const std::string value=std::string(4096,'A')+"compiler payload for shader A";
    C::ArtifactCache cache;need(cache.open(path,ctx),"new artifact DB");
    need(cache.store(key.data(),key.size(),value.data(),value.size()),"store");
    std::vector<char> out(value.size()+4,'!');
    auto hit=[&]{need(cache.load(key.data(),key.size(),nullptr,0)==value.size(),"size probe hit");need(cache.load(key.data(),key.size(),out.data(),value.size())==value.size(),"data hit");need(std::string(out.data(),value.size())==value,"exact payload");need(out.back()=='!',"copy canary");};hit();
    // Unrelated build has the SAME namespace and reuses actual persisted bytes.
    cache.close();C::ArtifactCache otherBuild;need(otherBuild.open(path,ctx),"compatible build reopen");
    need(otherBuild.load(key.data(),key.size(),out.data(),value.size())==value.size(),"compatible build hit");otherBuild.close();
    need(cache.open(path,ctx),"original reopen");
    const auto changedKey=key+"changed-emitted-WGSL/options";
    need(cache.load(changedKey.data(),changedKey.size(),nullptr,0)==0,"changed request key miss");
    sqlite3* db=nullptr;need(sqlite3_open(path.c_str(),&db)==SQLITE_OK,"corruption fixture");
    auto rejects=[&](const char* change) {
        sql(db,change);std::fill(out.begin(),out.end(),'!');
        need(cache.load(key.data(),key.size(),nullptr,0)==0,"corrupt size probe must miss");
        need(cache.load(key.data(),key.size(),out.data(),value.size())==0,"corrupt copy must miss");
        need(std::all_of(out.begin(),out.end(),[](char x){return x=='!';}),"rejected copy wrote bytes");
        need(cache.store(key.data(),key.size(),value.data(),value.size()),"repair test fixture");
    };
    rejects("UPDATE artifacts SET value=x'01',encoding=0"); // old raw size-vs-length OOB bug
    rejects("UPDATE artifacts SET size=size+1");
    rejects("UPDATE artifacts SET size=-1");
    rejects("UPDATE artifacts SET size=67108865");
    rejects("UPDATE artifacts SET size='not-an-integer'");
    rejects("UPDATE artifacts SET full_key=x'4142'"); // forged index alone is not enough
    rejects("UPDATE artifacts SET context='different-driver'");
    rejects("UPDATE artifacts SET version=999");
    rejects("UPDATE artifacts SET digest='wrong'");
    rejects("UPDATE artifacts SET encoding=9");
    rejects("UPDATE artifacts SET encoding=1,value=x'00010203'"); // invalid zstd, either feature mode
    need(sqlite3_exec(db,"UPDATE artifacts SET value=NULL",nullptr,nullptr,nullptr)==SQLITE_CONSTRAINT,"null payload constraint");
    sqlite3_close(db);
    need(cache.load(key.data(),key.size(),out.data(),value.size()-1)==0,"wrong copy size miss");
    need(!cache.store(key.data(),C::ArtifactCache::MaxKey+1,value.data(),value.size()),"oversize key rejected before read");
    need(!cache.store(key.data(),key.size(),value.data(),C::ArtifactCache::MaxValue+1),"oversize value rejected before read");
    auto changed=context();changed.device+="new";
    C::ArtifactCache incompatible;need(!incompatible.open(path,C::artifactIdentity(changed)),"foreign context cannot relabel DB");
    std::vector<C::DeviceContext> changes;
    for(auto member:{&C::DeviceContext::backend,&C::DeviceContext::vendor,&C::DeviceContext::device,&C::DeviceContext::architecture,&C::DeviceContext::model,&C::DeviceContext::driver,&C::DeviceContext::os}) {
        auto x=context();x.*member+="new";changes.push_back(x);
    }
    for(const auto& x:changes)need(C::artifactIdentity(x)!=ctx,"GPU/driver/OS/backend context miss");
    need(C::artifactIdentity(C::DeviceContext{}).empty(),"missing adapter disables reuse");
    cache.close();
    {std::ofstream f(root/"truncated.db",std::ios::binary);f<<"SQLite format 3\0broken";}
    C::ArtifactCache truncated;need(!truncated.open((root/"truncated.db").string(),ctx),"truncated SQLite fail closed");
}
static aurora::gx::PipelineConfig validGX() {
    aurora::gx::PipelineConfig c{};
    c.shaderConfig.attrs[GX_VA_POS]={GX_DIRECT,3,GX_F32,0,12};
    c.shaderConfig.vtxStride=12;c.shaderConfig.tevStageCount=1;
    c.depthFunc=GX_ALWAYS;c.cullMode=GX_CULL_NONE;c.blendMode=GX_BM_NONE;c.blendFacSrc=GX_BL_ONE;c.blendFacDst=GX_BL_ZERO;c.blendOp=GX_LO_CLEAR;
    return c;
}
static void recipeTests(const fs::path& root) {
    auto gx=validGX();const auto hash=aurora::xxh3_hash(gx,1);
    need(C::recipeValid(1,hash,13,&gx,sizeof(gx)),"valid production GX recipe");
    need(!C::recipeValid(1,hash+1,13,&gx,sizeof(gx)),"stored config hash recomputed");
    auto sentinel=gx;sentinel.dstAlpha=UINT32_MAX;
    need(C::recipeShape(1,13,&sentinel,sizeof(sentinel)),"no-destination-alpha sentinel");
    auto bad=gx;bad.version=999;need(!C::recipeShape(1,999,&bad,sizeof(bad)),"unknown recipe version");
    need(!C::recipeShape(1,13,&gx,sizeof(gx)-1),"short recipe");
    std::vector<unsigned char> bytes(sizeof(gx)+1);std::memcpy(bytes.data(),&gx,sizeof(gx));need(!C::recipeShape(1,13,bytes.data(),bytes.size()),"unknown trailing field");
    for(const auto offset:{size_t(10),size_t(11),size_t(8+4+6),size_t(8+276+32+16),size_t(8+276+130),size_t(8+2472+18),size_t(2768)}) {
        std::memcpy(bytes.data(),&gx,sizeof(gx));bytes[offset]=255;
        need(!C::recipeShape(1,13,bytes.data(),sizeof(gx)),"unknown bool/padding field");
    }
    bad=gx;bad.shaderConfig.tevStageCount=17;need(!C::recipeShape(1,13,&bad,sizeof(bad)),"stage bound");
    bad=gx;bad.shaderConfig.numIndStages=5;need(!C::recipeShape(1,13,&bad,sizeof(bad)),"indirect bound");
    bad=gx;bad.depthFunc=static_cast<GXCompare>(99);need(!C::recipeShape(1,13,&bad,sizeof(bad)),"descriptor enum bound");
    sqlite3* db=nullptr;need(sqlite3_open((root/"recipe.db").c_str(),&db)==SQLITE_OK,"recipe open");
    need(C::identity(db,true),"recipe identity");sql(db,RecipeTable);need(C::ensureRecipeIntegrity(db),"integrity schema");
    need(writeRecipe(db,1,hash,13,&gx,sizeof(gx),0),"sealed recipe write");
    C::RecipeRows rows(db);C::Statement query(db,"SELECT type,hash,config_version,config_size,config,first_frame_used FROM pipeline_cache");
    need(query && sqlite3_step(query.p)==SQLITE_ROW && C::admitRecipe(rows,query.p),"production typed row admitted");sqlite3_reset(query.p);
    sql(db,"UPDATE pipeline_cache SET config_size=config_size-1");
    need(sqlite3_step(query.p)==SQLITE_ROW && !C::admitRecipe(rows,query.p),"wrong logical length rejected");sqlite3_reset(query.p);
    sql(db,"UPDATE pipeline_cache SET config_size=2772; UPDATE petari_recipe_integrity SET digest='bad'");
    need(sqlite3_step(query.p)==SQLITE_ROW && !C::admitRecipe(rows,query.p),"checksum rejects intact-size corrupted row");sqlite3_reset(query.p);
    sql(db,"UPDATE petari_cache_compatibility SET identity='foreign'");need(!C::identity(db,true),"no foreign relabelling");
    // Statements finalize before sqlite3_close_v2's deferred disposal.
    sqlite3_close_v2(db);
}
static void migrationTests(const fs::path& root) {
    const auto dir=root/"migration";fs::create_directory(dir);
    const auto legacy=dir/("pipeline_cache."+std::string(64,'a')+".db");sqlite3* old=nullptr;
    need(sqlite3_open(legacy.c_str(),&old)==SQLITE_OK,"legacy open");sql(old,RecipeTable);sql(old,"CREATE TABLE petari_cache_build(build_key TEXT); INSERT INTO petari_cache_build VALUES('"+std::string(64,'a')+"')");
    auto gx=validGX();const auto hash=aurora::xxh3_hash(gx,1);
    { C::Statement s(old,"INSERT INTO pipeline_cache VALUES(1,?,13,2772,?,10)");sqlite3_bind_int64(s.p,1,hash);sqlite3_bind_blob(s.p,2,&gx,sizeof(gx),SQLITE_TRANSIENT);need(sqlite3_step(s.p)==SQLITE_DONE,"legacy recipe"); }
    sql(old,"INSERT INTO pipeline_cache VALUES(1,99,999,1,x'00',0)");sqlite3_close(old);
    const auto before=fileHash(legacy);std::ofstream(dir/("dawn_cache."+std::string(64,'a')+".db"))<<"unproven opaque blobs";
    sqlite3* db=nullptr;need(sqlite3_open((dir/C::filename("pipeline_cache")).c_str(),&db)==SQLITE_OK,"migration target");
    need(C::identity(db,true),"migration identity");sql(db,RecipeTable);need(C::ensureRecipeIntegrity(db),"migration integrity");
    auto writer=[&](int t,uint64_t h,int v,const void* p,size_t n,uint32_t rank){return writeRecipe(db,t,h,v,p,n,rank);};
    need(C::migrateRecipes(db,dir,writer),"migration commit");need(C::migrateRecipes(db,dir,writer),"migration idempotent");
    {C::Statement s(db,"SELECT files,accepted,rejected FROM petari_recipe_migration");need(sqlite3_step(s.p)==SQLITE_ROW && sqlite3_column_int(s.p,0)==1 && sqlite3_column_int(s.p,1)==1 && sqlite3_column_int(s.p,2)==1,"migration row counts");}
    need(fileHash(legacy)==before,"legacy source bytes untouched");
    need(fs::file_size(dir/("dawn_cache."+std::string(64,'a')+".db"))==21,"legacy Dawn untouched");
    need(!C::emptyRecipes(db),"seed not reimported into warmed DB");sqlite3_close(db);
}
static void checkSeed(const char* path) {
    sqlite3* db=nullptr;need(sqlite3_open_v2(path,&db,SQLITE_OPEN_READONLY,nullptr)==SQLITE_OK,"private stamped seed open");
    need(C::identity(db) && C::recipeContainer(db,true),"stamped container identity/schema");
    unsigned rows=0;{
        C::RecipeRows validation(db);C::Statement s(db,"SELECT type,hash,config_version,config_size,config,first_frame_used FROM pipeline_cache");need(bool(s),"stamped query");
        int step;while((step=sqlite3_step(s.p))==SQLITE_ROW) {
            if(!C::admitRecipe(validation,s.p)) {
                std::fprintf(stderr,"stamped seed rejected hash=%016llx type=%d version=%d size=%d\n",(unsigned long long)sqlite3_column_int64(s.p,1),sqlite3_column_int(s.p,0),sqlite3_column_int(s.p,2),sqlite3_column_int(s.p,3));
                throw std::runtime_error("Python stamp / native typed-admission mismatch");
            }++rows;
        }need(step==SQLITE_DONE && rows>0,"stamped complete rows");
    }sqlite3_close(db);std::printf("Python-stamped native recipes accepted=%u PASS\n",rows);
}
int main(int argc,char** argv) try {
    if(argc==3 && std::string(argv[1])=="--check-private-seed") {checkSeed(argv[2]);return 0;}
    need(argc==1,"unexpected args");
    char pattern[]="/tmp/petari-cache-compat-XXXXXX";const auto* path=mkdtemp(pattern);need(path,"private root");
    const fs::path root(path);fs::create_directory(root/"home");setenv("HOME",(root/"home").c_str(),1);
    shaTests();identityTests();artifactTests(root);recipeTests(root);migrationTests(root);
    fs::remove_all(root);std::puts("Production cache compatibility, typed decoder, corruption, full keys, context and read-only migration PASS");return 0;
} catch(const std::exception& e){std::fprintf(stderr,"FAIL: %s\n",e.what());return 1;}
