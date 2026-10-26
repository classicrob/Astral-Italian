// Production Dawn callbacks. No index-only key, unchecked size probe or legacy
// blob relabelling. Tests invoke this same ArtifactCache implementation.
#include "pipeline_artifact_cache.hpp"
#include "../internal.hpp"
#include "../io.hpp"
#include <filesystem>
#include <mutex>

namespace aurora::webgpu {
static std::mutex cache_mutex;
static PetariPipelineCache::ArtifactCache artifactCache;
static bool cache_init() {
    const auto context=PetariPipelineCache::artifactContext();
    if(context.empty()) return false;
    const auto path=io::fs_path_from_string(g_config.cachePath)/("dawn_cache.v2."+context+".db");
    return artifactCache.open(io::fs_path_to_string(path),context);
}
size_t load_from_cache(const void* key,size_t keySize,void* value,size_t valueSize,void*) {
    std::lock_guard lock(cache_mutex);
    return cache_init()?artifactCache.load(key,keySize,value,valueSize):0;
}
void store_to_cache(const void* key,size_t keySize,const void* value,size_t valueSize,void*) {
    std::lock_guard lock(cache_mutex);
    if(cache_init()) artifactCache.store(key,keySize,value,valueSize);
}
void cache_prune() {
    // Not observing a key in one play session does not make it incompatible.
    // Retain validated learned blobs across scenes and compatible builds.
}
void cache_shutdown() {
    std::lock_guard lock(cache_mutex);
    std::fprintf(stderr,"[pipeline artifacts] hits=%llu misses=%llu rejected=%llu stored=%llu\n",
        (unsigned long long)artifactCache.hits,(unsigned long long)artifactCache.misses,
        (unsigned long long)artifactCache.rejected,(unsigned long long)artifactCache.stores);
    artifactCache.close();
}
} // namespace aurora::webgpu
