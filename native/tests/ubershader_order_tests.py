#!/usr/bin/env python3
"""Compile the production fallback queue comparator; no GPU or game assets."""
import subprocess
import sys
import tempfile
from pathlib import Path

root = Path(__file__).resolve().parents[2]
boot = (root / 'native/gx/ubershader_boot.inc').read_text()
runtime = (root / 'native/gx/pipeline_runtime.inc').read_text()
job = runtime[runtime.index('struct PetariUberJob {'):runtime.index('static std::deque<PetariUberJob>')]
queue = boot[boot.index('static unsigned petari_uber_job_priority('):boot.index('static void petari_uber_reorder_jobs(')]
source = r'''
#include <algorithm>
#include <cstdint>
#include <deque>
#include <functional>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <cstdio>
namespace gx { enum class PetariUberTier { Generic, Fast }; }
using PipelineRef = uint64_t;
struct BootState {
  std::string activeStage;
  std::map<std::string, std::set<PipelineRef>> stageKeys;
  std::map<std::string, std::map<PipelineRef, size_t>> stageOrder;
  std::map<std::string, std::map<PipelineRef, uint32_t>> stageFrames;
} petariUberBoot;
''' + job + '\nstatic std::deque<PetariUberJob> petariUberJobs;\n' + queue + r'''
static void check(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
static PetariUberJob item(PipelineRef key, gx::PetariUberTier tier = gx::PetariUberTier::Generic) {
  PetariUberJob job{}; job.key = key; job.tier = tier; return job;
}
int main() {
  petariUberBoot.activeStage = "A";
  // Ranked cheap/expensive keys, unranked keys, and another tier share priority 1.
  petariUberBoot.stageKeys["A"] = {11, 12, 101, 102, 19};
  petariUberBoot.stageOrder["A"] = {{12, 0}, {11, 1}};
  std::vector<PetariUberJob> mixed{item(101), item(11), item(19, gx::PetariUberTier::Fast), item(12), item(102)};
  mixed[1].hasIndirect = true; mixed[1].tevStages = 16; // cost must not override rank
  for (auto& job : mixed) job.priority = 1;
  std::vector<unsigned> permutation{0, 1, 2, 3, 4};
  do {
    std::vector<PetariUberJob> jobs;
    std::vector<PipelineRef> expected{12, 11};
    for (auto index : permutation) {
      jobs.push_back(mixed[index]);
      if (mixed[index].key == 101 || mixed[index].key == 102) expected.push_back(mixed[index].key);
    }
    expected.push_back(19);
    for (unsigned repeat = 0; repeat < 3; ++repeat) {
      std::stable_sort(jobs.begin(), jobs.end(), petari_uber_job_before);
      for (size_t n = 0; n < jobs.size(); ++n)
        check(jobs[n].key == expected[n], "mixed ranked/missing/tier order is unstable or incorrect");
    }
  } while (std::next_permutation(permutation.begin(), permutation.end()));
  // Exhaustively check strict weak ordering, including incomparable missing ranks.
  auto domain = mixed;
  for (auto job : mixed) { job.priority = 3; domain.push_back(job); }
  const auto less = petari_uber_job_before;
  const auto equivalent = [&](const auto& a, const auto& b) { return !less(a,b) && !less(b,a); };
  for (const auto& a : domain) for (const auto& b : domain) for (const auto& c : domain) {
    check(!less(a,a), "comparator is not irreflexive");
    check(!(less(a,b) && less(b,a)), "comparator is not asymmetric");
    check(!(less(a,b) && less(b,c)) || less(a,c), "ordering is not transitive");
    check(!(equivalent(a,b) && equivalent(b,c)) || equivalent(a,c), "incomparability is not transitive");
  }
  // Exercise the actual insertion path as well as stable_sort's mixed domain.
  for (auto key : {101u, 11u, 12u, 102u}) petari_uber_push_job(item(key));
  check(petariUberJobs[0].key == 12 && petariUberJobs[1].key == 11 &&
        petariUberJobs[2].key == 101 && petariUberJobs[3].key == 102, "insertion and sorting disagree");
  // Switching stages must use that stage's ranks; missing stage metadata ties all Generic keys.
  petariUberBoot.activeStage = "B";
  petariUberBoot.stageOrder["B"] = {{11, 0}, {12, 1}};
  std::stable_sort(mixed.begin(), mixed.end(), less);
  check(mixed[0].key == 11 && mixed[1].key == 12, "stage switch retained old ranks");
  petariUberBoot.activeStage = "missing";
  for (const auto& a : domain) for (const auto& b : domain) for (const auto& c : domain)
    check(!(equivalent(a,b) && equivalent(b,c)) || equivalent(a,c), "missing stage breaks strict weak ordering");
  petariUberBoot.activeStage = "A";
  petariUberBoot.stageFrames["A"] = {{201,7},{202,8},{203,8},{204,8},{206,7},{207,7}};
  petariUberBoot.stageOrder["A"] = {{201,90},{202,1},{203,0},{204,99},{205,0}};
  std::vector<PetariUberJob> framed{item(201),item(202),item(203),item(204),item(205),item(206,gx::PetariUberTier::Fast)};
  for (auto& job : framed) job.priority = 1;
  framed[0].tevStages=1;
  framed[1].tevStages=16; // more TEV cannot outrank an indirect config in the SAME frame
  framed[2].hasIndirect=true; framed[2].tevStages=1; framed[2].texgenExtent=9;
  framed[3].hasIndirect=true; framed[3].tevStages=2; framed[3].texgenExtent=2;
  framed[4].hasIndirect=true; framed[4].tevStages=99; // unknown frame: no guessed group or cost override
  std::vector<unsigned> framePermutation{0,1,2,3,4,5};
  const std::vector<PipelineRef> frameExpected{201,204,203,202,205,206};
  do {
    std::vector<PetariUberJob> jobs;
    for (auto index : framePermutation) jobs.push_back(framed[index]);
    std::stable_sort(jobs.begin(),jobs.end(),less);
    for (size_t n=0;n<jobs.size();++n) check(jobs[n].key==frameExpected[n],"frame/cost/missing/tier order differs");
  } while (std::next_permutation(framePermutation.begin(),framePermutation.end()));
  auto texgenHigh=framed[3]; texgenHigh.key=209; texgenHigh.texgenExtent=5;
  petariUberBoot.stageFrames["A"][209]=8;
  check(less(texgenHigh,framed[3]),"texgen extent is not the final cost component");
  auto missingRank=item(207); missingRank.priority=1;
  auto missingBoth=item(208); missingBoth.priority=1;
  domain.insert(domain.end(),framed.begin(),framed.end());
  domain.push_back(texgenHigh); domain.push_back(missingRank); domain.push_back(missingBoth);
  // Frontend scene ordinal is independent of missing metadata and shared aliases.
  petariUberBoot.stageKeys["Unknown"]={301,302,303};
  petariUberBoot.stageKeys["FileSelect"]={303,304,305};
  petariUberBoot.stageFrames["Unknown"]={{301,20},{302,10},{303,30}};
  petariUberBoot.stageFrames["FileSelect"]={{303,0},{304,0},{305,99}};
  petariUberBoot.stageOrder["Unknown"]={{301,0},{302,1},{303,2}};
  petariUberBoot.stageOrder["FileSelect"]={{303,0},{304,1},{305,2}};
  std::vector<PetariUberJob> front;
  for (auto key : {305u,303u,999u,301u,304u,302u}) { auto job=item(key); job.priority=3; front.push_back(job); }
  std::stable_sort(front.begin(),front.end(),less);
  const std::vector<PipelineRef> frontExpected{302,301,303,304,305,999};
  for(size_t n=0;n<front.size();++n) check(front[n].key==frontExpected[n],"frontend/shared alias order differs");
  domain.insert(domain.end(),front.begin(),front.end());
  const auto validate = [&] {
    for (const auto& a : domain) for (const auto& b : domain) for (const auto& c : domain) {
      check(!less(a,a),"frame comparator is not irreflexive");
      check(!(less(a,b)&&less(b,a)),"frame comparator is not asymmetric");
      check(!(less(a,b)&&less(b,c))||less(a,c),"frame comparator is not transitive");
      check(!(equivalent(a,b)&&equivalent(b,c))||equivalent(a,c),"frame incomparability is not transitive");
    }
  };
  validate();
  petariUberBoot.stageFrames["Unknown"].clear(); // whole frontend scene falls back to its existing ranks
  std::stable_sort(front.begin(),front.end(),less);
  check(front[0].key==301&&front[1].key==302&&front[2].key==303&&front[3].key==304,"missing frontend changed scene/rank order");
  validate();
  std::puts("Fallback ordering:120 legacy+720 frame permutations, cost components, frontend fallback, aliases and strict weak order PASS");
}
'''
with tempfile.TemporaryDirectory() as directory:
    path = Path(directory)
    cpp, binary = path / 'order.cpp', path / 'order'
    cpp.write_text(source)
    command = [sys.argv[1], '-std=c++20', '-O2']
    if sys.platform == 'darwin':
        sdk = subprocess.check_output(['xcrun', '--show-sdk-path'], text=True).strip()
        command += ['-isysroot', sdk]
    subprocess.run(command + [str(cpp), '-o', str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
