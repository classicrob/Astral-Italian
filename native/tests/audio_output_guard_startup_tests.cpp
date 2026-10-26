// Linking the real guard dylib exercises its interposers throughout dyld /
// libSystem initialization. Darwin 27's xzm malloc calls mach_timebase_info
// there, before TLS exists. Then exercise pass-through before main and verify
// the exact same interposers still detect prohibited calls when armed.
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <pthread.h>
#include <mach/mach_time.h>
extern "C" bool petari_au_guard_is_armed();
extern "C" void petari_au_guard_enter();
extern "C" unsigned petari_au_guard_leave();
namespace {
bool startupPassed=false;
void* volatile escape=nullptr;
__attribute__((constructor)) void beforeMain() {
    assert(!petari_au_guard_is_armed());
    mach_timebase_info_data_t info{};
    assert(mach_timebase_info(&info)==KERN_SUCCESS && info.numer && info.denom);
    escape=std::malloc(23); assert(escape); std::free(escape); escape=nullptr;
    pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
    assert(pthread_mutex_lock(&mutex)==0);
    assert(pthread_mutex_unlock(&mutex)==0);
    assert(pthread_mutex_destroy(&mutex)==0);
    startupPassed=true;
}
}
int main() {
    assert(startupPassed && !petari_au_guard_is_armed());
    petari_au_guard_enter();
    assert(petari_au_guard_leave()==0); // startup calls were passed through
    mach_timebase_info_data_t info{};
    petari_au_guard_enter();
    assert(mach_timebase_info(&info)==KERN_SUCCESS);
    escape=std::malloc(23); assert(escape); std::free(escape); escape=nullptr;
    assert(petari_au_guard_leave()>=3); // clock + allocation + free still caught
    assert(!petari_au_guard_is_armed());
    std::puts("guard dylib process-start / pre-main pass-through / armed detection PASS");
}
