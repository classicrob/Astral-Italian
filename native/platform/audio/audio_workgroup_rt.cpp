// The entire AU publisher's reachable native implementation. Deliberately
// separated from policy/adoption/reporting: no OS, clock, TLS or host service.
#include <petari/platform/audio_workgroup.hpp>
#include "workgroup_history_store.hpp"
namespace PetariNative::Platform::AudioWorkgroup {
History::FlightHistory History::audioHistory;
__attribute__((noinline)) void noteOutputRender(unsigned kind,std::uint64_t mach,std::uint64_t ns,
    std::uint64_t ticket,std::uint64_t generation,std::uint64_t value) noexcept {
    // Reserved logical actors, not sampled physical pthread identifiers.
    History::audioHistory.push(mach,kind,(kind==50 || kind==52 || kind==54) ? 1 : 0,value,ns,ticket,generation);
    if(kind==47 || kind==51) History::audioHistory.trigger(mach);
}
}
