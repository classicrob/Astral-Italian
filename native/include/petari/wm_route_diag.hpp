#pragma once
// SCRATCH ONLY: observation under PETARI_WM_DIAG=1. No renderer/input/state changes.
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <functional>
#include <thread>
#include <petari/latency_probe.hpp>
namespace PetariWmRouteDiag {
inline bool enabled() { static const bool on = [] { const char* p = std::getenv("PETARI_WM_DIAG"); return p && std::strcmp(p, "1") == 0; }(); return on; }
// A scene-init worker can run under the game CPU baton while host compose advances
// the atomic presentation ordinal. Pin it for each multi-line observation.
inline thread_local unsigned observationDepth = 0;
inline thread_local unsigned long long observationFrame = 0;
struct Observation {
    bool active;
    Observation() : active(enabled()) {
        if(active && observationDepth++==0) observationFrame=PetariNative::LatencyProbe::frame.load(std::memory_order_relaxed);
    }
    ~Observation() { if(active) --observationDepth; }
};
inline unsigned long long runtimeFrame() { return observationDepth ? observationFrame : PetariNative::LatencyProbe::frame.load(std::memory_order_relaxed); }
inline unsigned long long threadId() { return static_cast<unsigned long long>(std::hash<std::thread::id>{}(std::this_thread::get_id())); }
inline std::atomic<unsigned long long> eventSequence{0};
inline const char* phase = "startup";
inline bool world = false, selected = false;
inline unsigned worldEpoch = 0;
inline const void* readyOwner = nullptr;
inline unsigned long long budgetFrame = ~0ull;
inline unsigned eventLines = 0;
inline void setPhase(const char* p) { if (enabled()) phase = p; }
inline bool event() {
    if (!enabled() || !world) return false;
    if (budgetFrame != runtimeFrame()) {
        if(eventLines>48) std::fprintf(stderr,"[wm-budget] aurora_frame=%llu dropped-events=%u\n",budgetFrame,eventLines-48);
        budgetFrame = runtimeFrame(); eventLines = 0;
    }
    const unsigned attempt=eventLines++;
    if(attempt==48) std::fprintf(stderr,"[wm-budget] aurora_frame=%llu seq=%llu event-budget-exhausted=1 trace-absence-inconclusive=1\n",runtimeFrame(),++eventSequence);
    return attempt < 48;
}
inline void request(const char* source, const char* name, int id, int frames, const void* actor,
                    float x, float y, float z, int node, int layer) {
    Observation observation;
    if (event()) std::fprintf(stderr, "[wm-request] aurora_frame=%llu seq=%llu thread=%llu phase=%s source=%s name=%s id=%d frames=%d actor=%p target=(%.6g,%.6g,%.6g) node=%d layer=%d\n",
        runtimeFrame(), ++eventSequence, threadId(), phase, source, name ? name : "UNKNOWN", id, frames, actor, x,y,z,node,layer);
}
inline void body(const char* name, int step) {
    Observation observation;
    if ((step==0 || step==6 || step==20 || step==60 || (step>0 && step%60==0)) && event())
        std::fprintf(stderr,"[wm-body] aurora_frame=%llu seq=%llu thread=%llu phase=%s body=%s step=%d\n",runtimeFrame(),++eventSequence,threadId(),phase,name,step);
}
struct Model { const void* ptr{}; int domain{}, node{}; unsigned passes{}; };
inline Model models[4096];
inline unsigned modelCount = 0, modelOverflow = 0;
inline unsigned pass = 0; // 0 main/world big; 1 mini; no GPU visibility claim.
inline bool counting = false;
inline void addModel(const void* p, int domain, int node) {
    if (!p) return;
    for (unsigned i=0; i<modelCount; ++i) if (models[i].ptr==p && models[i].domain==domain) return;
    if (modelCount == 4096) { ++modelOverflow; return; }
    models[modelCount++] = {p,domain,node,0};
}
inline void submitted(const void* p) {
    if (!enabled() || !world || !counting || !p) return;
    for (unsigned i=0; i<modelCount; ++i) if (models[i].ptr==p) models[i].passes |= 1u << pass;
}
inline unsigned submittedModels(int domain, unsigned which) {
    unsigned n=0; for (unsigned i=0; i<modelCount; ++i) n += models[i].domain==domain && (models[i].passes & (1u<<which)); return n;
}
inline unsigned submittedNodes(int domain, unsigned which) {
    unsigned n=0;
    for (unsigned i=0; i<modelCount; ++i) {
        if (models[i].domain!=domain || models[i].node<0 || !(models[i].passes&(1u<<which))) continue;
        bool old=false; for (unsigned j=0; j<i; ++j) if (models[j].domain==domain && models[j].node==models[i].node && (models[j].passes&(1u<<which))) { old=true; break; }
        n += !old;
    }
    return n;
}
}
