// CPU-only SDL stream operations. Never opens an audio device or starts playback.
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_petari_audio_diag.h>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <vector>
struct Boundary { unsigned kind; const void* object; Sint64 value; };
static std::array<Boundary,256> seen;
static unsigned used=0;
static void SDLCALL observe(unsigned kind,const void* object,Sint64 value) {
    assert(used<seen.size()); seen[used++]={kind,object,value};
}
static void SDLCALL fill(void*,SDL_AudioStream* stream,int additional,int) {
    assert(additional>=0 && additional<=16384);
    std::array<std::int16_t,8192> data;
    for(unsigned n=0;n<data.size();++n) data[n]=static_cast<std::int16_t>(n*31);
    assert(SDL_PutAudioStreamData(stream,data.data(),additional));
}
static std::vector<unsigned char> converted(bool on) {
    assert(SDL_PetariConfigureAudioDiagnostic(on ? observe : nullptr,0));
    SDL_AudioSpec from{SDL_AUDIO_S16,2,32000}, to{SDL_AUDIO_F32,2,44100};
    auto* stream=SDL_CreateAudioStream(&from,&to); assert(stream);
    assert(SDL_SetAudioStreamGetCallback(stream,fill,nullptr));
    std::vector<unsigned char> result(2048);
    assert(SDL_GetAudioStreamData(stream,result.data(),result.size())==int(result.size()));
    SDL_DestroyAudioStream(stream);
    return result;
}
int main() {
    const auto off=converted(false); assert(used==0);
    const auto on=converted(true); assert(on==off);
    const unsigned expected[]={SDL_PETARI_GET_BEGIN,SDL_PETARI_GET_LOCK_REQUEST,SDL_PETARI_GET_LOCK_ACQUIRED,
        SDL_PETARI_PUT_BEGIN,SDL_PETARI_PUT_LOCK_REQUEST,SDL_PETARI_PUT_LOCK_ACQUIRED,
        SDL_PETARI_PUT_LOCK_RELEASED,SDL_PETARI_PUT_END,SDL_PETARI_CONVERT_BEGIN,
        SDL_PETARI_CONVERT_END,SDL_PETARI_GET_END};
    assert(used==sizeof(expected)/sizeof(*expected));
    for(unsigned n=0;n<used;++n) { assert(seen[n].kind==expected[n]); assert(seen[n].object==seen[0].object); }
    // Invalid stream setup still balances the acquired/released put lock and
    // supplies get/put error outcomes, without conversion or a fill callback.
    used=0;
    auto* empty=SDL_CreateAudioStream(nullptr,nullptr); assert(empty);
    std::int16_t pcm[8]{};
    assert(!SDL_PutAudioStreamData(empty,pcm,sizeof(pcm)));
    assert(used==5 && seen[0].kind==SDL_PETARI_PUT_BEGIN && seen[3].kind==SDL_PETARI_PUT_LOCK_RELEASED &&
        seen[4].kind==SDL_PETARI_PUT_END && seen[4].value==0);
    used=0;
    assert(SDL_GetAudioStreamData(empty,pcm,sizeof(pcm))==-1);
    assert(used==4 && seen[2].kind==SDL_PETARI_GET_LOCK_ACQUIRED && seen[3].kind==SDL_PETARI_GET_END && seen[3].value==-1);
    SDL_DestroyAudioStream(empty);
    assert(!SDL_PetariConfigureAudioDiagnostic(nullptr,18));
    assert(!SDL_PetariConfigureAudioDiagnostic(observe,3));
    assert(SDL_PetariConfigureAudioDiagnostic(observe,6));
    assert(SDL_PetariConfigureAudioDiagnostic(observe,18));
    assert(SDL_PetariConfigureAudioDiagnostic(nullptr,0));
    used=0; assert(converted(false)==off && used==0);
    std::puts("SDL transport observer: disabled bytes identical / recursive get-put order / error locks / explicit count PASS");
}
