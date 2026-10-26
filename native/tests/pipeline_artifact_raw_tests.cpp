// Raw-callback regression: validate logical size vs blob length BEFORE either
// Dawn's size probe or copy. This target deliberately has no zstd or Aurora link.
#include "pipeline_artifact_cache.hpp"
#include <filesystem>
#include <stdexcept>
#include <unistd.h>
static void need(bool v,const char* s){if(!v)throw std::runtime_error(s);}
int main() try {
    namespace C=PetariPipelineCache;
    char pattern[]="/tmp/petari-raw-cache-XXXXXX";const char* dir=mkdtemp(pattern);need(dir,"private root");
    std::filesystem::path root(dir);std::filesystem::create_directory(root/"home");setenv("HOME",(root/"home").c_str(),1);
    C::ArtifactCache cache;const auto path=(root/"raw.db").string();need(cache.open(path,"private-fixture-context"),"open");
    const std::string key="actual-full-key",value="four";
    need(cache.store(key.data(),key.size(),value.data(),value.size()),"raw store");char out[8]{};
    need(cache.load(key.data(),key.size(),nullptr,0)==4 && cache.load(key.data(),key.size(),out,4)==4,"raw positive");
    sqlite3* db=nullptr;need(sqlite3_open(path.c_str(),&db)==SQLITE_OK,"corrupt fixture");
    for(const auto* change:{"UPDATE artifacts SET value=x'01'","UPDATE artifacts SET size=3","UPDATE artifacts SET size=5","UPDATE artifacts SET encoding=1"}) {
        need(cache.store(key.data(),key.size(),value.data(),value.size()),"restore fixture");
        need(sqlite3_exec(db,change,nullptr,nullptr,nullptr)==SQLITE_OK,"corrupt");std::fill(std::begin(out),std::end(out),'!');
        need(cache.load(key.data(),key.size(),nullptr,0)==0,"size probe must reject");
        need(cache.load(key.data(),key.size(),out,4)==0,"copy must reject");need(out[0]=='!' && out[7]=='!',"no output write");
    }
    sqlite3_close(db);cache.close();std::filesystem::remove_all(root);
    std::puts("Production raw Dawn callback size/length and unsupported encoding fail closed PASS");return 0;
} catch(const std::exception& e){std::fprintf(stderr,"FAIL: %s\n",e.what());return 1;}
