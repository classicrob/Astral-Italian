// Included after the actual production PadRecorder.cpp by padrecorder_wire_tests.py.
// Only scene registration, cache maintenance, allocation and save-file sinks are
// inert test providers; recorder/readers/decompression/state methods are real.
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <new>
#include <cstddef>
#include <cstdint>

namespace {
unsigned checks;
bool padMode = true;
unsigned saveLoads;
std::vector<u8> saveBytes;
void check(bool ok, const char* why) {
    ++checks;
    if (!ok) { std::fprintf(stderr, "FAIL %s\n", why); std::exit(1); }
}
u32 be32(const u8* p) { return (u32(p[0]) << 24) | (u32(p[1]) << 16) | (u32(p[2]) << 8) | p[3]; }
void put32(u8* p, u32 n) { p[0] = n >> 24; p[1] = n >> 16; p[2] = n >> 8; p[3] = n; }
template <class T> u64 bits(const T& value) {
    static_assert(sizeof(T) == 4 || sizeof(T) == 8, "scalar width");
    if constexpr (sizeof(T) == 4) { u32 n; std::memcpy(&n, &value, 4); return n; }
    else { u64 n; std::memcpy(&n, &value, 8); return n; }
}
void vec(const Vec& v, const u8* p) {
    check(bits(v.x) == be32(p) && bits(v.y) == be32(p+4) && bits(v.z) == be32(p+8), "typed Vec scalar representations");
}
void vec2(const Vec2& v, const u8* p) {
    check(bits(v.x) == be32(p) && bits(v.y) == be32(p+4), "typed Vec2 scalar representations");
}
using Wire = std::array<u8, 0xF0>;
Wire status(u8 device, unsigned seed) {
    Wire w{};
    for (unsigned i=0; i<w.size(); ++i) w[i]=u8(i*29+seed);
    put32(w.data(), 0x80000800u | seed); put32(w.data()+4, 0x400u | seed); put32(w.data()+8, 0x100u | seed);
    // Exact scalar payloads include signed zero, infinities and NaNs. No FP math.
    const u32 special[] = {0x3f800000, 0xbf000000, 0x80000000, 0x7f800000, 0xff800000, 0x7fc12345, 0x7fa12345};
    for (unsigned i=0x0C; i<0x5C; i+=4) put32(w.data()+i,special[((i-12)/4+seed)%7]);
    w[0x5C]=device; w[0x5D]=u8(-1); w[0x5E]=u8(-2); w[0x5F]=16;
    for (unsigned i=0xB0; i<0xF0; i+=4) put32(w.data()+i,0x3e000000u+(i<<8)+seed);
    return w;
}
void verify(const KPADStatus& s, const Wire& w) {
    const u8* p=w.data();
    check(s.hold==be32(p)&&s.trig==be32(p+4)&&s.release==be32(p+8), "hold/trig/release host words");
    vec(s.acc,p+0x0C);check(bits(s.acc_value)==be32(p+0x18)&&bits(s.acc_speed)==be32(p+0x1C),"acc scalars");
    vec2(s.pos,p+0x20);vec2(s.vec,p+0x28);check(bits(s.speed)==be32(p+0x30),"pointer speed");
    vec2(s.horizon,p+0x34);vec2(s.hori_vec,p+0x3C);
    check(bits(s.hori_speed)==be32(p+0x44)&&bits(s.dist)==be32(p+0x48)&&bits(s.dist_vec)==be32(p+0x4C)&&bits(s.dist_speed)==be32(p+0x50),"horizon/distance scalars");
    vec2(s.acc_vertical,p+0x54);
    check(s.dev_type==p[0x5C]&&u8(s.wpad_err)==p[0x5D]&&u8(s.dpd_valid_fg)==p[0x5E]&&s.data_format==p[0x5F],"byte-sized status fields unchanged");
    if (s.dev_type==1 || s.dev_type==6) {
        vec2(s.ex_status.fs.stick,p+0x60);vec(s.ex_status.fs.acc,p+0x68);
        check(bits(s.ex_status.fs.acc_value)==be32(p+0x74)&&bits(s.ex_status.fs.acc_speed)==be32(p+0x78),"freestyle scalars");
        check(std::memcmp(reinterpret_cast<const u8*>(&s.ex_status)+28,p+0x7C,52)==0,"inactive freestyle union bytes unchanged");
    } else if (s.dev_type==2 || s.dev_type==7 || (s.dev_type>=16&&s.dev_type<=19)) {
        const auto& c=s.ex_status.cl;
        check(c.hold==be32(p+0x60)&&c.trig==be32(p+0x64)&&c.release==be32(p+0x68),"classic button words");
        vec2(c.lstick,p+0x6C);vec2(c.rstick,p+0x74);
        check(bits(c.ltrigger)==be32(p+0x7C)&&bits(c.rtrigger)==be32(p+0x80),"classic triggers");
        check(std::memcmp(reinterpret_cast<const u8*>(&s.ex_status)+36,p+0x84,44)==0,"inactive classic union bytes unchanged");
    } else if (s.dev_type==3) {
        const auto expected=[&](unsigned at){return (u64(be32(p+at))<<32)|be32(p+at+4);};
        check(bits(s.ex_status.bl.tgc_weight)==expected(0x60),"board tgc double representation");
        for (unsigned i=0;i<4;++i) {
            check(bits(s.ex_status.bl.weight[i])==expected(0x68+i*8),"board weight double representation");
            check(bits(s.ex_status.bl.weight_ave[i])==expected(0x88+i*8),"board averaged double representation");
        }
        check(bits(s.ex_status.bl.weight_err)==be32(p+0xA8)&&bits(s.ex_status.bl.tgc_weight_err)==be32(p+0xAC),"board signed error words");
    } else check(std::memcmp(&s.ex_status,p+0x60,0x50)==0,"inactive extension wire bytes unchanged");
    vec(s.mpls.angularVelocity,p+0xB0);vec(s.mpls.angle,p+0xBC);vec(s.mpls.direction.X,p+0xC8);
    vec(s.mpls.direction.Y,p+0xD4);vec(s.mpls.direction.Z,p+0xE0);check(s.unk_EC==be32(p+0xEC),"trailing status word");
}
std::vector<u8> delta(const Wire& w, Wire& prev) {
    std::vector<u8> out(0x21,0);
    out[30]=0xA5;out[31]=0x5A;out[32]=0xED; // reserved mask bytes are not sample bytes
    for (unsigned i=0;i<w.size();++i) if (w[i]!=prev[i]) {out[i/8]|=1u<<(i%8);out.push_back(w[i]);}
    prev=w;return out;
}
void packet(std::vector<u8>& out,u8 seq,const std::vector<Wire>& frames,Wire& prev) {
    std::vector<u8> data;
    for (const auto& w:frames) {auto d=delta(w,prev);data.insert(data.end(),d.begin(),d.end());}
    check(data.size()<65536&&frames.size()<256,"fixture packet widths");
    const auto pos=out.size();out.resize(pos+4);put32(out.data()+pos,(u32(frames.size())<<24)|(u32(data.size())<<8)|seq);
    out.insert(out.end(),data.begin(),data.end());
}
std::vector<u8> recording(const std::vector<Wire>& first,const std::vector<Wire>& second) {
    std::vector<u8> out(0x40,0);
    put32(out.data(),0x40);put32(out.data()+4,0x12345678);put32(out.data()+8,0xffffffff);
    out[12]=0xA1;out[13]=0xB2;out[14]=0xC3;out[15]=0xD4;put32(out.data()+16,0x80000000);put32(out.data()+20,0x89abcdef);
    for (unsigned i=24;i<64;++i) out[i]=u8(i*7);
    out.resize(0x44);put32(out.data()+0x40,17);
    for (unsigned i=0;i<17;++i) out.push_back(u8(i*11));
    Wire prev{};packet(out,0,first,prev);packet(out,1,second,prev);packet(out,2,{},prev);packet(out,99,{},prev);
    return out;
}
struct Recorder : PadRecorder {
    ~Recorder() {
        delete mDefaultReader;delete mHeader;
        std::free(mPacketBufferAlloc);std::free(mData);std::free(mPrevStatus[0]);std::free(mPrevStatus[1]);
    }
};
void replay() {
    Recorder rec;
    auto a=status(1,1),b=a,c=a;
    // Byte-level changes within a 32-bit word make swapped-history bugs visible.
    b[1]^=0x20;b[0x61]^=0x11;b[0xE3]^=0x23;c=b;c[2]^=0x40;c[0x62]^=0x70;
    auto tape=recording({a,b},{c});
    check(rec.mState==0&&rec.replayFrame(0,nullptr)==-1,"stopped read leaves output alone");
    rec.prepareReplay(reinterpret_cast<const char*>(tape.data()),tape.size());
    check(rec.mState==3&&rec.mReader!=nullptr,"valid BE recording reaches prepared state");
    check(rec.mHeader->mHeaderSize==64&&bits(rec.mHeader->_4)==0x12345678&&bits(rec.mHeader->_8)==0xffffffff&&bits(rec.mHeader->_10)==0x80000000&&bits(rec.mHeader->_14)==0x89abcdef,"all typed header words decoded");
    check(rec.mHeader->_C==0xA1&&rec.mHeader->_D==0xB2&&rec.mHeader->_E==0xC3&&rec.mHeader->_F==0xD4&&std::memcmp(rec.mHeader->_18,tape.data()+24,40)==0,"header byte payload unchanged");
    const u32 firstSize=(be32(tape.data()+0x55)>>8)&65535;
    check(rec.mPacketNo==1&&rec.mStatusNum==2&&rec.mDataSize==firstSize&&rec.mReadPos==4+firstSize,"packet count/size/sequence and priming read position");
    check(rec.startReplay()&&rec.mState==2&&rec._48==300,"prepared-to-playing state");
    std::array<KPADStatus,3> out;std::memset(out.data(),0xA5,sizeof(out));
    std::array<u8,sizeof(KPADStatus)> sentinel;std::memcpy(sentinel.data(),&out[2],sentinel.size());
    check(rec.replayFrame(0,out.data())==2,"packet zero emits two statuses");verify(out[0],a);verify(out[1],b);
    check(std::memcmp(&out[2],sentinel.data(),sentinel.size())==0,"sample count respects output extent");
    check(std::memcmp(rec.mPrevStatus[0],b.data(),b.size())==0,"history stays wire order after two statuses");
    const Wire zeros{};check(std::memcmp(rec.mPrevStatus[1],zeros.data(),zeros.size())==0,"other channel history remains independent");
    const u32 secondSize=(be32(tape.data()+0x59+firstSize)>>8)&65535;
    check(rec.mPacketNo==2&&rec.mStatusNum==1&&rec.mDataSize==secondSize&&rec.mReadPos==8+firstSize+secondSize,"next packet metadata and progression");
    check(rec.replayFrame(0,out.data())==1,"second packet emits one byte-delta status");verify(out[0],c);
    check(std::memcmp(rec.mPrevStatus[0],c.data(),c.size())==0,"history remains exact after cross-frame byte delta");
    check(rec.mPacketNo==3&&rec.mStatusNum==0&&rec.mDataSize==0&&rec.mState==2,"zero-status next packet preserved");
    std::array<u8,sizeof(out)> before;std::memcpy(before.data(),out.data(),before.size());
    check(rec.replayFrame(0,out.data())==0&&std::memcmp(before.data(),out.data(),before.size())==0,"zero samples leave output unchanged");
    check(rec.mState==0&&rec.mReader==nullptr&&rec.mStatusNum==0,"bad sequence stops through original lifecycle");
    rec.prepareReplay(reinterpret_cast<const char*>(tape.data()),tape.size());
    check(rec.mState==3&&rec.mPacketNo==1&&std::memcmp(rec.mPrevStatus[0],zeros.data(),zeros.size())==0,"restart resets raw delta history and primes packet zero");
    rec.stopReplay(); // original prepared state intentionally stays prepared
    check(rec.mState==3,"stopReplay prepared-state behavior unchanged");
} 
void variants() {
    Recorder rec;
    for (unsigned sequence=0;sequence<256;++sequence)for(unsigned count:{0u,1u,12u,255u})for(unsigned size:{0u,1u,0x1234u,0xffffu}) {
        u32 gotSequence,gotSize;s32 gotCount;
        rec.parsePacketHeader(&gotSequence,&gotSize,&gotCount,(count<<24)|(size<<8)|sequence);
        check(gotSequence==sequence&&gotSize==size&&gotCount==s32(count),"all packet field widths and unsigned extraction");
    }

    for (u8 device:{u8(0),u8(1),u8(2),u8(3),u8(5),u8(6),u8(7),u8(16),u8(17),u8(18),u8(19),u8(250),u8(255)}) {
        Wire old{},encodePrev{};KPADStatus history{},out{};auto a=status(device,7);
        auto data=delta(a,encodePrev);
        check(rec.decompress(&out,data.data(),&history)==data.data()+data.size(),"full decompressed byte cursor");verify(out,a);
        check(std::memcmp(&history,a.data(),a.size())==0,"full sample raw history");
        // Every byte can be independently changed by the next delta. This also
        // checks that inactive extension payloads are not read as float scalars.
        for (unsigned i=0;i<a.size();++i) {
            auto b=a;b[i]^=0x40;auto d=delta(b,encodePrev);
            check(rec.decompress(&out,d.data(),&history)==d.data()+d.size(),"single-byte delta cursor");verify(out,b);
            check(std::memcmp(&history,b.data(),b.size())==0,"single-byte delta preserves wire history");
            a=b;
        }
    }
    // Same typed playback, exercising the non-PadMode save sink without claiming
    // that the actual game save-data parser is present in this focused harness.
    padMode=false;auto w=status(1,3);auto tape=recording({w},{});
    rec.prepareReplay(reinterpret_cast<const char*>(tape.data()),tape.size());
    check(rec.mState==3&&saveLoads==1&&saveBytes.size()==17,"decoded user-file length reaches save sink");
    for (unsigned i=0;i<saveBytes.size();++i)check(saveBytes[i]==u8(i*11),"user-file bytes unchanged");
    padMode=true;
    Recorder invalid;put32(tape.data(),0x41);invalid.prepareReplay(reinterpret_cast<const char*>(tape.data()),tape.size());
    check(invalid.mReader==nullptr&&invalid._4A==90&&invalid.mState==3,"invalid header follows original failure branch");
    Recorder rollover;auto empty=recording({},{});empty.resize(0x55);Wire previous{};
    for (unsigned i=0;i<300;++i)packet(empty,u8(i),{},previous);
    packet(empty,0xff,{},previous);
    rollover.prepareReplay(reinterpret_cast<const char*>(empty.data()),empty.size());
    check(rollover.startReplay(),"empty packet stream starts");
    for (unsigned i=1;i<300;++i)check(rollover.replayFrame(0,nullptr)==0&&rollover.mState==2,"packet sequence wraps modulo byte width");
    check(rollover.mPacketNo==300&&rollover.replayFrame(0,nullptr)==0&&rollover.mState==0,"mismatched sequence after wrap stops");
}
}

