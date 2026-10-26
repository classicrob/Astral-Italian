// Recorder state machine only: no SDK, scene pointer, GUI, cache or app launch.
#include "record_normal_observation.hpp"
#include "tape_script_record.hpp"
#include <cstdio>
#include <stdexcept>
namespace P=PetariNative::App::TapePhase;
using O=PetariNative::App::TapeScriptRecord::IslandNormalObservation;
using T=PetariNative::App::TapeScriptRecord::Tail;
static void need(bool v,const char* why) {if(!v)throw std::runtime_error(why);}
static P::State first() {
    P::State s;s.inputFrame=1000;s.scene="Game";s.stage="IslandFleetGalaxy";
    s.scenario=1;s.kind="controller-normal";s.firstNormal=true;s.valid=true;return s;
}
int main() try {
    O observation;P::State s;
    need(observation.step(s,true)==O::None && observation.count()==0,"no startup packet counted");
    s=first();s.stage="FileSelect";need(observation.step(s,true)==O::None,"frontend Normal is not target coverage");
    s=first();need(observation.step(s,true)==O::Began,"start at exact first Normal marker");
    for(std::uint64_t n=2;n<=O::Packets;++n) {
        ++s.inputFrame;s.firstNormal=false;
        need(observation.step(s,true)==(n==O::Packets?O::Complete:O::None),"only packet14400 completes");
    }
    need(observation.count()==14400 && observation.valid()==14400 && observation.last()-observation.first()==14399,"exact prefix coverage");
    need(observation.step(s,true)==O::None,"completion emits no duplicate queue");
    T tail;need(tail.queue(),"completion queues tail, not power");
    need(tail.step(1,false,true)==T::None,"no tail outside control seam");
    need(tail.step(2,true,true)==T::Begin && tail.began(3,true),"actual tail begins at control seam");
    need(tail.step(180000000002ull,true,true)==T::None,"no power before180 actual seconds");
    need(tail.step(180000000003ull,false,true)==T::None,"no out-of-seam power");
    need(tail.step(180000000003ull,true,true)==T::Power,"real power permission after tail at seam");
    for(int kind=0;kind<8;++kind) {
        O bad;s=first();need(bad.step(s,true)==O::Began,"negative starts");
        ++s.inputFrame;s.firstNormal=false;
        if(kind==0)--s.inputFrame;
        if(kind==1)++s.inputFrame;
        if(kind==2)s.firstNormal=true;
        if(kind==3)s.stage="MarioFaceShipGalaxy";
        if(kind==4)s.kind="scenario-foreground";
        if(kind==5)s.scenario=2;
        if(kind==6)s.dead=true;
        need(bad.step(s,kind!=7)==O::Reject && bad.count()==1 && bad.error(),"duplicate/gap/restart/stage/overlay/scenario/death/input fails closed");
        need(bad.step(first(),true)==O::Reject,"reject never heals or retries");
    }
    {O bad;s=first();s.firstNormal=false;need(bad.step(s,true)==O::Reject,"missing first Normal cannot anchor");}
    {O bad;s=first();s.valid=false;
     for(std::uint64_t n=1;n<=O::Packets;++n) {
        if(n>1){++s.inputFrame;s.firstNormal=false;}
        need(bad.step(s,true)==(n==1?O::Began:n==O::Packets?O::Reject:O::None),"no valid target cannot complete");
     }}
    std::puts("Recorder Island14400 exact Normal packets, coverage, tail/control seam and negatives PASS");return 0;
} catch(const std::exception& e){std::fprintf(stderr,"FAIL: %s\n",e.what());return 1;}
