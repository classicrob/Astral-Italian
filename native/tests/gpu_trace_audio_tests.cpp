// Pure in-memory ring fixture; no CoreAudio device, compiler or app launched.
#include <petari/gpu_trace_audio.hpp>
#include "paced_ring.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <fstream>
#include <iterator>
#include <string>
#include <unistd.h>
using Ring = PetariNative::AudioSDL::Detail::PacedRing;
namespace AT = PetariNative::GpuTrace::Audio;
namespace GT = PetariNative::GpuTrace;
extern "C" void petari_audio_trace_read_off_probe(Ring* ring, std::int16_t* out, std::size_t count) {
    AT::read(*ring, out, count);
}
extern "C" std::uint64_t petari_audio_trace_report_off_probe(Ring* ring) {
    return AT::takeUnderruns(*ring);
}
extern "C" void petari_audio_callback_off_probe(Ring* ring, int bytes, int status) {
    PETARI_GPU_TRACE(AT::callbackBeginEnabled(*ring, bytes, status));
    PETARI_GPU_TRACE(AT::callbackEndEnabled(*ring, status));
}
int main() {
    Ring native, observed;
    const std::array<std::int16_t,8> input{{1,2,3,4,5,6,7,8}};
    std::array<std::int16_t,12> a{},b{};
    native.reset(0); observed.reset(0); native.write(input.data(),4); observed.write(input.data(),4);
    assert(!GT::enabled.load() && GT::run == nullptr);
    petari_audio_callback_off_probe(&observed,24,0);
    assert(AT::activeCallbacks.load()==0 && AT::lastEntryNs.load()==0 && AT::lastExitNs.load()==0);
    assert(native.read(a.data(),6)==4);
    petari_audio_trace_read_off_probe(&observed,b.data(),6);
    assert(a==b && native.level()==observed.level() && native.target()==observed.target());
    assert(native.largestRequest()==observed.largestRequest());
    assert(native.takeUnderrunFrames()==2 && petari_audio_trace_report_off_probe(&observed)==2);
    assert(petari_audio_trace_report_off_probe(&observed)==0);
    assert(GT::run==nullptr && GT::threadId==0 && GT::producerSequence==0);
    char temporary[]="/tmp/petari-audio-trace-XXXXXX";assert(mkdtemp(temporary));
    const std::string path=std::string(temporary)+"/trace.jsonl";
    assert(GT::configure("1",path.c_str(),"0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"));
    native.reset(0); observed.reset(0); native.write(input.data(),4); observed.write(input.data(),4);
    AT::callbackBeginEnabled(observed,24,0);
    native.read(a.data(),6); AT::read(observed,b.data(),6);
    AT::callbackEndEnabled(observed,1);
    assert(AT::activeCallbacks.load()==0 && AT::callbackState.depth==0 && AT::callbackState.span==0);
    assert(a==b && observed.level()==native.level());
    assert(AT::takeUnderruns(observed)==2 && native.takeUnderrunFrames()==2);
    assert(AT::takeUnderruns(observed)==0); // same destructive exchange; no extra exchange for tracing
    AT::callbackBeginEnabled(observed,12,1);
    native.read(a.data(),3); AT::read(observed,b.data(),3);
    AT::callbackEndEnabled(observed,1);
    assert(a==b && AT::takeUnderruns(observed)==3 && native.takeUnderrunFrames()==3);
    assert(AT::takeUnderruns(observed,"shutdown")==0);
    GT::shutdown(); assert(GT::run->ring.drops.load()==0 && GT::run->ioErrors.load()==0);
    std::ifstream stream(path);const std::string text((std::istreambuf_iterator<char>(stream)),std::istreambuf_iterator<char>());
    assert(text.find("audio_device_callback_entry")!=std::string::npos);
    assert(text.find("audio_device_callback_exit")!=std::string::npos);
    assert(text.find("\"realtime_api_status\":1")!=std::string::npos);
    assert(text.find("\"ring_level_frames\":4")!=std::string::npos);
    assert(text.find("\"protocol_error\":true")==std::string::npos);
    assert(text.find("\"callback_span\":0")==std::string::npos);
    assert(text.find("audio_device_ring_shortfall")!=std::string::npos);
    assert(text.find("audio_underrun_report_exchange")!=std::string::npos);
    assert(text.find("\"requested_frames\":6,\"copied_frames\":4")!=std::string::npos);
    assert(text.find("\"missing_frames\":2")!=std::string::npos);
    assert(text.find("\"missing_frames\":3")!=std::string::npos);
    assert(text.find("\"report_source\":\"shutdown\"")!=std::string::npos);
    assert(text.find("\"complete\":true")!=std::string::npos);
    stream.close();std::remove(path.c_str());rmdir(temporary);
}
