#include <petari/game_wait_history.hpp>
extern "C" int gw_work(int*);
extern "C" __attribute__((noinline)) int gw_probe_plain(int* value){return gw_work(value);}
extern "C" __attribute__((noinline)) int gw_probe_guarded(int* value){
    return PetariNative::GameWaitHistory::invoke(PetariNative::GameWaitHistory::Kind::Token,0,0,[&]{return gw_work(value);});
}
extern "C" __attribute__((noinline)) void gw_probe_metadata(int* value){
    PETARI_GAME_WAIT_EVENT(PetariNative::GameWaitHistory::Kind::Ready,gw_work(value));
}
