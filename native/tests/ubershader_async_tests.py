#!/usr/bin/env python3
"""Exercise actual fallback queue/prepare code with a controllable fake GPU.

No renderer window, cache, pinned dependency edits, or Metal compiler required.
The real scheduler include and real claim/publication functions are compiled;
only the GPU, cache records and surrounding Aurora globals are substituted.
"""
from pathlib import Path
import subprocess
import tempfile
import unittest

GX = Path(__file__).resolve().parents[1] / "gx"


def function(text, signature):
    start = text.index(signature)
    first = text.index("{", start)
    depth = 1
    end = first + 1
    while depth:
        depth += (text[end] == "{") - (text[end] == "}")
        end += 1
    return text[start:end]


PREFIX = r'''
#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <deque>
#include <filesystem>
#include <functional>
#include <map>
#include <mutex>
#include <set>
#include <stdexcept>
#include <thread>
#include <variant>
#include <vector>
#include <sqlite3.h>
namespace PetariPipeline {
using Clock=std::chrono::steady_clock;
double milliseconds(Clock::duration d) { return std::chrono::duration<double,std::milli>(d).count(); }
}
namespace aurora {
struct { const char* resourcesPath=""; } g_config;
namespace gx {
struct ShaderConfig { unsigned variant=1; bool covered=true; bool throws=false; };
struct PipelineConfig { ShaderConfig shaderConfig; unsigned fixed=1; };
}
namespace gfx {
using PipelineRef=uint64_t;
struct CompiledPipeline { bool main=false; };
struct RenderTargetLayout { uint64_t key=1; unsigned colorAttachmentCount=2,sampleCount=1; };
namespace detail { void finalize_render_target_layout(RenderTargetLayout& l) { l.key=2; } }
RenderTargetLayout scene_render_target_layout() { return {}; }
std::mutex g_pipelineMutex;
bool g_hasPipelineThread=true;
std::atomic<bool> g_pipelineThreadEnd=false;
std::condition_variable g_pipelineQueueCv,g_pipelineReadyCv;
enum class ShaderType { GX };
struct Known { ShaderType type=ShaderType::GX; std::variant<gx::PipelineConfig> config; };
std::map<uint64_t,Known> g_knownPipelines;
std::map<PipelineRef,CompiledPipeline> g_pipelines;
struct Pending { PipelineRef hash; };
std::deque<Pending> g_pipelineQueue;
struct Sample { bool drawBlocking=false; };
std::map<PipelineRef,Sample> petariPipelineSamples;
unsigned petariNormalWorkers=4;
constexpr unsigned kPetariOnDemandWorkers=2;
std::string petariActiveStage;
sqlite3* open_pipeline_cache_seed_db(const std::string&) { return nullptr; }
}
namespace gx {
bool enabled=true;
bool petari_uber_enabled() { return enabled; }
bool petari_uber_all() { return false; }
const char* petari_uber_reason(const ShaderConfig& c) { return c.covered?"covered":"unsupported"; }
bool petari_cached_shader_valid(const ShaderConfig&, uint64_t) { return true; }
uint64_t petari_uber_key(const PipelineConfig& c,const gfx::RenderTargetLayout& l) {
 return (uint64_t(c.fixed)<<32) | (uint64_t(c.shaderConfig.variant)<<8) | l.key;
}
std::mutex uberMutex;
std::map<gfx::PipelineRef,gfx::CompiledPipeline> uberPipelines;
enum class UberPreparation { Queued,Preparing,Ready,Failed };
std::map<gfx::PipelineRef,UberPreparation> uberPreparation;
std::map<gfx::PipelineRef,gfx::PipelineRef> associations;
std::atomic<uint64_t> uberMissing{0},uberPendingDraws{0},uberFailedDraws{0},uberUnknownDraws{0},uberSkippedUnready{0};
thread_local bool petariBuildingUber=false;
thread_local double petariUberShaderMs=0;
std::atomic<unsigned> builds{0};
std::atomic<bool> holdBuild=false,releaseBuild=false;
gfx::CompiledPipeline create_pipeline(const PipelineConfig& c,const gfx::RenderTargetLayout&) {
 ++builds;
 if(holdBuild) while(!releaseBuild) std::this_thread::yield();
 if(c.shaderConfig.throws) throw std::runtime_error("injected failure");
 return {.main=true};
}
gfx::PipelineRef petari_uber_lookup(gfx::PipelineRef key) { return associations[key]; }
}
}
'''

