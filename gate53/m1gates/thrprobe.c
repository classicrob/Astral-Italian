// Read-only thread priority probe (PORT 2026-10-23). Injected into a PERF COPY only (DYLD_INSERT_LIBRARIES).
// Every 500 ms for THRPROBE_SECONDS (default 10) writes, per thread: name, base/current priority, policy, time-constraint flag,
// QoS tier and cumulative user+system CPU ms, to THRPROBE_OUT. Nothing is changed in the target.
#include <mach/mach.h>
#include <mach/thread_policy.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
static void* run(void* arg) {
  (void)arg;
  pthread_setname_np("thrprobe");
  const char* path = getenv("THRPROBE_OUT"); if (!path) return NULL;
  int seconds = getenv("THRPROBE_SECONDS") ? atoi(getenv("THRPROBE_SECONDS")) : 10;
  FILE* f = fopen(path, "w"); if (!f) return NULL;
  struct timespec t0; clock_gettime(CLOCK_MONOTONIC, &t0);
  for (int i = 0; i < seconds * 2; i++) {
    usleep(500000);
    struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t);
    double s = (t.tv_sec - t0.tv_sec) + (t.tv_nsec - t0.tv_nsec) / 1e9;
    thread_act_array_t th; mach_msg_type_number_t n = 0;
    if (task_threads(mach_task_self(), &th, &n) != KERN_SUCCESS) continue;
    for (mach_msg_type_number_t k = 0; k < n; k++) {
      thread_extended_info_data_t e; mach_msg_type_number_t c = THREAD_EXTENDED_INFO_COUNT;
      if (thread_info(th[k], THREAD_EXTENDED_INFO, (thread_info_t)&e, &c) != KERN_SUCCESS) { mach_port_deallocate(mach_task_self(), th[k]); continue; }
      thread_time_constraint_policy_data_t tc; mach_msg_type_number_t tcc = THREAD_TIME_CONSTRAINT_POLICY_COUNT; boolean_t def = 0;
      int rt = thread_policy_get(th[k], THREAD_TIME_CONSTRAINT_POLICY, (thread_policy_t)&tc, &tcc, &def) == KERN_SUCCESS && !def;
      thread_identifier_info_data_t id; mach_msg_type_number_t idc = THREAD_IDENTIFIER_INFO_COUNT;
      unsigned long long tid = thread_info(th[k], THREAD_IDENTIFIER_INFO, (thread_info_t)&id, &idc) == KERN_SUCCESS ? id.thread_id : 0;
      fprintf(f, "%.1f\t%llu\t%s\tbase=%d\tcur=%d\tmax=%d\tpolicy=%d\trt=%d\tcpu_ms=%llu\tstate=%d\n", s, tid, e.pth_name[0] ? e.pth_name : "-",
              e.pth_priority, e.pth_curpri, e.pth_maxpriority, e.pth_policy, rt,
              (unsigned long long)((e.pth_user_time + e.pth_system_time) / 1000000ULL), e.pth_run_state);
      mach_port_deallocate(mach_task_self(), th[k]);
    }
    vm_deallocate(mach_task_self(), (vm_address_t)th, n * sizeof(*th));
    fflush(f);
  }
  fclose(f); return NULL;
}
__attribute__((constructor)) static void start(void) { pthread_t p; if (getenv("THRPROBE_OUT")) { pthread_create(&p, NULL, run, NULL); pthread_detach(p); } }
