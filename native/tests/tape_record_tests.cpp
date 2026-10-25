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
        if(std::string(argv[1])=="--native-policy")TR::setNeutralTailPolicy(PetariNative::App::TapeNeutral::Policy::NativeControls);
        PetariNative::App::RouteRecordWriter writer;
        require(writer.open(TR::routePath(),false),"open CSV");
        TR::Seam seam; seam.scene="Game"; seam.stage="TestGalaxy"; seam.scenario=1; seam.initEnd=true; seam.seedValid=true;
        KPADStatus empty{}; // the tape preserves slot zero even when KPAD returns zero samples
        for(unsigned i=0;i<307;++i) {
            if(i) {
                PetariNative::App::TapePhase::State state;
                state.inputFrame=i; state.scene="Game"; state.stage="TestGalaxy"; state.scenario=1;
                state.kind=i<205 ? "controller-normal" : "none";
                state.firstNormal=i==1; state.rngObserved=true; state.rng=99;
                TR::beforeRead(state);
                KP::read(0,120,0,0,0,0,&empty);
            }
            PetariNative::App::RouteRecordFrame row; row.stage="TestGalaxy"; row.valid=true; row.x=i;
            TR::beginControlSeam();
            const std::string mode=argv[1];
            if (i==104) TR::neutralTailBegin();
            if (i==105 && mode=="--policy-late") TR::setNeutralTailPolicy(PetariNative::App::TapeNeutral::Policy::NativeControls);
            if (i==204 && mode!="--power-missing") {
                if (mode=="--power-ambiguous") {
                    TR::endControlSeam();TR::powerRequest();TR::beginControlSeam();
                } else if (TR::powerRequest()) TR::acknowledgePower(mode!="--power-unacknowledged");
                if (mode=="--power-duplicate") TR::powerRequest();
            }
            writer.write(row); TR::seam(seam);TR::endControlSeam();
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
    for(const char* mode:{"--normal","--native-policy","--policy-late","--atexit","--killed","--power-missing","--power-ambiguous","--power-duplicate","--power-unacknowledged"}) {
        const auto dir=base/mode;fs::create_directories(dir);
        const pid_t child=::fork();require(child>=0,"fork");
        if(child==0) { ::execl(argv[0],argv[0],mode,dir.c_str(),nullptr); std::_Exit(127); }
        int status=0;require(::waitpid(child,&status,0)==child,"wait for child");
        const bool killed=std::string(mode)=="--killed";
        require(killed ? WIFSIGNALED(status)&&WTERMSIG(status)==SIGKILL : WIFEXITED(status)&&WEXITSTATUS(status)==0,"child exit");
        const auto text=contents(dir/"route.tape.json");
        require(text.find(std::string("\"neutral_tail_policy\":\"")+(std::string(mode)=="--native-policy"?"native-neutral-controls-v1":"single-bootstrap-v1")+'"')!=std::string::npos,
            "recorder persists selected policy, defaults to unchanged legacy");
        require(text.find(killed ? "\"complete\": false" : "\"complete\": true")!=std::string::npos,"complete reflects normal exit only");
        require(text.find(killed ? "\"seams\": 300," : "\"seams\": 307,")!=std::string::npos,"sidecar covers final seams or last checkpoint");
        require(text.find("\"schema\": 2")!=std::string::npos && text.find("\"marker_error\": \"\"")!=std::string::npos,"new recordings have validated schema2 markers");
        require(text.find("\"first_input_frame\": 1")!=std::string::npos && text.find("\"end_input_frame_exclusive\": 205")!=std::string::npos,"phase entry and exit are explicit read loops");
        const bool invalidPower=std::string(mode).find("--power-")==0 || std::string(mode)=="--policy-late";
        const auto control=text.find("\"session_control_error\":\"\"");
        require(invalidPower ? control==std::string::npos : control!=std::string::npos,"missing/ambiguous power ordering fails closed");
        if(!invalidPower)require(text.find("\"after_route_row\":204")!=std::string::npos &&
            text.find("\"before_input_frame\":205")!=std::string::npos && text.find("\"callback_observed\":true")!=std::string::npos,
            "real boundary and callback acknowledgement retained");
        std::ifstream csv(dir/"route.csv");std::string line;unsigned lines=0;while(std::getline(csv,line))++lines;
        require(lines==(killed?301u:308u),"CSV final partial block is flushed only on normal exit");
        if(!killed) {
            std::ifstream complete(dir/"route.csv");std::getline(complete,line);unsigned row=0;
            while(std::getline(complete,line)) {
                require(line.rfind(std::to_string(row)+",",0)==0,"finish neither inserts nor renumbers a row");
                std::istringstream fields(line);std::string value;
                for(unsigned column=0;column<=5;++column)require(static_cast<bool>(std::getline(fields,value,',')),"route x column exists");
                require(std::stof(value)==static_cast<float>(row),"finish never resamples/changes the saved actor x");++row;
            }
            require(row==307,"every original row retained, no finish sample");
        }
        std::ifstream kpd(dir/"route.kpd",std::ios::binary);char magic[8];kpd.read(magic,8);
        require(be64(kpd)==(killed?301u:308u),"KPD frame header includes the open loop, as specified");
        require(be64(kpd)==(killed?299u:306u),"KPD final read count or checkpoint count");
    }
    fs::remove_all(base);
    std::puts("recorder normal _Exit, atexit, idempotence and killed-checkpoint tests passed");
}