TESTS = r'''
using namespace aurora;
#define CHECK(x) do { if(!(x)) { std::fprintf(stderr,"failed line %d: %s\n",__LINE__,#x); return 1; } } while(0)
static std::function<void()> take(unsigned worker=0) {
 std::function<void()> job;
 std::lock_guard lock{gfx::g_pipelineMutex}; gfx::petari_take_uber_job(worker,job); return job;
}
int main(int argc,char** argv) {
 const std::string test=argv[1];
 gx::PipelineConfig c; gfx::RenderTargetLayout l;
 if(test=="claim") {
  gx::holdBuild=true;
  std::thread first([&]{gx::petari_uber_prepare(c,l);});
  while(!gx::builds) std::this_thread::yield();
  gfx::CompiledPipeline out;
  CHECK(!gx::petari_uber_ready(gx::petari_uber_key(c,l),out));
  std::vector<std::thread> more;
  for(int i=0;i<12;++i) more.emplace_back([&]{gx::petari_uber_prepare(c,l);});
  for(auto& t:more)t.join();
  CHECK(gx::builds==1);
  gx::releaseBuild=true; first.join();
  CHECK(gx::petari_uber_ready(gx::petari_uber_key(c,l),out));
  CHECK(!gx::petariBuildingUber);
 } else if(test=="failed") {
  c.shaderConfig.throws=true; gx::petari_uber_prepare(c,l);gx::petari_uber_prepare(c,l);
  gfx::CompiledPipeline out;const auto key=gx::petari_uber_key(c,l);
  CHECK(gx::builds==1); CHECK(!gx::petari_uber_ready(key,out));
  CHECK(gx::uberPreparation.at(key)==gx::UberPreparation::Failed);
  CHECK(!gx::petariBuildingUber);
 } else if(test=="variant") {
  gfx::petari_uber_queue_variant(c,l);
  gfx::petari_uber_queue_variant(c,l);
  c.shaderConfig.variant=2;gfx::petari_uber_queue_variant(c,l);
  CHECK(gfx::petariUberJobs.size()==2);
  CHECK(gfx::petariUberBoot.scheduled.size()==2);
  CHECK(!take(2));
  auto one=take();auto two=take(1);CHECK(one && two);one();two();
  CHECK(gx::builds==2);CHECK(gfx::petariUberBoot.completed.size()==2);
 } else if(test=="priority") {
  gfx::petari_uber_queue_variant(c,l,3);
  auto d=c;d.shaderConfig.variant=2;gfx::petari_uber_queue_variant(d,l,3);
  gfx::petari_uber_queue_variant(d,l,1); // promote existing job, don't duplicate
  CHECK(gfx::petariUberJobs.size()==2);
  CHECK(gfx::petariUberJobs.front().key==gx::petari_uber_key(d,l));
  gfx::g_pipelineQueue.push_back({9});gfx::petariPipelineSamples[9].drawBlocking=true;
  CHECK(!take()); // a real draw gets the reserved slot first
  gfx::g_pipelineQueue.clear();auto run=take();CHECK(run);run();
  CHECK(gfx::petariUberBoot.completed.contains(gx::petari_uber_key(d,l)));
 } else if(test=="boot") {
  gx::petari_uber_boot();
  CHECK(gfx::petariUberBoot.started);CHECK(gfx::petariUberJobs.size()==1);
  CHECK(gx::builds==0); // boot call never runs enumeration or compilation
  gfx::petari_uber_async_stop();CHECK(gfx::petariUberJobs.empty());CHECK(gx::builds==0);
 } else if(test=="wake") {
  const auto key=gx::petari_uber_key(c,l);gx::associations[99]=key;
  gfx::petari_uber_queue_variant(c,l);
  std::atomic<bool> waiting=false,ready=false;
  std::thread waiter([&]{ std::unique_lock lock{gfx::g_pipelineMutex};waiting=true;
   ready=gfx::g_pipelineReadyCv.wait_for(lock,std::chrono::seconds(2),[&]{return gfx::petari_uber_or_specialized_ready(99);}); });
  while(!waiting)std::this_thread::yield();
  auto run=take();CHECK(run);run();waiter.join();CHECK(ready);
  CHECK(!gfx::g_pipelines.contains(99)); // wake came from the fallback, not the specialised pipeline
 } else if(test=="unready") {
  auto key=gx::petari_uber_key(c,l);gx::petari_uber_scheduled(key);
  gx::petari_uber_not_ready(key,false);gx::petari_uber_not_ready(key,true);
  CHECK(gx::uberMissing==1);CHECK(gx::uberPendingDraws==1);CHECK(gx::uberSkippedUnready==1);
  c.shaderConfig.throws=true;gx::petari_uber_prepare(c,l);gx::petari_uber_not_ready(key,false);
  CHECK(gx::uberMissing==2);CHECK(gx::uberFailedDraws==1);
 }
 return 0;
}
'''


class Async(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        cls.addClassCleanup(cls.tmp.cleanup)
        runtime = (GX / "ubershader_runtime.inc").read_text()
        queue = (GX / "pipeline_runtime.inc").read_text()
        queue = queue[queue.index("struct PetariUberJob"):queue.index("// Recent compile cost")]
        source = PREFIX + "\nnamespace aurora::gx {\n" + "\n".join(function(runtime, sig) for sig in (
            "bool petari_uber_ready(", "void petari_uber_prepare(", "void petari_uber_scheduled(",
            "void petari_uber_not_ready(")) + "\n}\nnamespace aurora::gfx {\n" + queue + "\n}\n"
        source += '#include "ubershader_boot.inc"\n' + TESTS
        cpp = Path(cls.tmp.name) / "test.cpp"
        cpp.write_text(source)
        cls.binary = Path(cls.tmp.name) / "test"
        subprocess.run(["clang++", "-std=c++20", "-pthread", "-I", str(GX), str(cpp), "-lsqlite3", "-o", str(cls.binary)], check=True)

    def test_async_semantics(self):
        for test in ("claim", "failed", "variant", "priority", "boot", "wake", "unready"):
            with self.subTest(test=test):
                completed = subprocess.run([str(self.binary), test], capture_output=True, text=True, timeout=10)
                self.assertEqual(completed.returncode, 0, completed.stdout + completed.stderr)


if __name__ == "__main__":
    unittest.main()
