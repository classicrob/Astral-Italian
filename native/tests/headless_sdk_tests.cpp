#include "platform.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
using namespace PetariNative::Headless;
void check(bool value,const char* message){if(!value){std::fprintf(stderr,"%s\n",message);std::exit(1);}}
void word(Sample& bytes,unsigned offset,unsigned value){for(unsigned i=0;i<4;++i)bytes[offset+i]=static_cast<unsigned char>(value>>(24-i*8));}
int main(){
    Sample bytes{};bytes[92]=1;bytes[93]=0xff;bytes[94]=0xff;bytes[95]=5;
    word(bytes,0,0x80000800);word(bytes,4,0x0800);word(bytes,8,0x4000);
    word(bytes,12,0x80000000);word(bytes,16,0xbf800000);word(bytes,96,0x3f000000);
    word(bytes,100,0xbf000000);word(bytes,176,0x3f800000);word(bytes,236,0xabcdef12);
    auto sample=decodeSample(bytes);
    check(sample.hold==0x80000800 && sample.trig==0x0800 && sample.release==0x4000,"button words");
    check(sample.wpad_err==-1 && sample.dpd_valid_fg==-1 && sample.data_format==5,"signed and byte fields");
    unsigned zero;std::memcpy(&zero,&sample.acc.x,4);check(zero==0x80000000,"negative zero survives decode");
    check(sample.acc.y==-1 && sample.ex_status.fs.stick.x==0.5f && sample.ex_status.fs.stick.y==-0.5f,"Nunchuk union");
    check(sample.mpls.angularVelocity.x==1 && sample.unk_EC==0xabcdef12,"SMG2 trailing fields");
    bytes[92]=2;word(bytes,96,123);word(bytes,108,0x3f400000);sample=decodeSample(bytes);
    check(sample.ex_status.cl.hold==123 && sample.ex_status.cl.lstick.x==0.75f,"Classic union");
    bytes[92]=255;try{decodeSample(bytes);check(false,"unknown extension must fail");}catch(const ReplayError&){}
    std::puts("headless KPAD field decoder checks passed");
}
