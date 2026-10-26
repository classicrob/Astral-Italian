#pragma once
#include "pipeline_cache_identity.hpp"
#include <webgpu/webgpu_cpp.h>
#if defined(__APPLE__)
#include <sys/sysctl.h>
#else
#include <sys/utsname.h>
#endif
namespace PetariPipelineCache {
inline std::string cacheString(wgpu::StringView view) {
    if(view.IsUndefined() || !view.data) return {};
    if(view.length==WGPU_STRLEN) return std::string(view.data);
    return std::string(view.data,view.length);
}
inline std::string osContext() {
#if defined(__APPLE__)
    size_t size=0;
    if(sysctlbyname("kern.osversion",nullptr,&size,nullptr,0)!=0 || size<2 || size>1024) return {};
    std::string out(size,'\0');
    if(sysctlbyname("kern.osversion",out.data(),&size,nullptr,0)!=0) return {};
    if(!out.empty() && out.back()=='\0') out.pop_back();
    return "macOS-kernel-build:"+out;
#else
    struct utsname value{}; if(uname(&value)) return {};
    return std::string(value.sysname)+":"+value.release+":"+value.version;
#endif
}
inline void configureAdapter(const wgpu::AdapterInfo& info) {
    DeviceContext context;
    if(info.backendType==wgpu::BackendType::Undefined || info.backendType==wgpu::BackendType::Null) {setDeviceContext(context);return;}
    context.backend=std::to_string(static_cast<uint32_t>(info.backendType));
    context.vendor=std::to_string(info.vendorID); context.device=std::to_string(info.deviceID);
    context.architecture=cacheString(info.architecture); context.model=cacheString(info.device);
    context.driver=cacheString(info.description);context.os=osContext();
#if defined(__APPLE__)
    // Metal exposes no separate driver version/architecture string on some
    // adapters. The selected model + actual OS build identify Apple's driver.
    if(info.backendType==wgpu::BackendType::Metal && !context.os.empty() && DeviceContext::known(context.model)) {
        if(!DeviceContext::known(context.driver)) context.driver="AppleMetal:"+context.os;
        if(!DeviceContext::known(context.architecture)) context.architecture="Metal:"+context.model;
    }
#endif
    std::fprintf(stderr,"[pipeline compatibility device] backend=%s vendor=%s device=%s architecture=%s model=%s driver=%s os=%s\n",
        context.backend.c_str(),context.vendor.c_str(),context.device.c_str(),context.architecture.c_str(),context.model.c_str(),context.driver.c_str(),context.os.c_str());
    setDeviceContext(context);
}
} // namespace PetariPipelineCache
