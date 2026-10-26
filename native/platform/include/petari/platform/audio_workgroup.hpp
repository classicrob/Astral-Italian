#pragma once
#include <atomic>
#include <cstdint>
#include <cstdio>

namespace PetariNative::Platform::AudioWorkgroup {
struct Identity {
    std::uintptr_t thread = 0;
    std::uint64_t generation = 0;
    bool operator==(Identity b) const { return thread == b.thread && generation == b.generation; }
    explicit operator bool() const { return thread && generation; }
};
struct Origin {
    Identity caller{};
    std::uint64_t epoch = 0, queuedMach = 0;
};

// Experimental arm only. Configured before AI/DSP workers start. The ordinary
// path branches on this cached flag and never resolves workgroup TLS.
extern std::atomic<bool> enabled;
extern std::atomic<bool> observing;
void configure();                         // PETARI_AUDIO_WORKGROUP=async; otherwise off
void bindCallback(Identity, std::uintptr_t callback, bool safe);
void confirmStart(Identity, std::uintptr_t callback, bool safe);
void startSink(Identity, bool coreAudio, unsigned sourceRate = 32000);  // host-only, no interrupt/queue lock
void startCurrentSink(bool coreAudio, unsigned sourceRate);
void stopSink();                          // host-only; members leave on their own threads
std::uint64_t epoch();
std::uint64_t controlVersion();
void pauseProduction(bool);
void guestSafePoint(Identity);            // no interrupt lock; only the bound role joins
void guestExit(Identity);                 // no interrupt lock; before pthread_exit
void rendererSafePoint();                 // no DSP queue lock
void rendererExit();
Origin origin(Identity);
void checkArgument(Origin command, Origin argument);
void beginCycle(Origin, unsigned subframes, unsigned rate, std::uintptr_t left, std::uintptr_t right);
void subframeComplete();                  // final output written, before final acknowledgement
void report(std::FILE*);                  // control/reporter thread only
void collect();                          // release retired objects off realtime threads
void setGuestWake(void (*)());           // installed before real sink start
void setRendererWake(void (*)());
void noteAssigned(Identity, std::uint64_t mach);
void noteResumed(Identity, std::uint64_t mach);
void noteCallback(std::uint64_t level, std::uint64_t requested);
void noteShortfall(std::uint64_t frames);
void noteRegistration(Identity, std::uint64_t generation, unsigned frames, std::uintptr_t address);
// Patched SDL's C callback; called only after its cached observer-pointer check.
// Native fill hooks check observing before calling. No TLS or queries off-path.
void noteTransport(unsigned boundary, const void* object, std::int64_t value);
}
