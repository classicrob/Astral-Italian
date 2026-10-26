// Real pinned Dawn/Metal content keys and the production artifact SQLite class.
// A generator-output change deliberately keeps the recipe/ABI unchanged.
#include "pipeline_cache_device.hpp"
#include "pipeline_artifact_cache.hpp"
#include <webgpu/webgpu_cpp.h>
#include <filesystem>
#include <mutex>
#include <set>
#include <map>
#include <stdexcept>
#include <string>
#include <unistd.h>
namespace C=PetariPipelineCache;
static void need(bool b,const char* why){if(!b)throw std::runtime_error(why);}
struct Result {std::set<std::string> requested;std::map<std::string,bool> firstProbeHit;uint64_t hits=0,misses=0,stores=0;};
static Result compile(const std::filesystem::path& root,const std::string& wgsl) {
    const wgpu::InstanceFeatureName timed=wgpu::InstanceFeatureName::TimedWaitAny;
    wgpu::InstanceDescriptor id{.requiredFeatureCount=1,.requiredFeatures=&timed};
    auto instance=wgpu::CreateInstance(&id);need(bool(instance),"Dawn instance");
    wgpu::Adapter adapter;wgpu::RequestAdapterOptions opts{.backendType=wgpu::BackendType::Metal};
    const auto af=instance.RequestAdapter(&opts,wgpu::CallbackMode::WaitAnyOnly,[&](wgpu::RequestAdapterStatus s,wgpu::Adapter a,wgpu::StringView message){
        if(s==wgpu::RequestAdapterStatus::Success)adapter=std::move(a);
        else std::fprintf(stderr,"adapter: %.*s\n",int(message.length),message.data);
    });
    need(instance.WaitAny(af,5000000000)==wgpu::WaitStatus::Success && adapter,"Metal adapter");
    wgpu::AdapterInfo info;adapter.GetInfo(&info);C::configureAdapter(info);const auto context=C::artifactContext();need(!context.empty(),"real adapter identity");
    C::ArtifactCache cache;need(cache.open((root/(context+".db")).string(),context),"private production cache");
    Result result;std::mutex mutex;
    struct CallbackState { C::ArtifactCache* cache; Result* result; std::mutex* mutex; } state{&cache,&result,&mutex};
    wgpu::DawnCacheDeviceDescriptor caching({.nextInChain=nullptr});
    caching.SetDawnLoadCacheDataCallback([](std::span<const std::byte> key,std::span<std::byte> value,CallbackState* state) noexcept -> size_t {
        std::lock_guard lock(*state->mutex);state->result->requested.emplace(reinterpret_cast<const char*>(key.data()),key.size());
        const auto n=state->cache->load(key.data(),key.size(),value.data(),value.size());
        const std::string k(reinterpret_cast<const char*>(key.data()),key.size());
        state->result->firstProbeHit.try_emplace(k,n>0);
        return n;
    },&state);
    caching.SetDawnStoreCacheDataCallback([](std::span<const std::byte> key,std::span<const std::byte> value,CallbackState* state) noexcept {
        std::lock_guard lock(*state->mutex);state->cache->store(key.data(),key.size(),value.data(),value.size());
    },&state);
    wgpu::DeviceDescriptor dd({.nextInChain=&caching});unsigned errors=0;
    dd.SetUncapturedErrorCallback([](const wgpu::Device&,wgpu::ErrorType,wgpu::StringView message,unsigned* n){++*n;std::fprintf(stderr,"Dawn: %.*s\n",int(message.length),message.data);},&errors);
    wgpu::Device device;const auto df=adapter.RequestDevice(&dd,wgpu::CallbackMode::WaitAnyOnly,[&](wgpu::RequestDeviceStatus s,wgpu::Device d,wgpu::StringView message){
        if(s==wgpu::RequestDeviceStatus::Success)device=std::move(d);else std::fprintf(stderr,"device: %.*s\n",int(message.length),message.data);
    });
    need(instance.WaitAny(df,5000000000)==wgpu::WaitStatus::Success && device,"Dawn device");
    wgpu::ShaderSourceWGSL source;source.code=wgsl.c_str();wgpu::ShaderModuleDescriptor sd{.nextInChain=&source};auto shader=device.CreateShaderModule(&sd);
    wgpu::ComputePipelineDescriptor pd;pd.compute.module=shader;pd.compute.entryPoint="main";
    auto pipeline=device.CreateComputePipeline(&pd);need(bool(pipeline) && errors==0,"compute compilation");
    pipeline=nullptr;shader=nullptr;device.Destroy();device=nullptr;instance.ProcessEvents();adapter=nullptr;instance=nullptr;
    result.hits=cache.hits;result.misses=cache.misses;result.stores=cache.stores;
    need(!result.requested.empty(),"Dawn must exercise actual disk callbacks");return result;
}
int main() try {
    char pattern[]="/tmp/petari-dawn-keys-XXXXXX";const auto* p=mkdtemp(pattern);need(p,"private root");std::filesystem::path root(p);
    std::filesystem::create_directory(root/"home");setenv("HOME",(root/"home").c_str(),1);
    const auto generate=[](bool changed) {return std::string("@group(0) @binding(0) var<storage,read_write> output:array<u32>; @compute @workgroup_size(1) fn main(){ output[0]=")+(changed?"2u":"1u")+"; }";};
    const auto abi=std::string(PETARI_PIPELINE_RECIPE_KEY);
    const auto a=compile(root,generate(false)),b=compile(root,generate(false)),c=compile(root,generate(true));
    need(a.misses>0 && a.stores>0,"cold request misses and stores");
    need(b.hits>0 && a.requested==b.requested,"compatible build/source/CMake reuse actual full Dawn keys");
    size_t changedMisses=0;
    for(const auto& key:c.requested) if(!a.requested.contains(key)) {++changedMisses;need(!c.firstProbeHit.at(key),"changed emitted WGSL key must miss at Dawn");}
    need(changedMisses>0 && c.stores>0,"changed emitted WGSL produces new Dawn requests/stores without ABI bump");
    need(abi==PETARI_PIPELINE_RECIPE_KEY,"test did not bump recipe/generator ABI");
    std::fprintf(stdout,"Actual Dawn keys: cold stored=%llu warm hits=%llu changed-WGSL new-miss-keys=%llu; unchanged ABI=%s PASS\n",(unsigned long long)a.stores,(unsigned long long)b.hits,(unsigned long long)changedMisses,abi.c_str());
    std::filesystem::remove_all(root);return 0;
} catch(const std::exception& e){std::fprintf(stderr,"FAIL: %s\n",e.what());return 1;}
