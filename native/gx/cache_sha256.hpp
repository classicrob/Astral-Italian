#pragma once
#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <cstring>
#include <span>
#include <string>
#include <string_view>

namespace PetariPipelineCache {
// SHA-256 over cache bytes, independent of ABI, checkout and host libraries.
class Sha256 {
    std::array<std::uint32_t,8> state{0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};
    std::array<unsigned char,64> block{};
    std::uint64_t length=0;
    std::size_t used=0;
    void transform() {
        constexpr std::uint32_t k[]{0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};
        std::uint32_t w[64];
        for(unsigned i=0;i<16;++i)w[i]=(std::uint32_t(block[4*i])<<24)|(std::uint32_t(block[4*i+1])<<16)|(std::uint32_t(block[4*i+2])<<8)|block[4*i+3];
        for(unsigned i=16;i<64;++i)w[i]=w[i-16]+(std::rotr(w[i-15],7)^std::rotr(w[i-15],18)^(w[i-15]>>3))+w[i-7]+(std::rotr(w[i-2],17)^std::rotr(w[i-2],19)^(w[i-2]>>10));
        auto a=state[0],b=state[1],c=state[2],d=state[3],e=state[4],f=state[5],g=state[6],h=state[7];
        for(unsigned i=0;i<64;++i){auto t=h+(std::rotr(e,6)^std::rotr(e,11)^std::rotr(e,25))+((e&f)^(~e&g))+k[i]+w[i];auto u=(std::rotr(a,2)^std::rotr(a,13)^std::rotr(a,22))+((a&b)^(a&c)^(b&c));h=g;g=f;f=e;e=d+t;d=c;c=b;b=a;a=t+u;}
        state[0]+=a;state[1]+=b;state[2]+=c;state[3]+=d;state[4]+=e;state[5]+=f;state[6]+=g;state[7]+=h;
    }
public:
    void update(const void* data,std::size_t count) {
        auto p=static_cast<const unsigned char*>(data);length+=count;
        while(count){auto take=std::min(count,64-used);std::memcpy(block.data()+used,p,take);used+=take;p+=take;count-=take;if(used==64){transform();used=0;}}
    }
    void update(std::string_view value){update(value.data(),value.size());}
    std::string finish() {
        const auto bits=length*8;const unsigned char first=0x80,zero=0;update(&first,1);
        while(used!=56)update(&zero,1);
        unsigned char size[8];for(unsigned i=0;i<8;++i)size[i]=bits>>(56-8*i);update(size,8);
        constexpr char hex[]="0123456789abcdef";std::string result;result.reserve(64);
        for(auto word:state)for(int i=7;i>=0;--i)result+=hex[(word>>(i*4))&15];return result;
    }
};
inline std::string digest(const void* bytes,std::size_t size){Sha256 h;h.update(bytes,size);return h.finish();}
inline std::string digest(std::string_view value){return digest(value.data(),value.size());}
inline void hashNumber(Sha256& h,std::uint64_t value,unsigned bytes){for(unsigned i=0;i<bytes;++i){unsigned char b=value>>(8*i);h.update(&b,1);}}
} // namespace PetariPipelineCache
