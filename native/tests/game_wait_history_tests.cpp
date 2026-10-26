#include <petari/game_wait_ring.hpp>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <thread>
#include <vector>
namespace H=PetariNative::GameWaitHistory;
namespace {
unsigned checks=0,begins=0,ends=0;
void check(bool ok,const char* why) { ++checks; if(!ok){std::fprintf(stderr,"FAIL %s\n",why);std::exit(1);} }
H::Token begin(H::Kind,std::uint64_t,std::uint64_t) noexcept { ++begins; return {}; }
void end(const H::Token&) noexcept { ++ends; }
void event(H::Kind,std::uint64_t,std::uint64_t,std::uint64_t) noexcept { ++begins; }
void mark(int,std::uint64_t,std::uint64_t) noexcept { ++begins; }
void finish() noexcept { ++begins; }
}
int main() {
    using V=H::Verdict;
    check(H::classify(1,100000002,0,0)==V::Trigger,"strict wall >100ms");
    check(H::classify(1,100000001,0,0)==V::NearMiss,"exact100ms is near, not trigger");
    check(H::classify(1,200000001,0,20000000)==V::NearMiss,"exact10percent not trigger");
    check(H::classify(1,200000001,0,19999999)==V::Trigger,"one ns below10percent triggers");
    check(H::classify(1,50000001,0,0)==V::Ordinary,"exact50ms no summary");
    check(H::classify(1,50000002,0,0)==V::NearMiss,"above50ms near summary");
    check(H::classify(1,500000001,0,400000000)==V::NearMiss,"CPU-heavy slow phase still summarized");
    check(H::classify(0,200000001,0,0)==V::Invalid,"missing start is not zero CPU");
    check(H::classify(10,9,0,0)==V::Invalid,"backward wall rejected");
    check(H::classify(1,200000001,-1,0)==V::Invalid,"missing CPU rejected");
    check(H::classify(1,200000001,5,4)==V::Invalid,"backward CPU rejected");
    check(H::classify(1,UINT64_MAX,0,INT64_MAX)==V::NearMiss,"ratio arithmetic does not overflow");
    H::Ring<8> ring; H::Event e{};
    check(!ring.read(0,e) && !ring.read(1,e),"unpublished sequence refused");
    for(unsigned i=1;i<=16;++i){H::Event x;x.frame=i;x.begin=i*100; x.end=i*100+9;check(ring.push(x),"single writer publish");}
    check(!ring.read(8,e),"frozen prefix overwritten: fail closed");
    for(unsigned i=9;i<=16;++i) check(ring.read(i,e)&&e.frame==i&&e.begin==i*100,"wrap retains exact sequence/payload");
    H::Coverage c; c.prefix=c.suffix=true;check(c.complete(),"complete coverage");
    c.missing=1;check(!c.complete(),"missing sequence refuses attribution");c.missing=0;
    c.drops=1;check(!c.complete(),"producer drop refuses attribution");c.drops=0;
    c.clipped=1;check(!c.complete(),"cap/clipping refuses attribution");c.clipped=0;
    c.suffix=false;check(!c.complete(),"shutdown before post-history refuses attribution");
    H::Budget budget;check(budget.take(H::Budget::maxBytes),"exact byte cap");check(!budget.take(1),"byte overflow refused");
    H::Budget windows;for(unsigned i=0;i<16;++i)check(windows.take(0,true),"bounded window admitted");
    check(!windows.take(0,true)&&windows.windows==16,"seventeenth window refused");
    const H::Hooks hooks{begin,end,event,mark,finish};H::hooks.store(&hooks);
    int effects=0;
    check(H::invoke(H::Kind::Token,0,0,[&]{return ++effects;})==1 && !begins&&!ends,"off executes original once without hooks");
    PETARI_GAME_WAIT_EVENT(H::Kind::Ready,++effects);
    H::mark(0,1);H::finish();check(effects==1&&!begins&&!ends,"off erases metadata evaluation and callbacks");
    H::enabled.store(true);
    check(H::invoke(H::Kind::Token,0,0,[&]{return ++effects;})==2&&begins==1&&ends==1,"on preserves result and exactly-once operation");
    try { H::invoke(H::Kind::Token,0,0,[]{throw std::runtime_error("original");}); } catch(const std::runtime_error&) {}
    check(begins==2&&ends==2,"exception keeps original throw and balanced observation");
    H::enabled.store(false);H::hooks.store(nullptr);
    H::Ring<1024> concurrent;
    std::vector<std::thread> threads;
    for(unsigned t=0;t<4;++t)threads.emplace_back([&,t]{for(unsigned i=0;i<2048;++i){H::Event x;x.thread=t;x.value=i;x.object=t*4096+i;concurrent.push(x);}});
    for(auto& t:threads)t.join();
    unsigned present=0;
    for(auto seq=concurrent.frontier()-1024;seq<concurrent.frontier();++seq) {
        if(concurrent.read(seq,e)){++present;check(e.thread<4&&e.value<2048&&e.object==e.thread*4096+e.value,"concurrent snapshot never mixes invalid fields");}
    }
    check(present>0,"concurrent history retained");
    std::printf("game wait history passed (%u checks)\n",checks);
}
