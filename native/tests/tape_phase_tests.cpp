// Distinct adjacent-frame raw samples make comparison-only reanchoring fail.
#include "tape_play.hpp"
#include <revolution/kpad.h>
#include <algorithm>
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
void require(bool b,const char* why) { if(!b) { std::fprintf(stderr,"FAIL: %s\n",why); std::exit(1); } }
void number(std::ostream& o,std::uint64_t v,unsigned n) { for(unsigned i=n;i;--i)o.put(static_cast<char>(v>>(8*(i-1)))); }
std::uint32_t bits(float f) { std::uint32_t b;std::memcpy(&b,&f,4);return b; }
PH::State marker(unsigned canonical,unsigned actual,bool first=true,const char* kind="controller-normal",bool map=false) {
    PH::State s;s.inputFrame=actual;s.kind=kind;s.scene="Game";s.stage="WorldMap01Galaxy";s.scenario=1;
    s.firstNormal=first;s.valid=true;s.grounded=true;s.rngObserved=true;s.rng=99;
    s.position={bits(canonical-0.5f),bits(2),bits(3)};s.mapWait=map;return s;
}
TP::SeamState row(unsigned canonical) {
    TP::SeamState s;s.stage="WorldMap01Galaxy";s.scenario=1;s.valid=true;s.grounded=true;s.initEnd=true;
    s.x=canonical+0.5f;s.y=2;s.z=3;s.pointerObserved=true;s.pointerValid=1;s.pointerX=canonical*0.01f;
    s.rngObserved=true;s.rngSeed=99;
    return s;
}
struct Range { unsigned first,end;const char* kind;bool firstNormal,open,map; };
void tape(const fs::path& p,const std::vector<Range>& ranges) {
    fs::create_directories(p);
    const unsigned seams=ranges.back().open ? ranges.back().end : 20;
    std::ofstream meta(p/"route.tape.json");
    meta << "{\"schema\":2,\"complete\":true,\"seams\":" << seams << ",\"marker_protocol\":\"" << PH::kProtocol
         << "\",\"marker_error\":\"\",\"launch\":{\"stage\":\"WorldMap01Galaxy\",\"scenario\":1},\"play_ready_seams\":[],\"input_phases\":[";
    for(unsigned i=0;i<ranges.size();++i) {
        const auto& r=ranges[i];auto s=marker(r.first,r.first,r.firstNormal,r.kind,r.map && r.first>=5);
        meta << (i?",":"") << "{\"id\":" << i << ",\"first_input_frame\":" << r.first
             << ",\"end_input_frame_exclusive\":" << r.end << ",\"end_kind\":\"" << (r.open?"recording-prefix":"phase-transition")
             << "\",\"marker\":" << PH::stateJson(s) << ",\"checkpoints\":[";
        if(r.map) {
            const auto at=std::max(5u,r.first);auto cp=marker(at,at,at==r.first && r.firstNormal,r.kind,true);
            meta << "{\"kind\":\"map-wait\",\"state\":" << PH::stateJson(cp) << '}';
        }
        meta << "]}";
    }
    meta << "]}";meta.close();
    std::ofstream route(p/"route.csv");route << "frame,stage,scenario,valid,x,y,z,grounded,dead\n";
    for(unsigned i=0;i<seams;++i)route << i << ",WorldMap01Galaxy,1,1," << i << ".5,2,3,1,0\n";
    std::ofstream k(p/"route.kpd",std::ios::binary);k << "SMG2KPD1";number(k,seams+1,8);number(k,seams+1,8);
    for(unsigned f=0;f<=seams;++f) {
        bool active=false;for (const auto& r:ranges)active|=f>=r.first && f<r.end;
        const unsigned n=1+f%3;
        number(k,f,8);number(k,0,4);number(k,0,4);number(k,120,4);
        number(k,n,4);number(k,0,4);number(k,0,4);number(k,1,4);number(k,n,4);
        for(unsigned slot=0;slot<n;++slot) {
            H::Sample sample{};auto word=[&](unsigned at,std::uint32_t v) { for(unsigned j=0;j<4;++j)sample[at+j]=v>>(24-8*j); };
            word(0,active ? 0x100+f+slot : 0);word(32,active ? bits(f*0.01f) : 0);sample[92]=1;sample[94]=active ? 1:0;sample[95]=5;
            k.write(reinterpret_cast<const char*>(sample.data()),sample.size());
        }
    }
}
void wait(TP::Session& s,unsigned frames) { for(unsigned i=0;i<frames;++i)s.seam({}); }
void read(TP::Session& s,unsigned canonical) {
    KPADStatus b[120]{};std::int32_t err=9;const int n=s.read(0,120,b,&err);
    require(n==static_cast<int>(1+canonical%3) && err==0,"FIRST read count belongs to marker game loop");
    for(int i=0;i<n;++i)require(b[i].hold==0x100+canonical+i && bits(b[i].pos.x)==bits(canonical*0.01f),"all delivered buttons/pointers are exact, not adjacent frame");
}
int main() {
    const auto base=fs::temp_directory_path()/("phase-tests-"+std::to_string(::getpid()));
    tape(base/"map",{{3,7,"controller-normal",true,true,true}});
    for(unsigned delay:{0u,1u,3u}) {
        TP::Session s(base/"map",TP::Session::Mode::WholeSession);wait(s,3+delay);
        for(unsigned f=3;f<7;++f) {
            auto observed=marker(f,f+delay,f==3,"controller-normal",f>=5);
            const auto step=s.beforeRead(observed);
            if(f==3)require(step.kind==TP::Step::Kind::Start && s.acknowledgeSeed(99),"pre-read marker starts and verifies RNG");
            read(s,f);s.seam(row(f));
        }
        require(s.finished() && !s.gaveUp() && s.report()[0].rowsCompared==5,"shifted init/boot time preserves exact marker+four rows");
        require(s.reportJson().find(PH::kPolicy)!=std::string::npos,"new policy is labelled");
    }
    { TP::Session s(base/"map",TP::Session::Mode::WholeSession);wait(s,3);
      auto state=marker(3,3);state.position[0]++;s.beforeRead(state);require(s.gaveUp(),"sub-tolerance marker bit difference rejected"); }
    { TP::Session s(base/"map",TP::Session::Mode::WholeSession);wait(s,3);
      s.beforeRead(marker(3,3));KPADStatus b[120]{};std::int32_t err=0;s.read(0,120,b,&err);
      require(s.gaveUp(),"input cannot precede RNG acknowledgement"); }
    { TP::Session s(base/"map",TP::Session::Mode::WholeSession);wait(s,3);
      s.beforeRead(marker(3,3));require(!s.acknowledgeSeed(100),"wrong marker RNG readback rejected"); }
    { TP::Session s(base/"map",TP::Session::Mode::WholeSession);wait(s,3);
      s.beforeRead(marker(3,3));s.acknowledgeSeed(99);s.seam(row(3));require(s.gaveUp(),"missing first KPAD read rejected"); }
    { TP::Session s(base/"map",TP::Session::Mode::WholeSession);wait(s,3);
      s.beforeRead(marker(3,3));s.acknowledgeSeed(99);KPADStatus b[120]{};std::int32_t err;
      s.read(1,120,b,&err);require(s.gaveUp(),"wrong first channel rejected"); }
    { TP::Session s(base/"map",TP::Session::Mode::WholeSession);wait(s,3);
      KPADStatus b[120]{};std::int32_t err;s.read(1,120,b,&err);
      s.beforeRead(marker(3,3));require(s.gaveUp(),"marker after an earlier KPAD read is rejected even without a ledger"); }
    { TP::Session s(base/"map",TP::Session::Mode::WholeSession);wait(s,3);
      s.beforeRead(marker(3,3));s.acknowledgeSeed(99);read(s,3);auto wrong=row(3);wrong.pointerX=4*0.01f;
      s.seam(wrong);require(s.gaveUp(),"comparison-only pointer alignment rejected"); }
    { TP::Session s(base/"map",TP::Session::Mode::WholeSession);wait(s,3);
      s.beforeRead(marker(3,3));s.acknowledgeSeed(99);read(s,3);s.seam(row(3));
      s.beforeRead(marker(4,4,false,"controller-normal",true));require(s.gaveUp(),"map-wait at wrong relative loop rejected"); }
    { TP::Session s(base/"map",TP::Session::Mode::WholeSession);wait(s,36002);require(s.gaveUp(),"missing marker fails bounded wait"); }
    tape(base/"overlay",{{3,5,"controller-normal",true,false,false},{5,7,"scenario-foreground",false,false,false},{7,9,"controller-normal",false,true,false}});
    { TP::Session s(base/"overlay",TP::Session::Mode::WholeSession);wait(s,3);
      for(unsigned f=3;f<9;++f) {
        const char* kind=f>=5 && f<7 ? "scenario-foreground":"controller-normal";
        const auto step=s.beforeRead(marker(f,f,f==3,kind));
        if(step.kind==TP::Step::Kind::Start)require(s.acknowledgeSeed(99),"overlay/resume RNG");
        read(s,f);s.seam(row(f));
      }
      require(s.finished() && !s.gaveUp() && s.segmentCount()==3,"overlay entry and same-scene resume retain all reads"); }
    tape(base/"end",{{3,5,"controller-normal",true,false,false}});
    { TP::Session s(base/"end",TP::Session::Mode::WholeSession);wait(s,3);
      for(unsigned f=3;f<5;++f) { auto step=s.beforeRead(marker(f,f,f==3));if(step.kind==TP::Step::Kind::Start)s.acknowledgeSeed(99);read(s,f);s.seam(row(f)); }
      s.beforeRead(marker(5,5,false));require(s.gaveUp(),"late phase exit is not hidden as loading"); }
    tape(base/"restart",{{3,5,"controller-normal",true,false,false},{5,7,"controller-normal",true,true,false}});
    { TP::Session s(base/"restart",TP::Session::Mode::WholeSession);wait(s,3);
      for(unsigned f=3;f<7;++f) {
        auto step=s.beforeRead(marker(f,f,f==3 || f==5));
        if(step.kind==TP::Step::Kind::Start)require(s.acknowledgeSeed(99),"same-stage restart RNG");
        read(s,f);s.seam(row(f));
      }
      require(s.finished() && !s.gaveUp() && s.segmentCount()==2,"same-stage visits are separate marker/input phases"); }
    tape(base/"gap",{{3,7,"controller-normal",true,true,false}});
    { std::fstream k(base/"gap"/"route.kpd",std::ios::in|std::ios::out|std::ios::binary);
      k.seekp(24+40+3);k.put(1);k.close();
      bool threw=false;try { TP::Session s(base/"gap",TP::Session::Mode::WholeSession); }catch(const H::ReplayError&){threw=true;}
      require(threw,"non-neutral excluded control cannot disappear into a load gap"); }
    tape(base/"arrival",{{3,9,"controller-normal",true,true,false}});
    { auto text=[&] { std::ifstream in(base/"arrival"/"route.tape.json");std::ostringstream out;out<<in.rdbuf();return out.str(); }();
      const std::string empty="\"play_ready_seams\":[]";
      text.replace(text.find(empty),empty.size(),"\"play_ready_seams\":[{\"row\":5,\"stage\":\"WorldMap01Galaxy\",\"scenario\":1,\"rng_seed\":99}]");
      std::ofstream(base/"arrival"/"route.tape.json")<<text;
      TP::Session s(base/"arrival",TP::Session::Mode::WholeSession);wait(s,3);
      for(unsigned f=3;f<9;++f) {
        const auto start=s.beforeRead(marker(f,f,f==3));
        if(start.kind==TP::Step::Kind::Start)require(s.acknowledgeSeed(99),"arrival marker RNG");
        read(s,f);auto state=row(f);state.playReady=f>=5;
        const auto step=s.seam(state);
        if(step.kind==TP::Step::Kind::Start)require(s.acknowledgeSeed(99),"arrival play-ready RNG readback");
      }
      require(s.finished() && !s.gaveUp() && s.report()[0].playReadyPositionExact && s.report()[0].rngCheckpointVerified,
              "arrival keeps strict state/RNG play-ready anchor and exact first gameplay input"); }
    { bool threw=false;try { TP::parseSidecar("{\"schema\":3,\"seams\":20}"); }catch(const H::ReplayError&){threw=true;}require(threw,"unknown schema never falls back"); }
    { bool threw=false;try { TP::parseSidecar("{\"schema\":2,\"seams\":20}"); }catch(const H::ReplayError&){threw=true;}require(threw,"missing schema2 markers never use initEnd"); }
    fs::remove_all(base);std::puts("schema2 first-read frame/count/pointer, marker, overlay, exit, RNG and negative checks PASS");
}
