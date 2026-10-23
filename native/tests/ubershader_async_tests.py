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
unsigned petariNormalWorkers=4,petariActiveWorkers=4;
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
enum class PetariUberTier { Generic, Fast };
struct PetariUberKeys { gfx::PipelineRef generic, fast; };
uint64_t petari_uber_key(const PipelineConfig& c,const gfx::RenderTargetLayout& l,PetariUberTier tier) {
 return (uint64_t(c.fixed)<<32) | (uint64_t(c.shaderConfig.variant)<<8) | l.key |
     (tier==PetariUberTier::Fast && c.shaderConfig.variant ? uint64_t(1)<<60 : 0);
}
uint64_t petari_uber_key(const PipelineConfig& c,const gfx::RenderTargetLayout& l) { return petari_uber_key(c,l,PetariUberTier::Fast); }
PetariUberKeys petari_uber_keys(const PipelineConfig& c,const gfx::RenderTargetLayout& l) {
 return {petari_uber_key(c,l,PetariUberTier::Generic),petari_uber_key(c,l,PetariUberTier::Fast)};
}
std::mutex uberMutex;
std::map<gfx::PipelineRef,gfx::CompiledPipeline> uberPipelines;
enum class UberPreparation { Queued,Preparing,Ready,Failed };
std::map<gfx::PipelineRef,UberPreparation> uberPreparation;
std::map<gfx::PipelineRef,gfx::PipelineRef> associations;
std::atomic<uint64_t> uberMissing{0},uberPendingDraws{0},uberFailedDraws{0},uberUnknownDraws{0},uberSkippedUnready{0};
thread_local bool petariBuildingUber=false;
thread_local PetariUberTier petariBuildingUberTier=PetariUberTier::Fast;
std::atomic<unsigned> genericBuilds{0},fastBuilds{0};
thread_local double petariUberShaderMs=0;
std::atomic<unsigned> builds{0};
std::atomic<bool> holdBuild=false,releaseBuild=false;
gfx::CompiledPipeline create_pipeline(const PipelineConfig& c,const gfx::RenderTargetLayout&) {
 ++builds;
 if(petariBuildingUberTier==PetariUberTier::Generic)++genericBuilds;else ++fastBuilds;
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
  std::thread first([&]{gx::petari_uber_prepare(c,l,gx::PetariUberTier::Generic);});
  while(!gx::builds) std::this_thread::yield();
  gfx::CompiledPipeline out;
  CHECK(!gx::petari_uber_ready(gx::petari_uber_key(c,l,gx::PetariUberTier::Generic),out));
  std::vector<std::thread> more;
  for(int i=0;i<12;++i) more.emplace_back([&]{gx::petari_uber_prepare(c,l,gx::PetariUberTier::Generic);});
  for(auto& t:more)t.join();
  CHECK(gx::builds==1);
  gx::releaseBuild=true; first.join();
  CHECK(gx::petari_uber_ready(gx::petari_uber_key(c,l,gx::PetariUberTier::Generic),out));
  CHECK(!gx::petariBuildingUber);CHECK(gx::petariBuildingUberTier==gx::PetariUberTier::Fast);
 } else if(test=="tier_claim") {
  gx::holdBuild=true;
  std::thread generic([&]{gx::petari_uber_prepare(c,l,gx::PetariUberTier::Generic);});
  std::thread fast([&]{gx::petari_uber_prepare(c,l,gx::PetariUberTier::Fast);});
  while(gx::builds!=2)std::this_thread::yield();
  std::vector<std::thread> more;
  for(int i=0;i<12;++i)more.emplace_back([&,i]{gx::petari_uber_prepare(c,l,i%2 ? gx::PetariUberTier::Generic : gx::PetariUberTier::Fast);});
  for(auto& t:more)t.join();CHECK(gx::builds==2);
  gx::releaseBuild=true;generic.join();fast.join();
  gfx::CompiledPipeline out;const auto keys=gx::petari_uber_keys(c,l);
  CHECK(gx::petari_uber_ready(keys.generic,out) && gx::petari_uber_ready(keys.fast,out));
  CHECK(gx::genericBuilds==1 && gx::fastBuilds==1);
 } else if(test=="failed") {
  c.shaderConfig.throws=true; gx::petari_uber_prepare(c,l,gx::PetariUberTier::Generic);gx::petari_uber_prepare(c,l,gx::PetariUberTier::Generic);
  gfx::CompiledPipeline out;const auto key=gx::petari_uber_key(c,l,gx::PetariUberTier::Generic);
  CHECK(gx::builds==1); CHECK(!gx::petari_uber_ready(key,out));
  CHECK(gx::uberPreparation.at(key)==gx::UberPreparation::Failed);
  CHECK(!gx::petariBuildingUber);CHECK(gx::petariBuildingUberTier==gx::PetariUberTier::Fast);
 } else if(test=="variant") {
  gfx::petari_uber_queue_variant(c,l);
  gfx::petari_uber_queue_variant(c,l);
  c.shaderConfig.variant=2;gfx::petari_uber_queue_variant(c,l);
  CHECK(gfx::petariUberJobs.size()==2);
  CHECK(gfx::petariUberBoot.scheduled.size()==2);
  CHECK(!take(2) && !take(3)); // pre-title workers remain reserved
  gfx::petariActiveStage="FileSelect";gfx::petari_uber_observe_title(true,false);
  auto one=take(2);auto two=take(3);CHECK(one && two);one();two(); // release borrows all workers' speculative slots
  CHECK(gx::builds==2);CHECK(gfx::petariUberBoot.completed.size()==2);
 } else if(test=="priority") {
  gfx::petariUberTitleReleased=true;
  gfx::petari_uber_queue_variant(c,l,3);
  auto d=c;d.shaderConfig.variant=2;gfx::petari_uber_queue_variant(d,l,3);
  gfx::petari_uber_queue_variant(d,l,1); // promote existing job, don't duplicate
  CHECK(gfx::petariUberJobs.size()==2);
  CHECK(gfx::petariUberJobs.front().key==gx::petari_uber_key(d,l,gx::PetariUberTier::Generic));
  gfx::g_pipelineQueue.push_back({9});gfx::petariPipelineSamples[9].drawBlocking=true;
  CHECK(!take()); // a real draw gets the reserved slot first
  gfx::g_pipelineQueue.clear();auto run=take();CHECK(run);run();
  CHECK(gfx::petariUberBoot.completed.contains(gx::petari_uber_key(d,l,gx::PetariUberTier::Generic)));
 } else if(test=="stage") {
  auto d=c;d.shaderConfig.variant=2;
  gfx::petari_uber_queue_variant(c,l,3);gfx::petari_uber_queue_variant(d,l,3);
  gfx::petari_uber_schedule_stage(d,l,l,"YosshiHomeGalaxy");
  CHECK(gfx::petariUberJobs.size()==3);
  CHECK(gfx::petariUberJobs.front().key==gx::petari_uber_key(d,l,gx::PetariUberTier::Generic));
  CHECK(gfx::petariUberJobs.front().priority==1);
  CHECK(gfx::petariUberBoot.titleKeys.empty()); // active-stage keys are not relabelled title keys
  gfx::petari_uber_schedule_draw(c,l);
  CHECK(gfx::petariUberJobs.front().key==gx::petari_uber_key(d,l,gx::PetariUberTier::Generic)); // active stage stays above other demand
  auto job=take();CHECK(!job); // active-stage work is held until the title release
 } else if(test=="boot") {
  gx::petari_uber_boot();
  CHECK(gfx::petariUberBoot.started);CHECK(gfx::petariUberJobs.size()==1);
  CHECK(gx::builds==0); // boot call never runs enumeration or compilation
  CHECK(!gfx::petariUberTitleReleased);
  gfx::petariActiveStage="Unknown";gfx::petari_uber_observe_title(true,true);
  CHECK(!gfx::petariUberTitleReleased); // earlier menu scenes cannot release workers
  gfx::petariActiveStage="FileSelect";gfx::petari_uber_observe_title(true,true);
  CHECK(gfx::petariUberTitleReleased);
  gfx::petari_uber_async_stop();CHECK(gfx::petariUberJobs.empty());CHECK(gx::builds==0);
  gx::petari_uber_boot();CHECK(gfx::petariUberJobs.size()==1); // generation can restart after shutdown
  CHECK(!gfx::petariUberTitleReleased); // non-front-end eligibility gate resets each boot
 } else if(test=="front_only") {
  auto d=c;d.shaderConfig.variant=2;
  gfx::petari_uber_queue_variant(d,l); // non-front-end generic is held
  gfx::petari_uber_queue_pair(c,l,1,true); // front-end generic + fast
  CHECK(gfx::petariUberJobs.size()==3);
  auto job=take(3);CHECK(job);job();CHECK(gx::genericBuilds==1 && gx::fastBuilds==0); // all workers can prepare front-end Generic
  CHECK(!gfx::petari_uber_worker_ready(0) && !take()); // held jobs cannot keep the CV predicate true
  gfx::petariActiveStage="FileSelect";gfx::petari_uber_observe_title(true,true);
  job=take(3);CHECK(job);job(); // other generic precedes remaining fast
  CHECK(gx::genericBuilds==2 && gx::fastBuilds==0);
  job=take(2);CHECK(job);job();CHECK(gx::fastBuilds==1);
  CHECK(gx::petariBuildingUberTier==gx::PetariUberTier::Fast);
 } else if(test=="tiers") {
  gfx::petariUberTitleReleased=true;
  auto d=c;d.shaderConfig.variant=2;
  gfx::petari_uber_queue_pair(c,l,1,true); // front-end
  gfx::petari_uber_schedule_stage(d,l,l,"YosshiHomeGalaxy");
  CHECK(gfx::petariUberJobs.front().key==gx::petari_uber_keys(d,l).generic);
  auto job=take();CHECK(job);job();
  CHECK(gfx::petariUberJobs.front().key==gx::petari_uber_keys(d,l).fast); // active fast precedes front-end generic
  job=take();CHECK(job);job();CHECK(gx::genericBuilds==1 && gx::fastBuilds==1);
  CHECK(gfx::petariUberJobs.front().key==gx::petari_uber_keys(c,l).generic);
  gfx::petari_uber_schedule_stage(c,l,l,"MarioFaceShipGalaxy"); // old stage jobs lose their priority
  CHECK(gfx::petariUberJobs.front().key==gx::petari_uber_keys(c,l).generic);
 } else if(test=="alias") {
  c.shaderConfig.variant=0;gfx::petari_uber_queue_pair(c,l,1,true);
  CHECK(gfx::petariUberJobs.size()==1 && gfx::petariUberBoot.genericKeys.size()==1 && gfx::petariUberBoot.fastKeys.empty());
 } else if(test=="no_workers") {
  gfx::g_hasPipelineThread=false;gx::petari_uber_boot();
  CHECK(gfx::petariUberBoot.started);CHECK(gfx::petariUberJobs.empty());CHECK(gx::builds==0);
 } else if(test=="flag_off") {
  gx::enabled=false;gx::petari_uber_boot();
  CHECK(!gfx::petariUberBoot.started);CHECK(gfx::petariUberJobs.empty());CHECK(gx::builds==0);
 } else if(test=="wake") {
  gfx::petariUberTitleReleased=true;
  const auto key=gx::petari_uber_key(c,l,gx::PetariUberTier::Generic);gx::associations[99]=key;
  gfx::petari_uber_queue_variant(c,l);
  std::atomic<bool> waiting=false,ready=false;
  std::thread waiter([&]{ std::unique_lock lock{gfx::g_pipelineMutex};waiting=true;
   ready=gfx::g_pipelineReadyCv.wait_for(lock,std::chrono::seconds(2),[&]{return gfx::petari_uber_or_specialized_ready(99);}); });
  while(!waiting)std::this_thread::yield();
  auto run=take();CHECK(run);run();waiter.join();CHECK(ready);
  CHECK(!gfx::g_pipelines.contains(99)); // wake came from the fallback, not the specialised pipeline
 } else if(test=="unready") {
  auto key=gx::petari_uber_key(c,l,gx::PetariUberTier::Generic);gx::petari_uber_scheduled(key);
  gx::petari_uber_not_ready(key,false);gx::petari_uber_not_ready(key,true);
  CHECK(gx::uberMissing==1);CHECK(gx::uberPendingDraws==1);CHECK(gx::uberSkippedUnready==1);
  c.shaderConfig.throws=true;gx::petari_uber_prepare(c,l,gx::PetariUberTier::Generic);gx::petari_uber_not_ready(key,false);
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
            "bool petari_uber_ready(", "void petari_uber_prepare(const PipelineConfig& c,const gfx::RenderTargetLayout& l,PetariUberTier tier)", "void petari_uber_scheduled(",
            "void petari_uber_not_ready(")) + "\n}\nnamespace aurora::gfx {\n" + queue + "\n}\n"
        source += '#include "ubershader_boot.inc"\n' + TESTS
        cpp = Path(cls.tmp.name) / "test.cpp"
        cpp.write_text(source)
        cls.binary = Path(cls.tmp.name) / "test"
        subprocess.run(["clang++", "-std=c++20", "-pthread", "-I", str(GX), str(cpp), "-lsqlite3", "-o", str(cls.binary)], check=True)

    def test_async_semantics(self):
        for test in ("claim", "tier_claim", "failed", "variant", "priority", "stage", "boot", "wake", "unready", "no_workers", "flag_off", "front_only", "tiers", "alias"):
            with self.subTest(test=test):
                completed = subprocess.run([str(self.binary), test], capture_output=True, text=True, timeout=10)
                self.assertEqual(completed.returncode, 0, completed.stdout + completed.stderr)


if __name__ == "__main__":
    unittest.main()
