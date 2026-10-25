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
#include <sys/sysctl.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
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
#if defined(__APPLE__)
inline bool propertyCpu(CFTypeRef value, unsigned& cpu) {
    std::int64_t number = -1;
    if (value && CFGetTypeID(value) == CFNumberGetTypeID()) {
        if (!CFNumberGetValue(static_cast<CFNumberRef>(value), kCFNumberSInt64Type, &number)) return false;
    } else if (value && CFGetTypeID(value) == CFDataGetTypeID() && CFDataGetLength(static_cast<CFDataRef>(value)) == 4) {
        std::uint32_t native = 0;
        std::memcpy(&native, CFDataGetBytePtr(static_cast<CFDataRef>(value)), sizeof(native));
        number = native; // IODeviceTree cells are native-endian; no reg/name inference
    }
    if (number < 0 || number >= CpuTopology::capacity) return false;
    cpu = static_cast<unsigned>(number); return true;
}
inline char propertyCluster(CFTypeRef value) {
    char text[8]{};
    if (value && CFGetTypeID(value) == CFStringGetTypeID()) {
        if (!CFStringGetCString(static_cast<CFStringRef>(value), text, sizeof(text), kCFStringEncodingUTF8)) return 0;
    } else if (value && CFGetTypeID(value) == CFDataGetTypeID()) {
        const auto length = CFDataGetLength(static_cast<CFDataRef>(value));
        if (length < 1 || length >= static_cast<CFIndex>(sizeof(text))) return 0;
        std::memcpy(text, CFDataGetBytePtr(static_cast<CFDataRef>(value)), static_cast<std::size_t>(length));
    }
    return text[1] == 0 && (text[0] == 'P' || text[0] == 'E') ? text[0] : 0;
}
#endif
inline CpuTopology readTopology() {
    CpuTopology t;
#if defined(__APPLE__)
    std::size_t size = sizeof(t.ncpu);
    if (sysctlbyname("hw.ncpu", &t.ncpu, &size, nullptr, 0)) t.ncpu = -1;
    for (unsigned i = 0; i < 2; ++i) {
        char key[64]; std::snprintf(key, sizeof(key), "hw.perflevel%u.logicalcpu", i);
        size = sizeof(t.logical[i]);
        if (sysctlbyname(key, &t.logical[i], &size, nullptr, 0)) t.logical[i] = -1;
        std::snprintf(key, sizeof(key), "hw.perflevel%u.name", i);
        size = t.levelName[i].size() - 1;
        if (sysctlbyname(key, t.levelName[i].data(), &size, nullptr, 0)) t.levelName[i][0] = 0;
        // Header names are restricted to JSON-safe ASCII; raw strings never go
        // through the realtime ring. Counts do not imply CPU numbering.
        for (auto& c : t.levelName[i]) if (c && !((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') || c == '_' || c == '-')) c = '?';
    }
    const auto root = IORegistryEntryFromPath(kIOMainPortDefault, "IODeviceTree:/cpus");
    if (root) {
        t.registryOpened = true;
        io_iterator_t iterator = IO_OBJECT_NULL;
        if (IORegistryEntryGetChildIterator(root, kIODeviceTreePlane, &iterator) == KERN_SUCCESS) {
            while (auto node = IOIteratorNext(iterator)) {
                ++t.nodesSeen;
                auto id = IORegistryEntryCreateCFProperty(node, CFSTR("logical-cpu-id"), kCFAllocatorDefault, 0);
                auto type = IORegistryEntryCreateCFProperty(node, CFSTR("cluster-type"), kCFAllocatorDefault, 0);
                t.logicalIdProperties += id != nullptr;
                t.clusterTypeProperties += type != nullptr;
                unsigned cpu = 0;
                if (propertyCpu(id, cpu) && (t.ncpu < 0 || cpu < static_cast<unsigned>(t.ncpu))) t.add(cpu, propertyCluster(type));
                if (id) CFRelease(id);
                if (type) CFRelease(type);
                IOObjectRelease(node);
            }
            IOObjectRelease(iterator);
        }
        IOObjectRelease(root);
    }
#endif
    return t;
}
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
