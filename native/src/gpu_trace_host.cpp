#include <petari/gpu_trace_host.hpp>
#if defined(__APPLE__)
#include <sys/sysctl.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#endif

namespace PetariNative::GpuTrace {
namespace {
#if defined(__APPLE__)
bool propertyCpu(CFTypeRef value, unsigned& cpu) {
    std::int64_t number = -1;
    if (value && CFGetTypeID(value) == CFNumberGetTypeID()) {
        if (!CFNumberGetValue(static_cast<CFNumberRef>(value), kCFNumberSInt64Type, &number)) return false;
    } else if (value && CFGetTypeID(value) == CFDataGetTypeID() && CFDataGetLength(static_cast<CFDataRef>(value)) == 4) {
        return decodeTopologyCpu(CFDataGetBytePtr(static_cast<CFDataRef>(value)), 4, cpu);
    }
    if (number < 0 || number >= CpuTopology::capacity) return false;
    cpu = static_cast<unsigned>(number); return true;
}
char propertyCluster(CFTypeRef value) {
    char text[8]{};
    if (value && CFGetTypeID(value) == CFStringGetTypeID()) {
        if (!CFStringGetCString(static_cast<CFStringRef>(value), text, sizeof(text), kCFStringEncodingUTF8)) return 0;
    } else if (value && CFGetTypeID(value) == CFDataGetTypeID()) {
        const auto length = CFDataGetLength(static_cast<CFDataRef>(value));
        if (length < 1 || length >= static_cast<CFIndex>(sizeof(text))) return 0;
        std::memcpy(text, CFDataGetBytePtr(static_cast<CFDataRef>(value)), static_cast<std::size_t>(length));
    }
    return decodeTopologyCluster(text, std::strlen(text));
}
#endif
} // namespace
CpuTopology readTopology() {
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
} // namespace PetariNative::GpuTrace
