// Game-free session driven through the REAL native power callback. No GUI.
#include "tape_play.hpp"
#include <petari/platform/power.hpp>
#include <revolution/os.h>
#include <revolution/kpad.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <unistd.h>
namespace TP=PetariNative::App::TapePlay;
namespace PH=PetariNative::App::TapePhase;
namespace H=PetariNative::Headless;
namespace fs=std::filesystem;
extern "C" void __OSThreadInit(void);
namespace {
void check(bool b,const char* why) { if(!b) {std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);} }
void number(std::ostream& o,std::uint64_t v,unsigned n) {for(unsigned i=n;i;--i)o.put(static_cast<char>(v>>(8*(i-1))));}
std::uint32_t bits(float f) {std::uint32_t b;std::memcpy(&b,&f,4);return b;}
std::string slurp(const fs::path& p) {std::ifstream f(p);std::ostringstream s;s<<f.rdbuf();return s.str();}
void save(const fs::path& p,const std::string& s) {std::ofstream(p)<<s;}
PH::State state(unsigned frame,unsigned canonical) {
    PH::State s;s.inputFrame=frame;
    if(canonical>=3 && canonical<8) {s.kind="controller-normal";s.scene="Game";s.stage="WorldMap01Galaxy";s.scenario=1;
        s.firstNormal=canonical==3;s.valid=true;s.rngObserved=true;s.rng=99;
        s.position={bits(canonical<7?canonical-0.5f:5.5f),0,0};}
    return s;
}
std::string event(unsigned id,const char* kind,unsigned row,bool ack) {
    std::ostringstream o;o<<"{\"id\":"<<id<<",\"kind\":\""<<kind<<"\",\"phase_id\":0,\"after_route_row\":"<<row
        <<",\"before_input_frame\":"<<row+1<<",\"boundary\":\""<<PetariNative::App::TapePower::kBoundary
        <<"\",\"callback_observed\":"<<(ack?"true":"false")<<",\"neutral_elapsed_seconds\":1}";return o.str();
}
void tape(const fs::path& p,unsigned powerRow=5) {
    fs::create_directories(p);
    auto marker=state(3,3);
    std::ostringstream m;m<<"{\"schema\":2,\"complete\":true,\"seams\":12,\"marker_protocol\":\""<<PH::kProtocol
        <<"\",\"marker_error\":\"\",\"launch\":{\"stage\":\"WorldMap01Galaxy\",\"scenario\":1},\"input_phases\":["
        <<"{\"id\":0,\"first_input_frame\":3,\"end_input_frame_exclusive\":8,\"end_kind\":\"phase-transition\",\"marker\":"
        <<PH::stateJson(marker)<<",\"checkpoints\":[]}],\"session_control_protocol\":\""<<PetariNative::App::TapePower::kProtocol
        <<"\",\"session_control_error\":\"\",\"session_events\":["<<event(0,"neutral-tail-begin",3,false)<<','
        <<event(1,"power-request",powerRow,true)<<"]}";save(p/"route.tape.json",m.str());
    std::ofstream csv(p/"route.csv");csv<<"frame,stage,scenario,valid,x,y,z,grounded,dead\n";
    for(unsigned f=0;f<12;++f)csv<<f<<",WorldMap01Galaxy,1,"<<(f<9?1:0)<<','<<(f<6?f+0.5f:f<9?5.5f:0)<<",0,0,0,0\n";
    std::ofstream k(p/"route.kpd",std::ios::binary);k<<"SMG2KPD1";number(k,13,8);number(k,13,8);
    for(unsigned f=0;f<13;++f) {
        number(k,f,8);number(k,0,4);number(k,0,4);number(k,120,4);
        const bool detached=f>=9;
        number(k,detached?0:1,4);number(k,detached?0xfffffffe:0,4);
        number(k,detached?0xffffffff:0,4);number(k,detached?253:1,4);number(k,1,4); // slot zero retained even on return 0
        H::Sample sample{};auto word=[&](unsigned at,unsigned value){for(unsigned i=0;i<4;++i)sample[at+i]=value>>(24-8*i);};
        word(16,0xbf800000);for(unsigned at:{24u,52u,84u})word(at,0x3f800000);
        sample[92]=detached?253:1;sample[93]=detached?255:0;sample[95]=detached?0:5;
        if (!detached) { word(108,0xbf800000);word(116,0x3f800000); }
        k.write(reinterpret_cast<const char*>(sample.data()),sample.size());
    }
}
bool powerFlag=false;unsigned calls=0;
void realPowerCallback() {check(OSDisableInterrupts()==FALSE,"real callback executes under interrupt lock");powerFlag=true;++calls;}
// This tiny actor/update model deliberately freezes only AFTER the real power
// callback. A coherently shifted metadata event therefore changes state rows.
bool drive(const fs::path& p,unsigned delay=0,bool omit=false,bool falseAck=false,bool earlyGate=false,bool earlyExit=false,bool duplicateAck=false,int shutdownReadFault=0) {
    powerFlag=false;calls=0;OSSetPowerCallback(realPowerCallback);
    TP::Session session(p,TP::Session::Mode::WholeSession);
    float x=0;unsigned resetFrames=0;
    for(unsigned f=0;f<12+delay;++f) {
        const unsigned canonical=f>=delay?f-delay:0;
        const auto start=session.beforeRead(state(f,canonical));
        if(start.kind==TP::Step::Kind::Start)check(session.acknowledgeSeed(99),"marker RNG acknowledged");
        KPADStatus sample[120]{};s32 error=9;
        if (!(shutdownReadFault==1 && canonical==9)) {
            const auto returned=session.read(0,120,sample,&error);
            if (canonical>=9 && !session.gaveUp()) {
                check(returned==0 && error==-2,"shutdown returns exact recorded detach, not bootstrap neutral");
                const auto* connection=session.connection(0);
                check(connection && connection->probeError==-1 && connection->device==253,"shutdown connection exact");
            }
            if (shutdownReadFault==2 && canonical==9) session.read(0,120,sample,&error);
        }
        if(powerFlag)++resetFrames;else x=canonical+0.5f;
        TP::SeamState row;row.stage="WorldMap01Galaxy";row.scenario=1;row.valid=resetFrames<4;
        row.x=row.valid?x:0;row.rngObserved=true;row.rngSeed=99;
        const auto step=session.seam(row);
        if(step.powerRequest && !omit) {
            if(earlyGate)session.rejectEarlyPower();
            const bool expected=session.beforePowerRequest();
            PetariNative::Platform::Power::pressPowerButton();
            if(expected) {
                session.acknowledgePower(falseAck?false:powerFlag);
                if(duplicateAck)session.acknowledgePower(powerFlag);
            }
        }
        if(earlyExit && f==10+delay) {session.normalExit();return session.gaveUp();}
        if(session.gaveUp())return false;
    }
    session.normalExit();
    const auto report=session.reportJson();
    return session.finished() && !session.gaveUp() && calls==1 &&
        report.find("\"normal_exit_verified\":true")!=std::string::npos &&
        report.find("\"shutdown_rows_compared\":6")!=std::string::npos;
}
void rejected(const std::string& meta,const char* why) {
    bool failed=false;try {TP::parseSidecar(meta);}catch(const H::ReplayError&){failed=true;}check(failed,why);
}
void replace(std::string& s,const std::string& from,const std::string& to) {
    const auto at=s.find(from);check(at!=std::string::npos,"negative fixture text exists");s.replace(at,from.size(),to);
}
}
int main() {
    __OSThreadInit();
    const auto base=fs::temp_directory_path()/("tape-power-tests-"+std::to_string(getpid()));
    tape(base/"correct");check(drive(base/"correct"),"real callback preserves every state including shutdown");
    check(drive(base/"correct",2),"same existing marker offset anchors event and input/state together");
    tape(base/"early",4);check(!drive(base/"early"),"one-loop early event changes a strict state row");
    tape(base/"late",6);check(!drive(base/"late"),"one-loop late event changes a strict state row");
    check(!drive(base/"correct",0,true),"missing delivery cannot pass");
    check(!drive(base/"correct",0,false,true),"unobserved real callback cannot pass");
    check(!drive(base/"correct",0,false,false,true),"early wall-time power event fails without padding");
    check(drive(base/"correct",0,false,false,false,true),"normal exit before last teardown row fails");
    check(!drive(base/"correct",0,false,false,false,false,true),"duplicate callback acknowledgement fails");
    check(!drive(base/"correct",0,false,false,false,false,false,1),"omitted shutdown raw read fails");
    check(!drive(base/"correct",0,false,false,false,false,false,2),"extra shutdown raw read fails");
    const auto original=slurp(base/"correct"/"route.tape.json");
    auto bad=original;replace(bad,event(1,"power-request",5,true),event(1,"power-request",5,false));rejected(bad,"unacknowledged event rejected");
    bad=original;replace(bad,event(0,"neutral-tail-begin",3,false)+','+event(1,"power-request",5,true),"");rejected(bad,"missing power event rejected");
    bad=original;replace(bad,"\"before_input_frame\":6","\"before_input_frame\":7");rejected(bad,"ambiguous boundary rejected");
    bad=original;replace(bad,"\"after_route_row\":5","\"after_route_row\":5,\"after_route_row\":5");rejected(bad,"duplicate JSON ordering field rejected");
    bad=original;replace(bad,event(1,"power-request",5,true),event(1,"power-request",5,true)+','+event(1,"power-request",5,true));rejected(bad,"duplicate event rejected");
    bad=original;replace(bad,PetariNative::App::TapePower::kBoundary,"host-time-guess");rejected(bad,"unknown boundary rejected");
    bad=original;replace(bad,PetariNative::App::TapePower::kProtocol,"unknown-v2");rejected(bad,"unknown version rejected");
    TP::Observation observation;observation.requestedSeconds=90;observation.minimumPostSeconds=180;
    observation.update(49,100,true);observation.update(228.999,200,true);check(!observation.completed,"long neutral tail remains below minimum");
    observation.update(229,201,true);check(observation.completed,"at least180s neutral tail plus unchanged90s window");
    fs::remove_all(base);
    std::puts("power events: real callback, exact shutdown, early/late/missing/ambiguous/clock negatives PASS");
}
