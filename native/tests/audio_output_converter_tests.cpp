// CPU-only AudioConverter; no AudioComponent, device open or default setter.
#include "../audio/output_au_converter.hpp"
#include <cassert>
#include <cstdio>
#include <vector>
using namespace PetariNative::AudioOutputAU::Detail;
static std::vector<float> track(unsigned guest,double device,unsigned channels,unsigned chunk) {
    Converter converter;
    const bool opened=converter.open(guest,device,channels,4096);
    if(!opened) std::fprintf(stderr,"converter open guest=%u device=%.0f channels=%u error=%d\n",guest,device,channels,int(converter.error()));
    assert(opened);
    assert(converter.prewarm());
    Ring ring; assert(ring.configure(16384,0,channels));
    const unsigned total=guest/10;
    std::vector<float> result;
    std::array<std::int16_t,512> input{};
    std::array<float,4096> l{},r{};
    auto drain=[&] {
        const auto count=ring.level(); assert(count<=l.size());
        assert(ring.read(l.data(),channels==2 ? r.data() : nullptr,count,false)==count);
        for(unsigned n=0;n<count;++n) { result.push_back(l[n]); if(channels==2) result.push_back(r[n]); }
    };
    for(unsigned sent=0;sent<total;) {
        const auto count=std::min(chunk,total-sent);
        for(unsigned n=0;n<count;++n) {
            // DC inside the track, distinct channel signs, no full-scale clip.
            input[n*2]=8192; input[n*2+1]=-4096;
        }
        assert(converter.push(input.data(),count));
        assert(converter.convert(ring)); drain();
        assert(!converter.hasPendingInput()); sent+=count;
    }
    for(unsigned n=0;n<16;++n) {
        const auto before=converter.output(); assert(converter.convert(ring,true)); drain();
        if(before==converter.output()) break;
        assert(n!=15);
    }
    assert(converter.consumed()==total && converter.output()==std::uint64_t(device/10));
    assert(result.size()==converter.output()*channels);
    for(unsigned n=100;n+100<result.size()/channels;++n) {
        if(channels==2) { assert(std::fabs(result[n*2]-.25f)<.0001f); assert(std::fabs(result[n*2+1]+.125f)<.0001f); }
        else assert(std::isfinite(result[n]) && std::fabs(result[n])<=.5f);
    }
    return result;
}
int main() {
    for(auto guest:{32000u,48000u}) for(auto rate:{8000.,16000.,32000.,44100.,48000.,96000.}) for(auto channels:{1u,2u}) {
        const auto a=track(guest,rate,channels,32),b=track(guest,rate,channels,117);
        assert(a.size()==b.size());
        for(unsigned n=0;n<a.size();++n) assert(std::fabs(a[n]-b[n])<.000001f);
    }
    Converter converter; assert(converter.open(32000,44100,2,4096));
    Ring ring; assert(ring.configure(4096,0,2));
    assert(converter.convert(ring) && converter.output()==0); // temporary, not EOF
    std::array<std::int16_t,64> zero{};
    assert(converter.push(zero.data(),32)); assert(!converter.push(zero.data(),32));
    assert(converter.convert(ring)); assert(converter.consumed()==32);
    assert(converter.convert(ring)); // can resume after another temporary empty
    assert(converter.push(zero.data(),32)); assert(converter.convert(ring)); assert(converter.consumed()==64);
    std::puts("producer AudioConverter finite flush / temporary input / 32-48k SRC / chunk invariance PASS");
}