// Focused-test dependencies, using the real declarations and no ABI stand-ins.
JMapLinkInfo::JMapLinkInfo():mFollowID(-1),mZoneID(-1),mContext(-1) {}
NameObj::NameObj(const char* n):mName(n),mFlags(0),mExecutorIdx(-1) {}
NameObj::~NameObj() {}
void NameObj::init(const JMapInfoIter&) {}
void NameObj::initAfterPlacement() {}
void NameObj::movement() {}
void NameObj::draw() const {}
void NameObj::calcAnim() {}
void NameObj::calcViewAndEntry() {}
void NameObj::startMovement() {}
void NameObj::endMovement() {}
void* operator new[](std::size_t n,int alignment) {void* p=nullptr;if(posix_memalign(&p,alignment,n))throw std::bad_alloc();return p;}
extern "C" void DCInvalidateRange(void*,u32) {}
namespace MR {void zeroMemory(void* p,u32 n) {std::memset(p,0,n);}}
namespace GameSequenceFunction {
bool isPadMode() {return padMode;}
u8* getClearedSaveTempBuffer(s32 n) {check(n==17,"save sink receives decoded byte count");saveBytes.assign(n,0);return saveBytes.data();}
void loadSaveTempBufferAsGameData() {++saveLoads;}
}

int main() {replay();variants();std::printf("PASS %u production PadRecorder checks\n",checks);}
