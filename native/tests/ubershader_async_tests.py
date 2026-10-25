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
static unsigned bootPumps=0;
extern "C" void petari_boot_splash_progress(size_t,size_t) {}
static bool petari_uber_pump_boot_events() { ++bootPumps; return true; }
namespace PetariPipeline {
using Clock=std::chrono::steady_clock;
double milliseconds(Clock::duration d) { return std::chrono::duration<double,std::milli>(d).count(); }
bool fallbackPriority=true;
bool demandFallbackPriority() { return fallbackPriority; }
bool diagnostics() { return false; }
}
namespace aurora {
struct { const char* resourcesPath=""; } g_config;
namespace gx {
struct ShaderConfig { unsigned variant=1; bool covered=true; bool throws=false;
 unsigned numIndStages=0,tevStageCount=1,texgenExtent=1; };
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
bool petariGameplay=false;
std::atomic<bool> g_pipelineThreadEnd=false;
std::condition_variable g_pipelineQueueCv,g_pipelineReadyCv;
enum class ShaderType { GX };
struct Known { ShaderType type=ShaderType::GX; std::variant<gx::PipelineConfig> config; };
std::map<uint64_t,Known> g_knownPipelines;
std::map<PipelineRef,CompiledPipeline> g_pipelines;
struct Pending { PipelineRef hash; };
std::deque<Pending> g_pipelineQueue;
struct Sample { bool drawBlocking=false; ShaderType type=ShaderType::GX; };
std::map<PipelineRef,Sample> petariPipelineSamples;
unsigned petariNormalWorkers=4,petariActiveWorkers=4;
constexpr unsigned kPetariOnDemandWorkers=2;
std::string petariActiveStage;
uint64_t xxh3_hash(const gx::PipelineConfig& c, uint64_t) {
 return (uint64_t(c.fixed)<<32) | (uint64_t(c.shaderConfig.variant)<<16) |
     (c.shaderConfig.tevStageCount<<8) | (c.shaderConfig.numIndStages<<4) | c.shaderConfig.texgenExtent;
}
bool seedFixture=false;
sqlite3* open_pipeline_cache_seed_db(const std::string& path) {
 if(!seedFixture)return nullptr;
 sqlite3* db=nullptr;
 if(sqlite3_open_v2(path.c_str(),&db,SQLITE_OPEN_READONLY,nullptr)!=SQLITE_OK) { if(db)sqlite3_close(db); return nullptr; }
 return db;
}
}
namespace gx {
bool enabled=true;
bool petari_uber_enabled() { return enabled; }
bool petari_uber_all() { return false; }
const char* petari_uber_reason(const ShaderConfig& c) { return c.covered?"covered":"unsupported"; }
bool petari_cached_shader_valid(const ShaderConfig&, uint64_t) { return true; }
enum class PetariUberTier { Generic, Fast };
uint32_t petari_uber_variant(const ShaderConfig& c,PetariUberTier) {
 return c.tevStageCount | (c.texgenExtent<<5) | (c.numIndStages<<9);
}
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
static std::function<void()> take(unsigned worker=1) { // reserved worker 1 prepares before and after the title
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
  CHECK(!take(0)); // worker 0 stays reserved for the draw-blocking specialised compile
  CHECK(!take()); // menus keep specialised draw priority, even with a demanded fallback
  gfx::petariGameplay=true;
  PetariPipeline::fallbackPriority=false;CHECK(!take()); // override off: the real draw gets the slot first
  PetariPipeline::fallbackPriority=true;auto run=take();CHECK(run);run(); // another worker may finish the demanded fallback
  gfx::g_pipelineQueue.clear();
  CHECK(!take(0)); // worker 0 stays available even while the draw queue is empty
  gfx::petariActiveWorkers=1;auto solo=take(0);CHECK(solo);solo(); // single-worker progress
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
 } else if(test=="admission") {
  gfx::petariUberTitleReleased=true;
  gfx::petariActiveStage="A";
  auto expensive=c;expensive.shaderConfig.variant=2;expensive.shaderConfig.numIndStages=1;expensive.shaderConfig.tevStageCount=16;
  std::map<uint64_t,uint32_t> frames{{gfx::xxh3_hash(c,0),88},{gfx::xxh3_hash(expensive,0),88}};
  {
   gfx::PetariUberAdmission outer;
   gfx::petari_uber_admit_stage_frames({c,expensive},l,l,"A",frames);
   CHECK(gfx::petariUberAdmissions==1);
   CHECK(!take()); // no fallback dispatch sees an incomplete outer batch
   CHECK(gfx::petariUberJobs.front().key==gx::petari_uber_key(expensive,l,gx::PetariUberTier::Generic));
  }
  CHECK(gfx::petariUberAdmissions==0);
  gfx::petari_uber_admit_stage_frames({c},l,l,"FileSelect",{},false);
  CHECK(gfx::petariUberBoot.activeStage=="A"); // delayed frontend enumeration cannot steal current-stage priority
  auto run=take();CHECK(run);run();CHECK(gx::genericBuilds==1);
 } else if(test=="frame_alias") {
  gfx::petariActiveStage="A";
  auto alias=c;alias.shaderConfig.tevStageCount=c.shaderConfig.tevStageCount+1; // same fake runtime alias, distinct config
  std::map<uint64_t,uint32_t> frames{{gfx::xxh3_hash(c,0),90},{gfx::xxh3_hash(alias,0),88}};
  gfx::petari_uber_admit_stage_frames({c,alias},l,l,"A",frames);
  auto key=gx::petari_uber_key(c,l,gx::PetariUberTier::Generic);
  CHECK(gfx::petariUberBoot.stageFrames["A"].at(key)==88);
  CHECK(gfx::petariUberJobs.size()==2); // shared Generic/Fast jobs remain deduplicated
  gfx::petari_uber_admit_stage({c,alias},l,l,"A","missing.db");
  CHECK(gfx::petariUberBoot.stageFrames["A"].empty()); // invalid sidecar discards the whole stage frame map
  CHECK(gfx::petariUberJobs.size()==2);
 } else if(test=="boot") {
  gx::petari_uber_boot();
  CHECK(gfx::petariUberBoot.started);CHECK(gfx::petariUberJobs.size()==1);
  CHECK(gx::builds==0); // no seed assets in this fixture: no GPU compile on the main thread
  CHECK(!gfx::petariUberTitleReleased);
  gfx::petariActiveStage="Unknown";gfx::petari_uber_observe_title(true,true);
  CHECK(!gfx::petariUberTitleReleased); // earlier menu scenes cannot release workers
  gfx::petariActiveStage="FileSelect";gfx::petari_uber_observe_title(true,true);
  CHECK(gfx::petariUberTitleReleased);
  gfx::petari_uber_async_stop();CHECK(gfx::petariUberJobs.empty());CHECK(gx::builds==0);
  gx::petari_uber_boot();CHECK(gfx::petariUberJobs.size()==1); // generation can restart after shutdown
  CHECK(!gfx::petariUberTitleReleased && !take(2)); // release gate resets each boot
 } else if(test=="front_only") {
  auto d=c;d.shaderConfig.variant=2;
  gfx::petari_uber_queue_variant(d,l); // non-front-end generic is held
  gfx::petari_uber_queue_pair(c,l,1,true); // front-end generic + fast
  CHECK(gfx::petariUberJobs.size()==3);CHECK(!take(2));
  CHECK(!gfx::petari_uber_worker_ready(0) && !take(0)); // pre-title, worker 0 stays free for the boot screen's draws
  auto job=take();CHECK(job);job();CHECK(gx::genericBuilds==1 && gx::fastBuilds==0);
  CHECK(!gfx::petari_uber_worker_ready(1) && !take()); // held jobs cannot keep the CV predicate true
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
 } else if(test=="stage_order") {
  gfx::petariUberTitleReleased=true;
  auto nonind=c;nonind.fixed=10;nonind.shaderConfig.tevStageCount=16;
  auto ind=c;ind.fixed=11;ind.shaderConfig.numIndStages=1;
  auto tev=ind;tev.fixed=12;tev.shaderConfig.tevStageCount=3;
  auto tex=tev;tex.fixed=13;tex.shaderConfig.texgenExtent=3;
  auto tie=tex;tie.fixed=14;
  for(const auto& cfg:{nonind,ind,tev,tex,tie})gfx::petari_uber_schedule_stage(cfg,l,l,"MokumokuValleyGalaxy");
  std::vector<uint64_t> generic,fast;
  for(const auto& job:gfx::petariUberJobs)(job.tier==gx::PetariUberTier::Generic?generic:fast).push_back(job.key);
  // Active-stage Generic fallbacks keep the stage's first-use order (not a cost proxy).
  CHECK(generic==std::vector<uint64_t>({gx::petari_uber_keys(nonind,l).generic,gx::petari_uber_keys(ind,l).generic,
    gx::petari_uber_keys(tev,l).generic,gx::petari_uber_keys(tex,l).generic,gx::petari_uber_keys(tie,l).generic}));
  CHECK(fast==std::vector<uint64_t>({gx::petari_uber_keys(nonind,l).fast,gx::petari_uber_keys(ind,l).fast,
    gx::petari_uber_keys(tev,l).fast,gx::petari_uber_keys(tex,l).fast,gx::petari_uber_keys(tie,l).fast}));
  // Stage switches demote to the unchanged stable order; activation re-sorts.
  gfx::petari_uber_schedule_stage(c,l,l,"MarioFaceShipGalaxy");
  CHECK(gfx::petariUberJobs.front().key==gx::petari_uber_keys(c,l).generic);
  gfx::petari_uber_schedule_stage(nonind,l,l,"MokumokuValleyGalaxy");
  CHECK(gfx::petariUberJobs.front().key==gx::petari_uber_keys(nonind,l).generic);
  gfx::g_pipelineQueue.push_back({99});gfx::petariPipelineSamples[99].drawBlocking=true;
  CHECK(!take()); // cost sorting cannot preempt a draw-blocking specialised compile
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
 } else if(test=="prepresent_workers") {
  auto other=c;other.fixed=2;
  gfx::petari_uber_queue_pair(c,l,1,true);
  gfx::petari_uber_queue_pair(other,l,3,false);
  gfx::petariUberPrePresent=true;
  for(unsigned i=0;i<4;++i)CHECK(gfx::petari_uber_worker_ready(i));
  auto job=take(0);CHECK(job);job();
  CHECK(gx::genericBuilds==1 && gx::fastBuilds==0);
  for(unsigned i=0;i<4;++i)CHECK(!take(i)); // neither Fast nor unrelated Generic may run
  gfx::petariUberPrePresent=false;
  CHECK(!take(0) && !take(2)); // exact old pre-title reservation resumes
 } else if(test=="prepresent_cap" || test=="prepresent_failed" || test=="prepresent_ready") {
  gfx::petariUberPrePresent=true;gfx::petariUberBoot.frontEndScanned=true;
  if(test=="prepresent_failed")c.shaderConfig.throws=true;
  gfx::petari_uber_queue_variant(c,l,1,true);
  if(test!="prepresent_cap") { auto job=take(0);CHECK(job);job(); }
  gfx::PetariFrontendPump pump;
  auto start=PetariPipeline::Clock::now();
  bool ready=gfx::petari_uber_finish_pre_present(start,start+std::chrono::milliseconds(2),pump);
  CHECK(ready==(test=="prepresent_ready"));
  CHECK(!gfx::petariUberPrePresent && bootPumps>0);
  CHECK(PetariPipeline::milliseconds(PetariPipeline::Clock::now()-start)<100);
  CHECK(!take(0)); // cap/failure/success all restore worker0's normal reservation
 } else if(test=="frontend_data" || test=="frontend_missing") {
  gfx::seedFixture=true;
  auto dir=std::filesystem::path(argv[2])/test;
  std::filesystem::create_directory(dir);
  auto d=c;d.fixed=2;auto unrelated=c;unrelated.fixed=3;
  auto database=[&](const char* name,std::vector<gx::PipelineConfig> configs) {
   sqlite3* db=nullptr;CHECK(sqlite3_open((dir/name).c_str(),&db)==SQLITE_OK);
   CHECK(sqlite3_exec(db,"CREATE TABLE pipeline_cache(type INTEGER,hash INTEGER,config_version INTEGER,config_size INTEGER,config BLOB,first_frame_used INTEGER)",nullptr,nullptr,nullptr)==SQLITE_OK);
   for(auto& config:configs) {
    sqlite3_stmt* stmt=nullptr;CHECK(sqlite3_prepare_v2(db,"INSERT INTO pipeline_cache VALUES(1,?,13,?,?,0)",-1,&stmt,nullptr)==SQLITE_OK);
    sqlite3_bind_int64(stmt,1,static_cast<sqlite3_int64>(gfx::xxh3_hash(config,0)));
    sqlite3_bind_int(stmt,2,sizeof(config));sqlite3_bind_blob(stmt,3,&config,sizeof(config),SQLITE_TRANSIENT);
    CHECK(sqlite3_step(stmt)==SQLITE_DONE);sqlite3_finalize(stmt);
   }
   sqlite3_close(db);return 0;
  };
  CHECK(!database("Unknown.db",{c}));CHECK(!database("FileSelect.db",{d,c}));CHECK(!database("__global__.db",{c,d,unrelated}));
  gfx::petariUberPrePresent=true;gfx::PetariFrontendPump pump;
  const auto start=PetariPipeline::Clock::now(),deadline=start+std::chrono::seconds(1);
  gfx::petari_uber_scan_frontend(dir,l,l,&pump,deadline,{test=="frontend_missing" ? 0ULL : gfx::xxh3_hash(unrelated,0)});
  if(test=="frontend_missing") {
    CHECK(gfx::petariUberBoot.manifestErrors==1);
    while(auto job=take(0))job();
    CHECK(!gfx::petari_uber_finish_pre_present(start,deadline,pump));return 0;
  }
  CHECK(gfx::petariUberBoot.titleKeys.size()==3 && gfx::petariUberBoot.manifestErrors==0);
  CHECK(gfx::petariUberBoot.scheduled.contains(gx::petari_uber_keys(unrelated,l).generic)); // exact observed global-only bootstrap
  while(auto job=take(0))job();
  CHECK(gx::genericBuilds==3 && gx::fastBuilds==0);
  CHECK(gfx::petari_uber_finish_pre_present(start,deadline,pump));
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
        for test in ("claim", "tier_claim", "failed", "variant", "priority", "stage", "admission", "frame_alias", "boot", "wake", "unready", "no_workers", "flag_off", "front_only", "tiers", "stage_order", "alias", "prepresent_workers", "prepresent_cap", "prepresent_failed", "prepresent_ready", "frontend_data", "frontend_missing"):
            with self.subTest(test=test):
                completed = subprocess.run([str(self.binary), test, self.tmp.name], capture_output=True, text=True, timeout=10)
                self.assertEqual(completed.returncode, 0, completed.stdout + completed.stderr)


if __name__ == "__main__":
    unittest.main()
