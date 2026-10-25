#pragma once
// Trace-on only. Requested QoS is not effective QoS under an override. A CPU
// sample belongs to the calling thread only and may be stale immediately.
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#if defined(__APPLE__)
#include <pthread.h>
#include <pthread/qos.h>
#include <sys/qos.h>
#endif

namespace PetariNative::GpuTrace {
struct HostSample {
    std::uint64_t observer = 0, target = 0, sampledNs = 0;
    int selfQos = -1, requestedQos = -1, relativePriority = 0;
    int policy = -1, priority = -1, cpu = -1;
    int qosError = -1, schedError = -1, cpuError = -1;
};
#if defined(__APPLE__)
inline HostSample sampleHost(pthread_t target = pthread_self()) noexcept {
    HostSample s;
    s.observer = reinterpret_cast<std::uintptr_t>(pthread_self());
    s.target = reinterpret_cast<std::uintptr_t>(target);
    s.selfQos = static_cast<int>(qos_class_self());
    qos_class_t requested = QOS_CLASS_UNSPECIFIED;
    s.qosError = pthread_get_qos_class_np(target, &requested, &s.relativePriority);
    if (!s.qosError) s.requestedQos = static_cast<int>(requested);
    sched_param param{};
    s.schedError = pthread_getschedparam(target, &s.policy, &param);
    if (!s.schedError) s.priority = param.sched_priority;
    // There is no remote pthread CPU-number API. Never copy the waker's CPU
    // into the receiver's sample. The receiver gets a fresh sample at resume.
    if (pthread_equal(target, pthread_self())) {
        std::size_t cpu = 0;
        if (__builtin_available(macOS 11.0, *)) {
            s.cpuError = pthread_cpu_number_np(&cpu);
            if (!s.cpuError) s.cpu = static_cast<int>(cpu);
        }
    }
    return s;
}
#else
inline HostSample sampleHost() noexcept { return {}; }
#endif
struct CpuTopology {
    static constexpr unsigned capacity = 256;
    int ncpu = -1;
    std::array<int, 2> logical{{-1, -1}};
    std::array<std::array<char, 64>, 2> levelName{};
    std::array<char, capacity> cluster{}; // 0 unknown, E/P exact property only
    std::array<bool, capacity> seen{}, conflict{};
    bool registryOpened = false;
    unsigned nodesSeen = 0, logicalIdProperties = 0, clusterTypeProperties = 0;
    void add(unsigned cpu, char type) noexcept {
        if (cpu >= capacity || (type != 'P' && type != 'E')) return;
        if (seen[cpu] && cluster[cpu] != type) conflict[cpu] = true;
        seen[cpu] = true;
        cluster[cpu] = conflict[cpu] ? 0 : type;
    }
};
// Property byte decoding is platform-free and testable without leaking
// CoreFoundation/IOKit (including MacTypes.h ::Rect) into renderer/app TUs.
inline bool decodeTopologyCpu(const void* bytes, std::size_t length, unsigned& cpu) noexcept {
    if (!bytes || length != sizeof(std::uint32_t)) return false;
    std::uint32_t native = 0;
    std::memcpy(&native, bytes, sizeof(native));
    if (native >= CpuTopology::capacity) return false;
    cpu = native; return true;
}
inline char decodeTopologyCluster(const char* bytes, std::size_t length) noexcept {
    if (!bytes || (length != 1 && !(length == 2 && bytes[1] == 0))) return 0;
    return bytes[0] == 'P' || bytes[0] == 'E' ? bytes[0] : 0;
}
CpuTopology readTopology(); // implemented once; platform framework headers stay private
inline void writeTopology(std::FILE* file, const CpuTopology& t) {
    std::fprintf(file, ",\"topology\":{\"provenance\":\"IODeviceTree:/cpus logical-cpu-id+cluster-type; no numbering inference\","
        "\"registry_opened\":%s,\"ncpu\":%d,\"capacity\":256,\"perflevels\":[", t.registryOpened ? "true" : "false", t.ncpu);
    for (unsigned i = 0; i < 2; ++i) std::fprintf(file, "%s{\"level\":%u,\"logicalcpu\":%d,\"name\":\"%s\"}",
        i ? "," : "", i, t.logical[i], t.levelName[i].data());
    std::fprintf(file, "],\"nodes_seen\":%u,\"logical_id_properties\":%u,\"cluster_type_properties\":%u,\"cpus\":[",
        t.nodesSeen, t.logicalIdProperties, t.clusterTypeProperties);
    const unsigned count = t.ncpu > 0 ? (static_cast<unsigned>(t.ncpu) < t.capacity ? t.ncpu : t.capacity) : 0;
    for (unsigned i = 0; i < count; ++i) {
        char name[2] = {t.cluster[i], 0};
        std::fprintf(file, "%s{\"cpu\":%u,\"cluster\":\"%s\",\"conflict\":%s}", i ? "," : "", i,
            name[0] ? name : "unknown", t.conflict[i] ? "true" : "false");
    }
    std::fprintf(file, "]}");
}
}
