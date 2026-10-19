#include "replay.hpp"
#include "trace.hpp"
#include "gx_ledger.hpp"
#include "fp_mode.hpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <sstream>
#include <thread>
using namespace PetariNative::Headless;
namespace {
int checks=0;
void check(bool value,const char* message) {++checks;if(!value){std::fprintf(stderr,"headless test: %s\n",message);std::exit(1);}}
template<class F> void rejects(F call,const char* message) {try{call();}catch(const ReplayError&){++checks;return;}check(false,message);}
void number(std::ostream& out,std::uint64_t value,unsigned size) {for(unsigned i=size;i>0;--i)out.put(static_cast<char>(value>>(8*(i-1))));}
std::string tape() {
    std::ostringstream out;out<<"SMG2KPD1";number(out,2,8);number(out,2,8);
    for(unsigned frame=0;frame<2;++frame){
        number(out,frame,8);number(out,0,4);number(out,0,4);number(out,120,4);
        number(out,frame?0:1,4);number(out,frame?0xffffffff:0,4);number(out,0,4);number(out,1,4);number(out,1,4);
        Sample sample{};sample[0]=0x80;sample[236]=0xab;out.write(reinterpret_cast<const char*>(sample.data()),sample.size());
    }
    return out.str();
}
#if defined(__aarch64__)
__attribute__((noinline)) float multiply(volatile float& a,volatile float& b){return a*b;}
#endif
}
int main() {
    FrameClock clock;
    check(clock.advance()==1013512,"NTSC half tick floors the first frame");
    check(clock.advance()==2027025,"NTSC remainder is retained");
    for(unsigned i=2;i<60000;++i)clock.advance();
    check(clock.ticks()==60750000ULL*1001,"rational clock has no 60,000-frame drift");
    FrameClock pal(50,1);check(pal.advance()==1215000,"PAL rate");
    rejects([]{FrameClock invalid(0,1);},"zero frame rate rejected");
    std::istringstream source(tape());Replay replay(source);
    check(replay.frames()==2 && replay.frame()==0,"tape header");
    check(replay.connection(0)->device==1 && !replay.connection(3),"connection state comes from the tape");
    rejects([&]{replay.consume(1,120);},"channel mismatch");
    rejects([&]{replay.consume(0,16);},"capacity mismatch");
    rejects([&]{replay.endFrame();},"unconsumed reads");
    const auto& first=replay.consume(0,120);check(first.returned==1 && first.samples[0][0]==0x80,"sample bits preserved");
    rejects([&]{replay.consume(0,120);},"extra read");replay.endFrame();
    const auto& zero=replay.consume(0,120);check(zero.returned==0 && zero.error==-1 && zero.samples[0][236]==0xab,"zero return still carries slot zero and error");
    replay.endFrame();check(replay.finished(),"all frames and reads consumed");
    rejects([&]{replay.endFrame();},"extra frame");
    auto truncated=tape();truncated.pop_back();rejects([&]{std::istringstream in(truncated);Replay bad(in);},"truncated tape");
    auto trailing=tape()+"x";rejects([&]{std::istringstream in(trailing);Replay bad(in);},"trailing bytes");
    auto corrupt=tape();corrupt[8+8+8+8+4+3]=4;rejects([&]{std::istringstream in(corrupt);Replay bad(in);},"invalid channel");
    GxLedger fifo;std::uint8_t storage[64]{};const std::uint8_t data[3]={0x61,0x12,0x34};
    fifo.write(data,3);check(fifo.bytes()==3,"direct FIFO byte count");
    fifo.begin(storage,64);fifo.write(data,3);check(fifo.end()==32,"display list padding");
    check(!std::memcmp(storage,data,3) && storage[31]==0 && fifo.bytes()==3,"display-list capture does not double-count direct FIFO");
    rejects([&]{fifo.end();},"unbalanced display-list end");
    fifo.begin(storage,2);rejects([&]{fifo.write(data,3);},"display-list overflow");
    State state;state.integer("player_valid",0);state.text("stage","A,\"B");state.real("x",-0.0f);
    check(state.fields.at("x_bits")=="0x80000000","signed-zero bits");
    std::uint32_t bits=0x3f800001;float one;std::memcpy(&one,&bits,4);state.real("y",one);
    float roundtrip=std::stof(state.fields.at("y"));check(!std::memcmp(&one,&roundtrip,4),"float32 round-trip decimal");
    rejects([&]{state.real("x",std::numeric_limits<float>::quiet_NaN());},"NaN trace fails");
    rejects([&]{state.text("invented","0");},"schema refuses unknown column");
    std::ostringstream csv;Trace trace(csv);trace.write(0,state);trace.finish();
    check(csv.str().find("\"A,\"\"B\"")!=std::string::npos,"CSV escaping");
    rejects([&]{trace.write(2,state);},"trace cannot skip frames");
#if defined(__aarch64__)
    const bool original=FloatingPointMode::flushToZero();
    volatile float normal=std::numeric_limits<float>::min(),half=0.5f;
    {FloatingPointMode gradual(false);check(multiply(normal,half)>0,"gradual underflow switch");}
    {FloatingPointMode flush(true);check(multiply(normal,half)==0,"FPCR.FZ flushes a denormal product");
     bool inherited=false;std::thread child([&]{inherited=FloatingPointMode::flushToZero() && multiply(normal,half)==0;});child.join();check(inherited,"new pthread inherits FZ");}
    check(FloatingPointMode::flushToZero()==original,"FPCR policy restores caller state");
#endif
    std::printf("headless core: %d checks passed\n",checks);
}
