#include <petari/tape_input_observer.hpp>
#include <array>
#include <cstdio>
#include <cstdlib>
using namespace PetariNative::Input;
void require(bool value) { if (!value) std::abort(); }
int main() {
    std::array<unsigned char, 240> data{};
    std::int32_t error = 123, returned = 456;
    int calls = 0, connections = 0, before = 0, after = 0;
    auto original = [&] { ++calls; data.fill(0x5a); error = -2; returned = 3; return 1; };
    auto connection = [&] { ++connections; return std::make_pair(-1, 253u); };
    TapeObserver::install({});
    require(TapeObserver::dispatch(0,120,data.data(),&error,&returned,original,connection)==1);
    require(calls==1 && connections==0 && error==-2 && returned==3);
    for (auto b : data) require(b==0x5a);
    struct Counts { int* before; int* after; } counts{&before,&after};
    TapeObserver::install({
        [](std::int32_t ch,std::uint32_t cap,void* p) {
            require(ch==0 && cap==120); ++*static_cast<Counts*>(p)->before;
        },
        [](std::int32_t,std::uint32_t,std::int32_t n,std::int32_t e,std::int32_t probe,
           std::uint32_t device,const void* buffer,void* p) {
            require(n==3 && e==-2 && probe==-1 && device==253);
            require(static_cast<const unsigned char*>(buffer)[239]==0x5a);
            ++*static_cast<Counts*>(p)->after;
        }, &counts});
    require(TapeObserver::dispatch(0,120,data.data(),&error,&returned,original,connection)==1);
    require(calls==2 && connections==1 && before==1 && after==1);
    // Disabling restores the original path, including unhandled reads and all
    // original side effects; even connection observation must not execute.
    TapeObserver::install({});
    auto unhandled = [&] { ++calls; return 0; };
    require(TapeObserver::dispatch(0,120,data.data(),&error,&returned,unhandled,connection)==0);
    require(calls==3 && connections==1 && before==1 && after==1 && error==-2 && returned==3);
    std::puts("tape observer off path identical; enabled callbacks surround actual output PASS");
}
