// Real Darwin backend contract, no latency/energy success assertion. The original
// native_platform_os <=1.5ms fixture remains authoritative for timing admission.
#include "monitor_wait.hpp"
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <thread>
#include <csignal>
#include <sys/wait.h>
#include <unistd.h>
using namespace PetariNative::Platform::MonitorWait;
int main() {
    setenv("PETARI_BATON_WAIT_MODE","critical",1);
    configure();assert(mode()==Mode::Critical && notificationAvailable());
    notify(); // Before wait: registration must not erase an already queued edge.
    auto result=waitCritical(1000000000,false);
    assert(!result.error && result.notification && result.generation==1 && !result.stale);
    // First timer was cancelled. Its old generation must not reach this wait.
    result=waitCritical(0,true);
    assert(!result.error && result.timer && result.generation==2 && !result.stale);
    std::thread notifier([] {std::this_thread::sleep_for(std::chrono::milliseconds(10));notify();});
    result=waitCritical(500000000,true);notifier.join();
    assert(!result.error && result.notification && !result.stale);
    const pid_t child=fork();assert(child>=0);
    if(!child){afterForkChild();_exit(notificationAvailable()?1:0);}
    int status=0;assert(waitpid(child,&status,0)==child && WIFEXITED(status) && WEXITSTATUS(status)==0);
    assert(notificationAvailable()); // Closing child's descriptor cannot affect parent.
    result=waitCritical(0,true);assert(result.timer && !result.stale && !result.error);
    // A backend error must terminate the experimental arm, not claim fallback.
    const pid_t bad=fork();assert(bad>=0);
    if(!bad){afterForkChild();waitCritical(0,true);_exit(9);}
    assert(waitpid(bad,&status,0)==bad && WIFSIGNALED(status) && WTERMSIG(status)==SIGABRT);
    const pid_t invalid=fork();assert(invalid>=0);
    if(!invalid){setenv("PETARI_BATON_WAIT_MODE","critical-realtime",1);configure();_exit(9);}
    assert(waitpid(invalid,&status,0)==invalid && WIFSIGNALED(status) && WTERMSIG(status)==SIGABRT);
}
