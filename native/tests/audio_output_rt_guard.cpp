// Test-only dyld interposition. Production has none of this code. enter/leave
// and all TLS are prepared before calling the SAME production render symbol.
#include <atomic>
#include <cstdlib>
#include <cstdio>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <AudioToolbox/AudioToolbox.h>
#include <CoreAudio/CoreAudio.h>
namespace {
thread_local bool guarded=false;
std::atomic<unsigned> violations{0};
void mark() { if(guarded) violations.fetch_add(1,std::memory_order_relaxed); }
}
extern "C" void petari_au_guard_enter() { guarded=true; }
extern "C" unsigned petari_au_guard_leave() { guarded=false; return violations.exchange(0); }
#define INTERPOSE(replacement,original) \
    __attribute__((used)) static const struct { const void* to; const void* from; } pair_##original \
    __attribute__((section("__DATA,__interpose"))) = {reinterpret_cast<const void*>(&replacement),reinterpret_cast<const void*>(&original)}
static void* watch_malloc(size_t n) { mark(); return malloc(n); } INTERPOSE(watch_malloc,malloc);
static void* watch_calloc(size_t a,size_t b) { mark(); return calloc(a,b); } INTERPOSE(watch_calloc,calloc);
static void* watch_realloc(void* p,size_t n) { mark(); return realloc(p,n); } INTERPOSE(watch_realloc,realloc);
static void watch_free(void* p) { mark(); free(p); } INTERPOSE(watch_free,free);
static int watch_lock(pthread_mutex_t* p) { mark(); return pthread_mutex_lock(p); } INTERPOSE(watch_lock,pthread_mutex_lock);
static int watch_unlock(pthread_mutex_t* p) { mark(); return pthread_mutex_unlock(p); } INTERPOSE(watch_unlock,pthread_mutex_unlock);
static int watch_signal(pthread_cond_t* p) { mark(); return pthread_cond_signal(p); } INTERPOSE(watch_signal,pthread_cond_signal);
static int watch_broadcast(pthread_cond_t* p) { mark(); return pthread_cond_broadcast(p); } INTERPOSE(watch_broadcast,pthread_cond_broadcast);
static int watch_wait(pthread_cond_t* p,pthread_mutex_t* m) { mark(); return pthread_cond_wait(p,m); } INTERPOSE(watch_wait,pthread_cond_wait);
static int watch_timedwait(pthread_cond_t* p,pthread_mutex_t* m,const timespec* t) { mark(); return pthread_cond_timedwait(p,m,t); } INTERPOSE(watch_timedwait,pthread_cond_timedwait);
static int watch_sem(sem_t* p) { mark(); return sem_wait(p); } INTERPOSE(watch_sem,sem_wait);
static int watch_sleep(const timespec* a,timespec* b) { mark(); return nanosleep(a,b); } INTERPOSE(watch_sleep,nanosleep);
static int watch_usleep(useconds_t t) { mark(); return usleep(t); } INTERPOSE(watch_usleep,usleep);
static int watch_clock(clockid_t c,timespec* t) { mark(); return clock_gettime(c,t); } INTERPOSE(watch_clock,clock_gettime);
static int watch_time(timeval* t,void* z) { mark(); return gettimeofday(t,z); } INTERPOSE(watch_time,gettimeofday);
static kern_return_t watch_base(mach_timebase_info_t t) { mark(); return mach_timebase_info(t); } INTERPOSE(watch_base,mach_timebase_info);
static kern_return_t watch_info(thread_inspect_t t,thread_flavor_t f,thread_info_t o,mach_msg_type_number_t* n) {
    mark(); return thread_info(t,f,o,n);
} INTERPOSE(watch_info,thread_info);
static FILE* watch_open(const char* p,const char* m) { mark(); return fopen(p,m); } INTERPOSE(watch_open,fopen);
static size_t watch_write(const void* p,size_t s,size_t n,FILE* f) { mark(); return fwrite(p,s,n,f); } INTERPOSE(watch_write,fwrite);
static ssize_t watch_fdwrite(int d,const void* p,size_t n) { mark(); return write(d,p,n); } INTERPOSE(watch_fdwrite,write);
static OSStatus watch_unit_get(AudioUnit u,AudioUnitPropertyID p,AudioUnitScope s,AudioUnitElement e,void* d,UInt32* n) {
    mark(); return AudioUnitGetProperty(u,p,s,e,d,n);
} INTERPOSE(watch_unit_get,AudioUnitGetProperty);
static OSStatus watch_unit_set(AudioUnit u,AudioUnitPropertyID p,AudioUnitScope s,AudioUnitElement e,const void* d,UInt32 n) {
    mark(); return AudioUnitSetProperty(u,p,s,e,d,n);
} INTERPOSE(watch_unit_set,AudioUnitSetProperty);
static OSStatus watch_object_get(AudioObjectID o,const AudioObjectPropertyAddress* a,UInt32 q,const void* v,UInt32* n,void* d) {
    mark(); return AudioObjectGetPropertyData(o,a,q,v,n,d);
} INTERPOSE(watch_object_get,AudioObjectGetPropertyData);
static OSStatus watch_convert(AudioConverterRef c,AudioConverterComplexInputDataProc p,void* u,UInt32* n,AudioBufferList* b,AudioStreamPacketDescription* d) {
    mark(); return AudioConverterFillComplexBuffer(c,p,u,n,b,d);
} INTERPOSE(watch_convert,AudioConverterFillComplexBuffer);
