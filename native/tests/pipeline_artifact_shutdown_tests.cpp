// Exercise the production terminal operation with _Exit (no destructors),
// repeated/concurrent callers, and callbacks arriving after the final report.
// All files and HOME are private; no device or user's cache is opened.
#include "pipeline_artifact_cache.hpp"
#include <filesystem>
#include <fstream>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <sys/wait.h>
#include <unistd.h>

static void need(bool value,const char* why) { if(!value) throw std::runtime_error(why); }
static std::string read(const std::filesystem::path& path) {
    std::ifstream file(path); return {std::istreambuf_iterator<char>(file),{}};
}
int main() try {
    namespace C=PetariPipelineCache;
    char pattern[]="/tmp/petari-artifact-exit-XXXXXX";
    const char* dir=mkdtemp(pattern);need(dir,"private root");
    const std::filesystem::path root(dir),dbPath=root/"artifact.db",reportPath=root/"report.log";
    std::filesystem::create_directory(root/"home");setenv("HOME",(root/"home").c_str(),1);
    const std::string key="full-Dawn-key",value="compiled-shader-bytes",context="private-context";
    const pid_t child=fork();need(child>=0,"fork");
    if(child==0) {
        C::ArtifactCache cache;
        FILE* report=std::fopen(reportPath.c_str(),"w");need(report,"report");
        need(cache.open(dbPath.string(),context),"open");
        need(cache.load(key.data(),key.size(),nullptr,0)==0,"cold miss");
        need(cache.store(key.data(),key.size(),value.data(),value.size()),"store");
        std::string out(value.size(),'!');
        need(cache.load(key.data(),key.size(),nullptr,0)==value.size(),"size probe");
        need(cache.load(key.data(),key.size(),out.data(),out.size())==value.size() && out==value,"hit");
        // The production cache_shutdown serializes this operation and the
        // callbacks with cache_mutex. Either exit caller may arrive first.
        std::mutex mutex;
        auto finish=[&] { std::lock_guard lock(mutex);cache.finish(report); };
        std::thread a(finish),b(finish);a.join();b.join();
        need(!cache.open(dbPath.string(),context),"late callback cannot reopen");
        need(!cache.open((root/"late.db").string(),context),"late callback cannot create a DB");
        need(cache.load(key.data(),key.size(),out.data(),out.size())==0,"late read refused");
        need(!cache.store(key.data(),key.size(),value.data(),value.size()),"late store refused");
        need(!cache.finish(report),"terminal finish remains idempotent");
        need(cache.hits==1 && cache.misses==1 && cache.stores==1 && cache.rejected==0,"counters frozen");
        std::_Exit(0); // Match Host::leave: no cache/file destructors or atexit.
    }
    int status=0;need(waitpid(child,&status,0)==child && WIFEXITED(status) && WEXITSTATUS(status)==0,"normal child exit");
    need(read(reportPath)=="[pipeline artifacts] hits=1 misses=1 rejected=0 stored=1\n[pipeline artifacts close] persistence=0 checkpoint=0 close=0\n","one flushed report and successful close before _Exit");
    need(!std::filesystem::exists(dbPath.string()+"-wal") && !std::filesystem::exists(dbPath.string()+"-shm"),"clean DB close before _Exit");
    need(!std::filesystem::exists(root/"late.db"),"no post-finish DB created");
    {
        C::ArtifactCache reopened;need(reopened.open(dbPath.string(),context),"next process can reopen");
        std::string out(value.size(),'!');
        need(reopened.load(key.data(),key.size(),out.data(),out.size())==value.size() && out==value,"committed bytes survive _Exit");
        reopened.close();need(reopened.open(dbPath.string(),context),"ordinary close/reopen remains supported");
    }
    {
        C::ArtifactCache unopened;FILE* report=std::tmpfile();need(report,"empty report");
        need(unopened.finish(report) && !unopened.finish(report),"uninitialized shutdown once");
        need(!unopened.open((root/"never.db").string(),context),"uninitialized shutdown terminal");
        std::rewind(report);char line[128]{};need(std::fgets(line,sizeof(line),report),"zero report present");
        need(std::string(line)=="[pipeline artifacts] hits=0 misses=0 rejected=0 stored=0\n","one zero report");
        need(std::fgets(line,sizeof(line),report) && std::string(line)=="[pipeline artifacts close] persistence=0 checkpoint=0 close=0\n" && std::fgetc(report)==EOF,"one successful empty close");
        std::fclose(report);need(!std::filesystem::exists(root/"never.db"),"no DB on uninitialized shutdown");
    }
    std::filesystem::remove_all(root);
    std::puts("Production artifact finish before _Exit, durable close, exactly-once report and late callback refusal PASS");
    return 0;
} catch(const std::exception& e) { std::fprintf(stderr,"FAIL: %s\n",e.what());return 1; }
