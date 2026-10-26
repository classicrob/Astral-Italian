#include "game_wait_history_runtime.hpp"
#include <petari/game_wait_ring.hpp>
#include <petari/frame_telemetry.hpp>
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <unistd.h>
#include <memory>
#include <thread>
#include <pthread.h>
#include <time.h>
#if defined(__APPLE__)
#include <mach/mach.h>
#include <mach/task_info.h>
#include <CoreGraphics/CoreGraphics.h>
#endif

namespace PetariNative::GameWaitHistory {
namespace {
constexpr std::uint64_t preNs=2000000000,postNs=250000000,filterNs=250000;
constexpr unsigned cellCount=65536,stackCount=32;
struct Active { std::atomic<std::uint64_t> id{0},start{0},kind{0},object{0}; };
struct State {
    Ring<cellCount> ring;
    Ring<128> bindings;
    std::array<Event,cellCount> copied{};
    std::array<std::uint64_t,cellCount> sequences{};
    std::array<Active,stackCount> active;
    std::atomic<std::uint64_t> frame{0},phaseStart{0},mainOs{0},holder{0},holderHost{0},interruptOwner{0};
    std::atomic<std::uint64_t> ids{1},invalid{0},phases{0},slow{0},triggers{0},filtered{0},filteredNs{0},pendingMain{0},interruptPending{0},overflows{0};
    std::atomic<bool> stop{false},sleeping{false};
    std::uint64_t mainThread=0,started=0,windowId=0;
    std::int64_t phaseCpu=-1;
    std::uint64_t previousFaults=0,previousPageins=0,lastVm=0;
    std::thread writer;
    std::FILE* file=nullptr;
    Budget budget;
    bool outputFailed=false;
#if defined(__APPLE__)
    mach_port_t mainPort=MACH_PORT_NULL;
#endif
};
static_assert(sizeof(State)<32ull*1024*1024,"history hard memory cap");
State* state=nullptr; // immutable once hooks are published; storage lives until process exit
thread_local unsigned depth=0;
thread_local std::uint64_t generation=0;
std::atomic_flag initialized=ATOMIC_FLAG_INIT,finished=ATOMIC_FLAG_INIT;
std::uint64_t now() noexcept { return FrameTelemetry::nowNs(); }
std::uint64_t threadId() noexcept { return reinterpret_cast<std::uintptr_t>(pthread_self()); }
std::int64_t cpuNow() noexcept {
    timespec value{};
    if (clock_gettime(CLOCK_THREAD_CPUTIME_ID,&value)) return -1;
    return static_cast<std::int64_t>(value.tv_sec)*1000000000+value.tv_nsec;
}
void push(Kind kind,std::uint64_t start,std::uint64_t end,std::uint64_t object=0,
          std::uint64_t value=0,std::uint64_t id=0,std::uint64_t detail=0,std::uint64_t cpu=0,
          std::uint64_t packet=0,std::uint64_t ticket=0,std::uint64_t aux0=0,std::uint64_t aux1=0) noexcept {
    if (!generation) {
        generation=state->ids.fetch_add(1,std::memory_order_relaxed);
        state->bindings.push({static_cast<std::uint64_t>(Kind::ThreadBind),start,start,threadId(),0,0,0,0,0,0,0,0,generation});
    }
    const Event e{static_cast<std::uint64_t>(kind),start,end,threadId(),object,value,
                  state->frame.load(std::memory_order_relaxed),id,detail,cpu,packet,ticket,generation,aux0,aux1};
    if (kind==Kind::ThreadBind) state->bindings.push(e);
    else state->ring.push(e);
}
bool waitKind(Kind kind) noexcept {
    return kind==Kind::InterruptMutex || kind==Kind::CpuWait || kind==Kind::Fifo || kind==Kind::Token ||
           kind==Kind::DrawDone || kind==Kind::FrameSlot || kind==Kind::Staging || kind==Kind::RenderQueue ||
           kind==Kind::RenderSync || kind==Kind::Snapshot;
}
Token begin(Kind kind,std::uint64_t object,std::uint64_t value) noexcept {
    const auto stamp=now(),id=state->ids.fetch_add(1,std::memory_order_relaxed);
    Token token{stamp,static_cast<std::uint64_t>(kind),object,value,id,UINT64_MAX};
    if (kind==Kind::InterruptMutex) token.value=state->interruptOwner.load(std::memory_order_relaxed);
    if (threadId()==state->mainThread) {
        if (depth==stackCount) { ++state->overflows; return token; }
        token.depth=depth++;
        auto& slot=state->active[token.depth];
        slot.start.store(stamp,std::memory_order_relaxed); slot.kind.store(token.kind,std::memory_order_relaxed);
        slot.object.store(object,std::memory_order_relaxed); slot.id.store(id,std::memory_order_release);
        if (waitKind(kind)) state->pendingMain.fetch_add(1,std::memory_order_relaxed);
        if (kind==Kind::InterruptMutex) state->interruptPending.fetch_add(1,std::memory_order_relaxed);
    }
    return token;
}
void end(const Token& token) noexcept {
    const auto stamp=now();
    if (token.depth!=UINT64_MAX) {
        auto& slot=state->active[token.depth];
        if (!depth || token.depth!=depth-1 || slot.id.load(std::memory_order_acquire)!=token.id) ++state->invalid;
        else --depth;
        slot.id.store(0,std::memory_order_release);
        if (waitKind(static_cast<Kind>(token.kind))) state->pendingMain.fetch_sub(1,std::memory_order_relaxed);
        if (static_cast<Kind>(token.kind)==Kind::InterruptMutex) state->interruptPending.fetch_sub(1,std::memory_order_relaxed);
    }
    if (stamp<token.start) { ++state->invalid; return; }
    if (stamp-token.start>=filterNs) {
        push(static_cast<Kind>(token.kind),token.start,stamp,token.object,token.value,token.id,
             state->holder.load(std::memory_order_relaxed));
    } else { ++state->filtered; state->filteredNs.fetch_add(stamp-token.start,std::memory_order_relaxed); }
}
void eventImpl(Kind kind,std::uint64_t object,std::uint64_t value,std::uint64_t detail) noexcept {
    const bool main=threadId()==state->mainThread;
    if (main && (kind==Kind::CpuResumed || kind==Kind::Sleep || kind==Kind::HostWork || kind==Kind::ThreadBind))
        state->mainOs.store(object,std::memory_order_relaxed);
    const auto stamp=now();
    if (kind==Kind::BatonGrant) {
        state->holder.store(object,std::memory_order_relaxed);
        state->holderHost.store(detail,std::memory_order_relaxed);
        if (!state->sleeping.load(std::memory_order_relaxed) && !state->pendingMain.load(std::memory_order_relaxed) &&
            object!=state->mainOs.load(std::memory_order_relaxed)) return;
    }
    if (kind==Kind::InterruptOwner) {
        const auto previous=state->interruptOwner.exchange(object,std::memory_order_relaxed);
        const bool pending=state->interruptPending.load(std::memory_order_relaxed)!=0;
        if (object) { if (pending) push(kind,stamp,stamp,object,1,0,detail); }
        else if (pending || (value && stamp>=value && stamp-value>=filterNs))
            push(kind,value,stamp,previous,0,0,detail);
        return;
    }
    if (kind==Kind::Sleep && main) state->sleeping.store(true,std::memory_order_relaxed);
    if (kind==Kind::CpuResumed && main) state->sleeping.store(false,std::memory_order_relaxed);
    if ((kind==Kind::Ready || kind==Kind::CpuResumed || kind==Kind::Sleep) &&
        !main && object!=state->mainOs.load(std::memory_order_relaxed)) return;
    push(kind,stamp,stamp,object,value,0,detail,0,(kind==Kind::Capture || kind==Kind::Worker)?object:0,kind==Kind::Capture?value:0);
}
void markImpl(int mark,std::uint64_t frame,std::uint64_t stamp) noexcept {
    if (threadId()!=state->mainThread) { ++state->invalid; return; }
    if (!stamp) stamp=now(); const auto cpu=cpuNow();
    state->frame.store(frame,std::memory_order_relaxed);
    if (mark==0) {
        if (const auto old=state->phaseStart.load(std::memory_order_relaxed)) push(Kind::InvalidPhase,old,stamp,0,0,frame);
        state->phaseCpu=cpu; state->phaseStart.store(stamp,std::memory_order_release);
        push(Kind::Phase,stamp,stamp,0,0,frame,0,cpu<0?UINT64_MAX:static_cast<std::uint64_t>(cpu));
        return;
    }
    if (mark!=1) { ++state->invalid; return; }
    const auto start=state->phaseStart.exchange(0,std::memory_order_acq_rel);
    ++state->phases;
    const auto verdict=classify(start,stamp,state->phaseCpu,cpu);
    const auto used=cpu>=state->phaseCpu && state->phaseCpu>=0 ? static_cast<std::uint64_t>(cpu-state->phaseCpu) : UINT64_MAX;
    push(Kind::Phase,start,stamp,0,1,frame,static_cast<unsigned>(verdict),used);
    if (verdict==Verdict::Invalid) { push(Kind::InvalidPhase,start,stamp,0,0,frame,0,used); }
    // One summary for EVERY wall>50ms phase (including qualifying triggers).
    if (start && stamp>=start && stamp-start>50000000) {
        ++state->slow; if (verdict==Verdict::Trigger) ++state->triggers;
        push(verdict==Verdict::Trigger?Kind::Trigger:Kind::NearMiss,start,stamp,0,0,frame,static_cast<unsigned>(verdict),used);
    }
}
void line(const char* text) noexcept {
    if (state->outputFailed) return;
    const auto bytes=std::strlen(text);
    if (!state->budget.take(bytes)) { state->outputFailed=true; return; }
    if (std::fwrite(text,1,bytes,state->file)!=bytes) state->outputFailed=true;
}
void eventLine(const char* type,std::uint64_t seq,const Event& e) noexcept {
    char text[1024];
    const auto count=std::snprintf(text,sizeof(text),
        "{\"type\":\"%s\",\"sequence\":%llu,\"kind\":%llu,\"name\":\"%s\",\"begin_ns\":%llu,\"end_ns\":%llu,\"thread\":%llu,\"object\":%llu,\"value\":%llu,\"frame\":%llu,\"id\":%llu,\"detail\":%llu,\"cpu_ns\":%llu,\"packet\":%llu,\"ticket\":%llu,\"generation\":%llu,\"aux0\":%llu,\"aux1\":%llu}\n",type,
        (unsigned long long)seq,(unsigned long long)e.kind,kindName(static_cast<Kind>(e.kind)),(unsigned long long)e.begin,(unsigned long long)e.end,
        (unsigned long long)e.thread,(unsigned long long)e.object,(unsigned long long)e.value,(unsigned long long)e.frame,
        (unsigned long long)e.id,(unsigned long long)e.detail,(unsigned long long)e.cpu,(unsigned long long)e.packet,(unsigned long long)e.ticket,(unsigned long long)e.generation,(unsigned long long)e.aux0,(unsigned long long)e.aux1);
    if (count<0 || static_cast<std::size_t>(count)>=sizeof(text)) { state->outputFailed=true; return; }
    line(text);
}
void vmSample(std::uint64_t stamp) noexcept {
#if defined(__APPLE__)
    task_events_info_data_t info{}; mach_msg_type_number_t count=TASK_EVENTS_INFO_COUNT;
    const auto start=now();
    const auto result=task_info(mach_task_self(),TASK_EVENTS_INFO,reinterpret_cast<task_info_t>(&info),&count);
    const auto stop=now();
    const bool valid=result==KERN_SUCCESS;
    const auto faults=valid?info.faults:0, pageins=valid?info.pageins:0;
    push(Kind::Vm,start,stop,result,faults,0,pageins,0,0,0,
         valid && faults>=state->previousFaults?faults-state->previousFaults:UINT64_MAX,
         valid && pageins>=state->previousPageins?pageins-state->previousPageins:UINT64_MAX);
    if (valid) { state->previousFaults=faults; state->previousPageins=pageins; }
#endif
    state->lastVm=stamp;
}
void windowSample() noexcept {
#if defined(__APPLE__)
    const auto start=now();
    if (!state->windowId) { push(Kind::WindowState,start,now(),0,0); return; }
    CFArrayRef windows=CGWindowListCopyWindowInfo(kCGWindowListOptionIncludingWindow,static_cast<CGWindowID>(state->windowId));
    if (!windows) { push(Kind::WindowState,start,now(),0,UINT64_MAX); return; }
    unsigned observed=0;
    for (CFIndex i=0;i<CFArrayGetCount(windows) && observed<1;++i) {
        auto item=static_cast<CFDictionaryRef>(CFArrayGetValueAtIndex(windows,i));
        int pid=0,number=0; double alpha=0; CGRect bounds{};
        auto pidRef=static_cast<CFNumberRef>(CFDictionaryGetValue(item,kCGWindowOwnerPID));
        if (!pidRef || !CFNumberGetValue(pidRef,kCFNumberIntType,&pid) || pid!=getpid()) continue;
        auto id=static_cast<CFNumberRef>(CFDictionaryGetValue(item,kCGWindowNumber));
        auto alphaRef=static_cast<CFNumberRef>(CFDictionaryGetValue(item,kCGWindowAlpha));
        auto rect=static_cast<CFDictionaryRef>(CFDictionaryGetValue(item,kCGWindowBounds));
        const bool valid=id && alphaRef && rect && CFNumberGetValue(id,kCFNumberIntType,&number) &&
            CFNumberGetValue(alphaRef,kCFNumberDoubleType,&alpha) && CGRectMakeWithDictionaryRepresentation(rect,&bounds);
        if (valid && static_cast<std::uint64_t>(number)!=state->windowId) continue;
        const bool on=CFDictionaryGetValue(item,kCGWindowIsOnscreen)==kCFBooleanTrue;
        push(Kind::WindowState,start,now(),number,(valid?1:0)|(on?2:0)|(alpha==1.0?4:0),0,0,0,0,0,
             bounds.size.width>0?static_cast<std::uint64_t>(bounds.size.width):0,
             bounds.size.height>0?static_cast<std::uint64_t>(bounds.size.height):0);
        ++observed;
    }
    CFRelease(windows);
    if (!observed) push(Kind::WindowState,start,now(),0,0);
#endif
}
void sample(std::uint64_t stamp) noexcept {
    auto start=state->phaseStart.load(std::memory_order_acquire);
    for (auto& slot:state->active) {
        const auto id=slot.id.load(std::memory_order_acquire);
        if (!id) continue;
        const auto candidate=slot.start.load(std::memory_order_relaxed);
        if (id!=slot.id.load(std::memory_order_acquire)) continue;
        if (!start || candidate<start) start=candidate;
    }
    if (!start || stamp<start || stamp-start<=20000000) return;
#if defined(__APPLE__)
    thread_basic_info_data_t info{}; mach_msg_type_number_t count=THREAD_BASIC_INFO_COUNT;
    const auto before=now();
    const auto rc=thread_info(state->mainPort,THREAD_BASIC_INFO,reinterpret_cast<thread_info_t>(&info),&count);
    const auto after=now();
    const auto cpu=rc==KERN_SUCCESS?(static_cast<std::uint64_t>(info.user_time.seconds)+info.system_time.seconds)*1000000000+
        (static_cast<std::uint64_t>(info.user_time.microseconds)+info.system_time.microseconds)*1000:UINT64_MAX;
    push(Kind::RunState,before,after,state->mainThread,rc,0,rc==KERN_SUCCESS?info.run_state:UINT64_MAX,cpu,
         0,0,rc==KERN_SUCCESS?info.suspend_count:UINT64_MAX,state->holder.load(std::memory_order_relaxed));
#endif
    if (stamp-state->lastVm>=100000000) { vmSample(stamp); windowSample(); }
}
struct Window { std::uint64_t start=0,end=0,frame=0,ids[16]{}; unsigned count=0; };
void dump(Window& window) noexcept {
    if (!state->budget.take(0,true)) { line("{\"type\":\"refused\",\"reason\":\"window-cap\"}\n"); return; }
    const auto copyStart=now();
    push(Kind::Writer,copyStart,copyStart); // actual post-history boundary, not a fabricated frame
    const auto frontier=state->ring.frontier();
    const auto first=frontier>cellCount?frontier-cellCount:1;
    Coverage coverage; coverage.drops=state->ring.drops()+state->invalid.load()+state->overflows.load()+state->bindings.drops();
    if (state->bindings.frontier()>129) ++coverage.clipped;
    unsigned size=0;
    for (auto seq=first;seq<frontier;++seq) {
        Event e;
        if (!state->ring.read(seq,e)) { ++coverage.missing; continue; }
        if (e.end<=window.start) coverage.prefix=true;
        if (e.end>=window.end) coverage.suffix=true;
        if (e.end<window.start || e.begin>window.end) continue;
        if (e.kind==static_cast<unsigned>(Kind::InvalidPhase)) ++coverage.clipped;
        state->copied[size]=e; state->sequences[size++]=seq;
    }
    coverage.prefix=coverage.prefix && window.start>=state->started;
    if (state->stop.load()) coverage.suffix=false; // early shutdown cannot invent posthistory
    char text[768];
    std::snprintf(text,sizeof(text),"{\"type\":\"window\",\"frame\":%llu,\"start_ns\":%llu,\"end_ns\":%llu,\"triggers\":%u,\"prefix\":%s,\"suffix\":%s,\"missing\":%llu,\"drops\":%llu,\"clipped\":%llu,\"complete\":%s,\"attribution\":\"%s\",\"copied\":%u}\n",
        (unsigned long long)window.frame,(unsigned long long)window.start,(unsigned long long)window.end,window.count,
        coverage.prefix?"true":"false",coverage.suffix?"true":"false",(unsigned long long)coverage.missing,
        (unsigned long long)coverage.drops,(unsigned long long)coverage.clipped,coverage.complete()?"true":"false",coverage.complete()?"eligible-boundaries-only":"REFUSED-incomplete",size);
    line(text);
    for (unsigned i=0;i<window.count;++i) {
        std::snprintf(text,sizeof(text),"{\"type\":\"window-trigger\",\"frame\":%llu}\n",(unsigned long long)window.ids[i]); line(text);
    }
    const auto roster=state->bindings.frontier();
    for (auto seq=1;seq<roster && seq<=128;++seq) {
        Event binding; if (state->bindings.read(seq,binding)) eventLine("binding",seq,binding);
        else line("{\"type\":\"refused\",\"reason\":\"binding-gap\"}\n");
    }
    for (unsigned i=0;i<size;++i) eventLine("event",state->sequences[i],state->copied[i]);
    const auto end=now();
    std::snprintf(text,sizeof(text),"{\"type\":\"writer\",\"copy_begin_ns\":%llu,\"end_ns\":%llu,\"bytes_total\":%llu}\n",
        (unsigned long long)copyStart,(unsigned long long)end,(unsigned long long)state->budget.bytes); line(text);
    std::fflush(state->file);
}
void writerMain() noexcept {
    pthread_setname_np("Petari game wait history");
    // No QoS/priority/affinity override. This observer never touches production locks.
    Window pending; std::uint64_t consumed=1; unsigned unsettled=0;
    while (true) {
        const auto stamp=now(); sample(stamp);
        const auto frontier=state->ring.frontier();
        if (frontier-consumed>cellCount) { ++state->invalid; consumed=frontier-cellCount; }
        for (;consumed<frontier;++consumed) {
            Event e; if (!state->ring.read(consumed,e)) {
                // A reserved cell may still be publishing. Writer defers at most
                // four polls; producers never wait. A real hole is refused.
                if (!state->stop.load() && ++unsettled<4) break;
                ++state->invalid; unsettled=0; continue;
            }
            unsettled=0;
            if (e.kind!=static_cast<unsigned>(Kind::NearMiss) && e.kind!=static_cast<unsigned>(Kind::Trigger)) continue;
            eventLine("summary",consumed,e);
            if (e.kind!=static_cast<unsigned>(Kind::Trigger)) continue;
            if (pending.count && e.end>pending.end) { dump(pending); pending={}; }
            if (!pending.count) { pending.start=e.end>preNs?e.end-preNs:0; pending.frame=e.frame; }
            if (pending.count<16) pending.ids[pending.count++]=e.frame; else ++state->overflows;
            pending.end=e.end+postNs;
        }
        if (pending.count && (stamp>=pending.end || state->stop.load())) { dump(pending); pending={}; }
        if (state->stop.load()) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    char text[1024];
    std::snprintf(text,sizeof(text),"{\"type\":\"footer\",\"complete\":%s,\"windows\":%u,\"bytes_before_footer\":%llu,\"drops\":%llu,\"invalid\":%llu,\"filtered_short_spans\":%llu,\"filtered_ns\":%llu,\"output_failed\":%s,\"phases\":%llu,\"slow_summaries\":%llu,\"trigger_summaries\":%llu}\n",
        state->outputFailed?"false":"true",state->budget.windows,(unsigned long long)state->budget.bytes,
        (unsigned long long)state->ring.drops(),(unsigned long long)state->invalid.load(),
        (unsigned long long)state->filtered.load(),(unsigned long long)state->filteredNs.load(),state->outputFailed?"true":"false",(unsigned long long)state->phases.load(),(unsigned long long)state->slow.load(),(unsigned long long)state->triggers.load());
    line(text); std::fflush(state->file); std::fclose(state->file); state->file=nullptr;
}
void finishImpl() noexcept {
    if (finished.test_and_set()) return;
    enabled.store(false,std::memory_order_release);
    state->stop.store(true,std::memory_order_release);
    if (state->writer.joinable()) state->writer.join();
}
const Hooks callbacks{begin,end,eventImpl,markImpl,finishImpl};
} // namespace
bool initializeFromEnvironment(std::uint64_t windowId) noexcept {
    const char* opt=std::getenv("PETARI_GAME_WAIT_HISTORY");
    if (!opt || std::strcmp(opt,"1")) return true;
    if (initialized.test_and_set()) return state!=nullptr;
    const char* path=std::getenv("PETARI_GAME_WAIT_HISTORY_FILE");
    if (!path || path[0]!='/') { std::fputs("[game wait history] refused: explicit NEW absolute FILE required\n",stderr); return false; }
    try {
        auto owned=std::make_unique<State>();
        owned->file=std::fopen(path,"wx");
        if (!owned->file) { std::fputs("[game wait history] refused: output exists or cannot be created\n",stderr); return false; }
        state=owned.release(); state->windowId=windowId; state->mainThread=threadId(); state->started=now(); state->ring.prefault();
#if defined(__APPLE__)
        state->mainPort=mach_thread_self(); // retained for this diagnostic process lifetime
#endif
        line("{\"type\":\"header\",\"schema\":1,\"clock\":\"steady-ns\",\"cells\":65536,\"max_memory_bytes\":33554432,\"max_output_bytes\":67108864,\"max_windows\":16,\"filter_ns\":250000,\"pre_ns\":2000000000,\"post_ns\":250000000,\"cpu_scope\":\"matched-retrace-to-end-frame\",\"vm_scope\":\"process-wide-not-thread-attribution\",\"independent_audio_transport\":true,\"interrupt_owner_zero\":\"release-intent-not-kernel-unlock\",\"run_state_sampling\":\"point-samples-no-wait-channel\",\"memory_scope\":\"recorder-owned-fixed-storage\"}\n");
        vmSample(now());
        state->writer=std::thread(writerMain);
        hooks.store(&callbacks,std::memory_order_release); enabled.store(true,std::memory_order_release);
        std::atexit(finishImpl);
        return true;
    } catch (...) {
        if (state && state->file) { std::fclose(state->file); state->file=nullptr; }
        std::fputs("[game wait history] initialization failed; diagnostic OFF\n",stderr); return false;
    }
}
} // namespace PetariNative::GameWaitHistory
