// Recorder lifecycle against real CSV/KPAD writers. Normal _Exit flushes every seam; SIGKILL leaves its checkpoint.
#include "tape_record.hpp"
#include "route_record_writer.hpp"
#include <petari/kpad_record.hpp>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
namespace fs = std::filesystem;
namespace TR = PetariNative::App::TapeRecord;
namespace KP = PetariNative::Input::KpadRecord;
void require(bool ok, const char* why) { if (!ok) { std::fprintf(stderr,"FAIL: %s\n",why); std::exit(1); } }
std::string contents(const fs::path& p) { std::ifstream f(p); std::ostringstream out; out<<f.rdbuf(); return out.str(); }
std::uint64_t be64(std::istream& in) { std::uint64_t v=0; for(int i=0;i<8;++i) v=(v<<8)|static_cast<unsigned char>(in.get()); return v; }
int main(int argc,char** argv) {
    if (argc==3) {
        setenv("PETARI_TAPE_RECORD",argv[2],1);
        PetariNative::App::RouteRecordWriter writer;
        require(writer.open(TR::routePath(),false),"open CSV");
        TR::Seam seam; seam.scene="Game"; seam.stage="TestGalaxy"; seam.scenario=1; seam.initEnd=true; seam.seedValid=true;
        for(unsigned i=0;i<307;++i) {
            if(i) KP::read(0,120,0,0,0,0,nullptr);
            PetariNative::App::RouteRecordFrame row; row.stage="TestGalaxy"; row.valid=true; row.x=i;
            writer.write(row); TR::seam(seam);
        }
        if(std::string(argv[1])=="--killed") { ::kill(::getpid(),SIGKILL); std::_Exit(2); }
        if(std::string(argv[1])=="--atexit") return 0;
        TR::finish();
        const auto before=contents(fs::path(argv[2])/"route.tape.json");
        TR::finish(); TR::seam(seam); // neither a second close nor a later seam may reopen/truncate the tape
        require(before==contents(fs::path(argv[2])/"route.tape.json"),"idempotent finalization");
        std::_Exit(0); // the app's normal power exit uses this, without running destructors or atexit
    }
    const auto base=fs::temp_directory_path()/("tape_record_tests_"+std::to_string(::getpid()));
    for(const char* mode:{"--normal","--atexit","--killed"}) {
        const auto dir=base/mode;fs::create_directories(dir);
        const pid_t child=::fork();require(child>=0,"fork");
        if(child==0) { ::execl(argv[0],argv[0],mode,dir.c_str(),nullptr); std::_Exit(127); }
        int status=0;require(::waitpid(child,&status,0)==child,"wait for child");
        const bool killed=std::string(mode)=="--killed";
        require(killed ? WIFSIGNALED(status)&&WTERMSIG(status)==SIGKILL : WIFEXITED(status)&&WEXITSTATUS(status)==0,"child exit");
        const auto text=contents(dir/"route.tape.json");
        require(text.find(killed ? "\"complete\": false" : "\"complete\": true")!=std::string::npos,"complete reflects normal exit only");
        require(text.find(killed ? "\"seams\": 300," : "\"seams\": 307,")!=std::string::npos,"sidecar covers final seams or last checkpoint");
        std::ifstream csv(dir/"route.csv");std::string line;unsigned lines=0;while(std::getline(csv,line))++lines;
        require(lines==(killed?301u:308u),"CSV final partial block is flushed only on normal exit");
        std::ifstream kpd(dir/"route.kpd",std::ios::binary);char magic[8];kpd.read(magic,8);
        require(be64(kpd)==(killed?301u:308u),"KPD frame header includes the open loop, as specified");
        require(be64(kpd)==(killed?299u:306u),"KPD final read count or checkpoint count");
    }
    fs::remove_all(base);
    std::puts("recorder normal _Exit, atexit, idempotence and killed-checkpoint tests passed");
}
