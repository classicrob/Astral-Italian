#!/usr/bin/env python3
"""Enable optional timestamp CSV using Aurora's existing readback ring, without Tracy."""
from pathlib import Path
import sys
source,output=map(Path,sys.argv[1:]);text=source.read_text()
def replace(a,b):
 global text
 if text.count(a)!=1:raise ValueError(f'GPU timing anchor {text.count(a)}: {a}')
 text=text.replace(a,b)
if source.name=='gpu.cpp':
 replace('''#ifdef TRACY_ENABLE
      if (feature == wgpu::FeatureName::TimestampQuery) {
        requiredFeatures.push_back(feature);
      }
#endif''','''      if (feature == wgpu::FeatureName::TimestampQuery) {
#ifdef TRACY_ENABLE
        requiredFeatures.push_back(feature);
#else
        const auto* path = std::getenv("PETARI_GPU_TIMING_CSV");
        if (path && *path) requiredFeatures.push_back(feature);
#endif
      }''')
else:
 replace('#ifdef TRACY_ENABLE\n\n#include <tracy/TracyC.h>','#include <cstdio>\n#include <cstdlib>\n#ifdef TRACY_ENABLE\n#include <tracy/TracyC.h>\n#endif')
 # Remove the entire old non-Tracy no-op implementation.
 text=text[:text.index('\n#else\n\nnamespace aurora::webgpu::gpu_prof {')]
 replace('  int64_t submitNs = 0;','  int64_t submitNs = 0;\n  uint64_t frame = 0;')
 replace('bool g_enabled = false;','''FILE* g_csv = nullptr;
uint64_t g_csvFrame = 0;
bool g_enabled = false;''')
 replace('void emit_context(const Slot& slot, uint64_t frameBegin) {','#ifdef TRACY_ENABLE\nvoid emit_context(const Slot& slot, uint64_t frameBegin) {')
 replace('TimestampBounds event_bounds(const Slot& slot, const uint64_t* ts) {','#endif\nTimestampBounds event_bounds(const Slot& slot, const uint64_t* ts) {')
 replace('  const uint64_t lastFrameEnd = std::exchange(g_lastFrameEnd, frameEnd);','''  if (g_csv) {
    const auto begin = ts[FrameBeginQuery];
    const auto end = ts[FrameEndQuery];
    // Never synthesize elapsed GPU time from consecutive frame ends. In particular,
    // Dawn/Metal can expose stale begin timestamps; leave invalid rows empty.
    const bool valid = begin && end > begin && (!g_lastFrameEnd || begin >= g_lastFrameEnd) && end-begin < SaneFrameGapNs;
    if (valid) std::fprintf(g_csv, "%llu,%.6f,,valid\\n", (unsigned long long)slot.frame, double(end-begin)*1e-6);
    else std::fprintf(g_csv, "%llu,,,invalid_timestamp\\n", (unsigned long long)slot.frame);
    std::fflush(g_csv);
  }
#ifndef TRACY_ENABLE
  g_lastFrameEnd = ts[FrameEndQuery];
  return;
#else
  const uint64_t lastFrameEnd = std::exchange(g_lastFrameEnd, frameEnd);''')
 replace('  TracyPlot("aurora: gpuPasses", int64_t(slot.passCount));','  TracyPlot("aurora: gpuPasses", int64_t(slot.passCount));\n#endif')
 replace('  g_enabled = g_device.HasFeature(wgpu::FeatureName::TimestampQuery);','''  const auto* path = std::getenv("PETARI_GPU_TIMING_CSV");
#ifndef TRACY_ENABLE
  if (!path || !*path) return;
#endif
  g_enabled = g_device.HasFeature(wgpu::FeatureName::TimestampQuery);
  if (path && *path) {
    if (g_enabled) { g_csv = std::fopen(path,"w"); if(g_csv) std::fprintf(g_csv,"frame,gpu_ms,gx_ms,timestamp_status\\n"); }
    std::fprintf(stderr,"[gx gpu timing] available=%u csv_open=%u columns=frame,gpu_ms,gx_ms,timestamp_status gx_ms=unavailable_mixed_passes\\n",g_enabled,g_csv!=nullptr);
  }''')
 replace('  TracyPlotConfig("aurora: gpuFrameMs",','#ifdef TRACY_ENABLE\n  TracyPlotConfig("aurora: gpuFrameMs",')
 replace('  Log.info("GPU profiling enabled ({} zones max)", MaxZones);','#endif\n  Log.info("GPU profiling enabled ({} zones max)", MaxZones);')
 replace('void shutdown() {','void shutdown() {\n  after_submit();\n  if(g_csv){std::fclose(g_csv);g_csv=nullptr;}')
 replace('  auto& slot = record_slot();\n  if (slot.state != SlotState::Free) {','  auto& slot = record_slot();\n  const auto csvFrame = g_csvFrame++;\n  if (slot.state != SlotState::Free) {\n    if(g_csv) std::fprintf(g_csv,"%llu,,,readback_busy\\n",(unsigned long long)csvFrame);')
 replace('  slot.state = SlotState::Recording;','  slot.frame = csvFrame;\n  slot.state = SlotState::Recording;')
 # Unavailable timestamp pairs still need a CSV row.
 replace('  if (frameBegin == 0 || frameEnd <= frameBegin) {\n    return;','  if (frameBegin == 0 || frameEnd <= frameBegin) {\n    if(g_csv) std::fprintf(g_csv,"%llu,,,invalid_timestamp\\n",(unsigned long long)slot.frame);\n    return;')
output.parent.mkdir(parents=True,exist_ok=True)
if not output.exists() or output.read_text()!=text:output.write_text(text)
