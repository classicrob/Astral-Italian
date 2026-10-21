// Synthetic native header controls; NOT a game/parser replay. Compile only by allocated T2.
#include <array>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/mman.h>
static bool force_io_failure=false;
static unsigned capture_calls=0,error_calls=0;
static ssize_t p_ff_test_write(int fd,const void* data,size_t size) {
  if(fd==STDERR_FILENO){++error_calls;return static_cast<ssize_t>(size);}
  ++capture_calls;
  if(force_io_failure){errno=EINTR;return -1;}
  return ::write(fd,data,size);
}
#define write p_ff_test_write
#ifndef P_FIFO_CAPTURE_HEADER
#error Supply reviewed exact generated header using P_FIFO_CAPTURE_HEADER
#endif
#include P_FIFO_CAPTURE_HEADER
#undef write
namespace FF=PWorkerFirstFault;
static void check(bool good,const char* why){if(!good){std::fprintf(stderr,"CONTROL FAIL %s\n",why);std::exit(97);}}
struct A {const void* data=nullptr;unsigned size=0;unsigned char stride=0;bool le=false;};
struct V {unsigned cnt=0,type=0,frac=0;};
struct F {std::array<V,32> attrs{};};
struct GX {std::array<A,32> arrays{};std::array<unsigned,32> vtxDesc{};std::array<F,8> vtxFmts{};unsigned lastVtxFmt=8,lastVtxSize=0;};
struct Reader {
  std::array<unsigned char,70000> bytes{};std::size_t pos=0;
  const unsigned char* data()const{return bytes.data();}
  std::size_t size()const{return bytes.size();}
  std::size_t offset()const{return pos;}
  std::size_t remaining()const{return size()-pos;}
};
static GX gx{};static Reader reader{};
struct EdgeReader {
  const unsigned char* ptr;std::size_t extent,pos=0;
  const unsigned char* data()const{return ptr;}
  std::size_t size()const{return extent;}
  std::size_t offset()const{return pos;}
  std::size_t remaining()const{return pos<=extent?extent-pos:0;}
};
static void early_span(const unsigned char* ptr){EdgeReader r{ptr,1};FF::SpanScope<EdgeReader> span(r,gx);return;}
static void ranges(){
  const long page=::sysconf(_SC_PAGESIZE);check(page>0,"page size");
  const std::size_t n=static_cast<std::size_t>(page);
  void* mapping=::mmap(nullptr,n*2,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANON,-1,0);
  check(mapping!=MAP_FAILED,"guard allocation");
  auto* bytes=static_cast<unsigned char*>(mapping);check(::mprotect(bytes+n,n,PROT_NONE)==0,"guard protection");
  bytes[n-1]=0x41;const auto* edge=bytes+n-1;
  {EdgeReader r{edge,0};FF::SpanScope<EdgeReader> span(r,gx);check(FF::state.enabled,"range capture setup");
    FF::command(0x40,0,0);check(FF::state.current.n==0,"zero span must not read");
    FF::command(0x40,1,0);check(FF::state.current.n==0&&(FF::state.limits&FF::BadRange),"outside zero span");
    FF::cp(0xbf,0,gx);FF::cp(0xbf,1,gx);}
  check(FF::state.data==nullptr&&!FF::state.active,"zero span RAII");
  {EdgeReader r{edge,1};FF::SpanScope<EdgeReader> span(r,gx);
    FF::command(0x41,0,0);check(FF::state.current.n==1&&FF::state.current.bytes[0]==0x41,"last byte before PROT_NONE");
    FF::command(0x41,1,0);check(FF::state.current.n==0,"end boundary");
    FF::command(0x41,2,0);check(FF::state.current.n==0,"outside short span");
    FF::cp(0xbf,0,gx);FF::cp(0xbf,3,gx);FF::cp(0xbf,4,gx);FF::cp(0xbf,2,gx);
    FF::PrimitiveScope<EdgeReader> badfmt(r,0x90,99,1,gx);
    check((FF::state.limits&FF::BadFormat)&&!FF::state.pending,"invalidfmt no extra array read");}
  check(FF::state.data==nullptr&&!FF::state.active,"short span RAII");
  early_span(edge);check(FF::state.data==nullptr&&!FF::state.active,"early return RAII");
  check(capture_calls==0,"range probes must not trigger output");
  check(::munmap(mapping,n*2)==0,"guard cleanup");
  std::puts("CONTROL PASS mode=ranges zero/short/guard/end/outside/cp/fmt/early-return");
}
int main(int argc,char** argv){
  if(argc==2&&std::strcmp(argv[1],"--describe")==0){std::puts("P_FIFO_WORKER_NATIVE_CONTROL_v1");return 0;}
  check(argc==2,"mode required");if(std::strcmp(argv[1],"ranges")==0){ranges();return 0;}const bool off=std::strcmp(argv[1],"off")==0;
  const bool setupfail=std::strcmp(argv[1],"setupfail")==0;
  force_io_failure=std::strcmp(argv[1],"io")==0;
  check(off||setupfail||force_io_failure||std::strcmp(argv[1],"on")==0,"unknown mode");
  {
    FF::SpanScope<Reader> span(reader,gx);
    if(off||setupfail){FF::command(0x41,0,reader.remaining());FF::warning();check(!FF::state.enabled&&FF::state.fd<0,"must be disabled");check(FF::state.commands==0&&capture_calls==0,"OFF/setupfail must not capture");check(error_calls==(setupfail?1u:0u),"setup status");}
    else{
      check(FF::state.enabled,"capture setup");FF::command(0x90,0,reader.remaining());
      for(unsigned i=0;i<3;++i){FF::PrimitiveScope<Reader> primitive(reader,0x90,0,3,gx);FF::primitive_bytes(4,12);reader.pos+=12;}
      check(FF::state.primitives==3&&(FF::state.limits&FF::RingLoss),"primitive eviction must be explicit before64 commands");
      for(unsigned i=0;i<33;++i)FF::cp(0xbfu,reader.offset(),gx);
      reader.bytes[69900]=0x41;reader.bytes[69901]=0x39;
      for(unsigned i=0;i<70;++i)FF::command(0x41,69900,99);
      FF::native_array(0xffffu,24,gx.arrays[24]);
      const unsigned before=capture_calls,errors=error_calls;FF::warning();
      check(FF::state.warning,"warning latch");check(capture_calls-before<=15&&error_calls-errors<=1,"16 total writes maximum");
      if(force_io_failure)check(capture_calls-before==15&&error_calls-errors==1&&(FF::state.limits&FF::IoFailure),"forced I/O failure");
      const unsigned after=capture_calls;FF::warning();check(capture_calls==after,"warning once");
      check(FF::state.limits&FF::RawLimit,"raw window capped");
      // Deliberately exhaust only diagnostic text capacity; never write beyond it.
      for(unsigned i=0;i<FF::MaxOutput+3;++i)FF::put('x');
      check(FF::state.used<=FF::MaxOutput&&(FF::state.limits&FF::OutputLimit),"text capacity bounded");
      FF::command(0x39,69901,98);
      const unsigned b2=capture_calls,e2=error_calls;FF::indexed(24,3,0x600,4,0,16,gx.arrays[24]);
      check(FF::state.fault,"fault latch");check(capture_calls-b2<=15&&error_calls-e2<=1,"fault16 total writes maximum");
      const unsigned after2=capture_calls;FF::indexed(24,3,0x600,4,0,16,gx.arrays[24]);check(capture_calls==after2,"fault once");
      check(FF::state.used<=FF::MaxOutput,"output storage bounded");
    }
  }
  check(FF::state.data==nullptr&&FF::state.size==0&&!FF::state.active,"span lifetime cleared");
  std::printf("CONTROL PASS mode=%s captureCalls=%u errorCalls=%u limits=%u\n",argv[1],capture_calls,error_calls,FF::state.limits);
  return 0;
}
