#include "draw_observer.hpp"
#include <cassert>
#include <cstring>
#include <memory>
#include <sstream>
#include <streambuf>

using namespace PetariNative::Visual;
namespace {
Snapshot sample{};
unsigned reads=0;
Snapshot fake(const GameSystem*,const MainLoopFramework*) noexcept { ++reads;return sample; }
SourceContext context(std::uint64_t frame=10) {
    SourceContext out{};std::strcpy(out.key.scene,"Game");std::strcpy(out.key.stage,"RedBlueExGalaxy");
    out.key.placedScenario=out.key.selectedScenario=1;out.valid=1;out.sourceFrame=frame;out.sampledEpoch=7;return out;
}
Snapshot ready() {
    Snapshot out{};out.key=context().key;out.initState=6;out.bufferMode=3;out.renderInterval=1;
    out.frameworkPresent=out.xfbPresent=out.controllerPresent=out.currentSceneNonnull=out.executedSceneNonnull=1;
    out.initializedGame=out.canonicalPlayerValid=out.executedSceneIsCurrent=1;return out;
}
void cycle(DrawObserver& o,const Snapshot& s,std::uint64_t frame) {
    o.observe(Phase::LoopEntry,s);o.observe(Phase::DrawEntry,s);o.observe(Phase::DrawExit,s);o.postEndFrame(context(frame));
}
struct FailedWrite : std::streambuf { int_type overflow(int_type) override { return traits_type::eof(); } };
}
int main() {
    assert(drawObserverCallbacks()==nullptr);
    auto o=std::make_unique<DrawObserver>();assert(!installDrawObserver(*o,fake));
    auto bad=context();bad.valid=0;assert(!o->begin(bad));
    assert(o->begin(context()));assert(!installDrawObserver(*o,nullptr));assert(installDrawObserver(*o,fake));
    assert(!o->begin(context()));assert(!installDrawObserver(*o,fake));
    sample=ready();sample.renderFlag=0;
    const auto* cb=drawObserverCallbacks();assert(cb);
    cb->loopEntry(nullptr,nullptr);sample.renderFlag=1;cb->drawEntry(nullptr,nullptr);cb->drawExit(nullptr,nullptr);
    assert(reads==3 && o->size()==3 && o->fault()==Fault::None);
    assert(o->data()[0].snapshot.renderFlag==0 && o->data()[1].snapshot.renderFlag==1);
    assert(o->data()[1].drawSerial==o->data()[2].drawSerial);
    std::ostringstream stream;assert(!writeDrawObservations(stream,*o));assert(stream.str().empty());
    o->postEndFrame(context(11));assert(uninstallDrawObserver(*o));assert(drawObserverCallbacks()==nullptr);
    cb->loopEntry(nullptr,nullptr);assert(reads==3);
    assert(writeDrawObservations(stream,*o));assert(stream.str().find("\"qualification\":\"BLOCKED\"")!=std::string::npos);
    FailedWrite failed;std::ostream sink(&failed);assert(!writeDrawObservations(sink,*o));
    assert(o->begin(context()));o->observe(Phase::DrawEntry,ready());assert(o->fault()==Fault::Protocol);
    assert(o->begin(context()));o->observe(Phase::LoopEntry,ready());o->observe(Phase::DrawEntry,ready());
    o->observe(Phase::DrawExit,ready());o->postEndFrame(context(12));assert(o->fault()==Fault::Protocol);
    assert(o->begin(context()));sample=ready();sample.invalid=1;o->observe(Phase::LoopEntry,sample);
    assert(o->fault()==Fault::InvalidSnapshot && o->size()==1);
    assert(o->begin(context()));sample=ready();std::memset(sample.key.stage,'x',64);o->observe(Phase::LoopEntry,sample);
    assert(o->fault()==Fault::InvalidSnapshot);
    assert(o->begin(context()));sample=ready();sample.executedSceneIsCurrent=0;
    cycle(*o,sample,11);assert(o->fault()==Fault::None);assert(o->data()[1].snapshot.initializedGame==1 && o->data()[1].snapshot.executedSceneIsCurrent==0);
    assert(o->begin(context()));sample=ready();sample.initState=0;sample.initializedGame=0;
    cycle(*o,sample,11);assert(o->data()[1].snapshot.executedSceneIsCurrent==1 && !o->data()[1].snapshot.initializedGame);
    assert(o->begin(context()));sample=ready();sample.key.selectedScenario=2;
    cycle(*o,sample,11);assert(!o->data()[1].sourceKeyMatches);
    assert(o->begin(context()));sample=ready();
    for (std::uint64_t i=0;i<DrawEventCapacity/4;++i) cycle(*o,sample,11+i);
    assert(o->size()==DrawEventCapacity && o->fault()==Fault::None);
    o->observe(Phase::LoopEntry,sample);assert(o->fault()==Fault::Capacity && o->size()==DrawEventCapacity);
}
