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
queue = boot[boot.index('static unsigned petari_uber_job_priority('):boot.index('static void petari_uber_queue_variant(')]
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
  std::puts("Fallback ordering: 120 mixed permutations, strict weak ordering, insertion and stage switches pass");
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
