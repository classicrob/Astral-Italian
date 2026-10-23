#pragma once
#include <array>
#include <cstdint>
namespace aurora::gx {
// Common selector/routing programs measured in Fluffy's fallback-heavy frames.
// IDs 1..8 are exact; every other program retains the interpreter (ID zero).
struct PetariUberArgProgram {unsigned count=0;std::array<std::array<uint8_t,10>,3> stages{};};
inline constexpr auto PetariUberArgPrograms=[] {
  std::array<PetariUberArgProgram,8> out{};
  // 1b09a7f92bdaed4d: 12922 fallback draws in the measured heavy-frame sample.
  out[0].count=1;
  out[0].stages[0]={4,2,8,15,7,4,1,7,0,0};
  // 70da0b447ef57689: 6558 fallback draws in the measured heavy-frame sample.
  out[1].count=1;
  out[1].stages[0]={2,15,15,15,1,7,7,7,0,0};
  // aaea00809a403c0c: 2675 fallback draws in the measured heavy-frame sample.
  out[2].count=2;
  out[2].stages[0]={4,8,10,2,4,6,5,1,0,0};
  out[2].stages[1]={15,10,1,0,7,6,5,1,0,0};
  // 0cc3f29eff0a2e84: 2304 fallback draws in the measured heavy-frame sample.
  out[3].count=2;
  out[3].stages[0]={14,2,10,15,6,7,5,7,0,0};
  out[3].stages[1]={15,0,8,1,7,5,4,7,0,0};
  // a98049ccb9bef5ce: 1406 fallback draws in the measured heavy-frame sample.
  out[4].count=1;
  out[4].stages[0]={15,12,8,10,7,4,5,7,0,0};
  // 5b5170746821cf9d: 677 fallback draws in the measured heavy-frame sample.
  out[5].count=3;
  out[5].stages[0]={4,8,10,2,4,6,5,1,0,0};
  out[5].stages[1]={15,10,1,0,7,6,5,1,0,0};
  out[5].stages[2]={15,8,10,0,0,7,7,7,0,0};
  // f4791d90f5ca2283: 437 fallback draws in the measured heavy-frame sample.
  out[6].count=3;
  out[6].stages[0]={15,2,10,2,4,2,5,1,0,0};
  out[6].stages[1]={15,10,1,0,7,6,6,7,0,0};
  out[6].stages[2]={4,8,10,0,7,4,6,0,0,0};
  // fcd71189821a2b10: 338 fallback draws in the measured heavy-frame sample.
  out[7].count=1;
  out[7].stages[0]={14,8,10,15,6,7,5,7,0,0};
  return out;
}();
}
