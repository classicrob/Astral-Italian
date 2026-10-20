#include <petari/save_backup.hpp>
#include <cassert>
#include <functional>
#include <iostream>

using namespace PetariNative::SaveWire;
namespace {
void number(Bytes& b, std::size_t at, std::uint64_t value, unsigned width, bool legacy) {
    for (unsigned i=0;i<width;++i) b[at+(legacy?i:width-1-i)]=static_cast<unsigned char>(value>>(i*8));
}
template<std::size_t N> Bytes attributes(const Field (&fields)[N], bool legacy) {
    unsigned size=0; for(auto f:fields)size+=f.width;
    Bytes data(4+N*4+size);
    number(data,0,N,2,legacy);number(data,2,size,2,legacy);
    unsigned offset=0;
    for(unsigned i=0;i<N;++i) {
        number(data,4+i*4,nameHash(fields[i].name),2,legacy);
        number(data,6+i*4,offset,2,legacy);
        number(data,4+N*4+offset,0x123456789abcdef0ULL+i,fields[i].width,legacy);
        offset+=fields[i].width;
    }
    return data;
}
Bytes fixture(bool legacy) {
    Bytes bytes(12448);
    const unsigned offsets[]={128,4096,4192,8160,8256,12224,12320,12448};
    const char* names[]={"user1","config1","user2","config2","user3","config3","sysconf"};
    number(bytes,4,3,4,legacy);number(bytes,8,7,4,legacy);number(bytes,12,bytes.size(),4,legacy);
    for(unsigned record=0;record<7;++record) {
        std::memcpy(bytes.data()+16+record*16,names[record],std::strlen(names[record]));
        number(bytes,28+record*16,offsets[record],4,legacy);
        unsigned at=offsets[record];bytes[at]=2;bytes[at+1]=record==6?1:record%2?3:6;at+=4;
        auto chunk=[&](std::uint32_t sig,Bytes payload) {
            const unsigned header=legacy?16:12;
            number(bytes,at,sig,4,legacy);number(bytes,at+4,headerHash(sig),4,legacy);
            number(bytes,at+8,header+payload.size(),4,legacy);
            std::copy(payload.begin(),payload.end(),bytes.begin()+at+header);at+=header+payload.size();
            assert(at<=offsets[record+1]);
        };
        if(record==6)chunk(0x53595343,attributes(systemFields,legacy));
        else if(record%2) {
            chunk(0x434f4e46,Bytes{0xff});chunk(0x4d494920,Bytes{2,1,2,3,4,5,6,7,8,3});
            Bytes time(8);number(time,0,0x0123456789abcdefULL,8,legacy);chunk(0x4d495343,time);
        } else {
            chunk(0x504c4159,attributes(playerFields,legacy));
            Bytes flags(4);number(flags,0,0x8123,2,legacy);number(flags,2,0x4567,2,legacy);chunk(0x464c4731,flags);
            Bytes lumas(128);for(unsigned i=0;i<64;++i)number(lumas,i*2,0x100+i,2,legacy);chunk(0x53544631,lumas);
            Bytes events(8);for(unsigned i=0;i<4;++i)number(events,i*2,0x200+i,2,legacy);chunk(0x564c4531,events);
            Bytes galaxy(2);number(galaxy,0,1,2,legacy);
            auto g=attributes(galaxyFields,legacy),s=attributes(scenarioFields,legacy);
            Bytes recordData(g.begin()+24,g.end());
            number(recordData,0,0x1234,2,legacy);number(recordData,2,13,2,legacy);recordData[4]=1;
            galaxy.insert(galaxy.end(),g.begin(),g.begin()+24);galaxy.insert(galaxy.end(),s.begin(),s.begin()+16);
            galaxy.insert(galaxy.end(),recordData.begin(),recordData.end());galaxy.insert(galaxy.end(),s.begin()+16,s.end());
            chunk(0x47414c41,galaxy);chunk(0x5353574d,Bytes{1,2,3,4,5,6,7,8,7});
        }
    }
    number(bytes,0,legacy?legacyChecksum(bytes.data()+4,bytes.size()-4):checksum(bytes.data()+4,bytes.size()-4),4,legacy);
    return bytes;
}
void refuses(const std::function<void()>& action) {
    bool refused=false;try{action();}catch(const std::exception&){refused=true;}assert(refused);
}
void writeFixture(const std::filesystem::path& path,const Bytes& data) {
    std::ofstream file(path,std::ios::binary);file.write(reinterpret_cast<const char*>(data.data()),data.size());assert(file.good());
}
}
int main() {
    const auto little=fixture(true),big=fixture(false);
    assert(canonical(big,false)==big);
    assert(canonical(little,true)==big);
    for(std::size_t n:{0u,15u,127u,4095u,12447u})refuses([&]{canonical(Bytes(little.begin(),little.begin()+n),true);});
    auto bad=little;bad[1000]^=1;refuses([&]{canonical(bad,true);});
    for(unsigned at:{8u,28u,128u,129u,140u,148u,150u}) {
        bad=little;bad[at]=0xff;number(bad,0,legacyChecksum(bad.data()+4,bad.size()-4),4,true);
        refuses([&]{canonical(bad,true);});
    }
    unsigned char unaligned[12]{};store<std::int32_t>(unaligned+1,-1234567);assert(load<std::int32_t>(unaligned+1)==-1234567);
    store<std::uint64_t>(unaligned+3,0x123456789abcdef0ULL);assert(load<std::uint64_t>(unaligned+3)==0x123456789abcdef0ULL);
    char temporary[]="/tmp/astra-save-wire-XXXXXX";auto* made=::mkdtemp(temporary);assert(made);
    const std::filesystem::path root(made),file=root/"GameData.bin",backup=root/"GameData.bin.le-backup";
    writeFixture(file,little);
    std::filesystem::create_directory(backup);
    refuses([&]{migrateFile(file,little,big);});assert(readRegular(file)==little);
    std::filesystem::remove(backup);writeFixture(backup,big);
    refuses([&]{migrateFile(file,little,big);});assert(readRegular(file)==little);
    std::filesystem::remove(backup);std::filesystem::create_symlink(file,backup);
    refuses([&]{migrateFile(file,little,big);});assert(readRegular(file)==little);
    std::filesystem::remove(backup);
    migrateFile(file,little,big);assert(readRegular(file)==big && readRegular(backup)==little);
    writeFixture(file,little);migrateFile(file,little,big);assert(readRegular(file)==big && readRegular(backup)==little);
    refuses([&]{migrateFile(file,little,big);});assert(readRegular(backup)==little);
    std::filesystem::remove_all(root);
    std::cout<<"save wire: BE/LE all chunks, unaligned fields, bounds/checksum, backup refusal and resume PASS\n";
}
