// Included in the GX shader TU, where the existing decode/TEV helpers are visible.
#include "ubershader.hpp"
#include "ubershader_programs.hpp"
#include "../gfx/recording.hpp"
#include "../gfx/encoding.hpp"

#include <petari/ubershader_mode.hpp>
#include <atomic>
#include <cmath>
#include <chrono>
#include <map>
#include <sqlite3.h>
#include <filesystem>
#include <exception>
namespace aurora::gx {
thread_local bool petariBuildingUber = false;
thread_local PetariUberTier petariBuildingUberTier = PetariUberTier::Fast;
thread_local double petariUberShaderMs = 0;
bool petari_uber_enabled() { static const bool b = [] { return PetariNative::Ubershaders::resolve(std::getenv("PETARI_UBERSHADER")).mode != PetariNative::Ubershaders::Mode::Off; }(); return b; }
bool petari_uber_all() { static const bool b = [] { auto p = std::getenv("PETARI_UBERSHADER"); return p && std::strcmp(p,"all")==0; }(); return b; }
bool petari_uber_diagnostics() {
  static const bool enabled = [] {
    const auto on = [](const char* key) { auto p = std::getenv(key); return p && std::strcmp(p,"1")==0; };
    return on("PETARI_UBERSHADER_DIAG") || on("PETARI_PIPELINE_DIAG");
  }();
  return enabled;
}
bool petari_uber_needs_uniform(gfx::PipelineRef key, bool& specializedReady) {
  if (!petari_uber_enabled()) return false;
  if (petari_uber_all()) return true;
  // Published specialised pipelines remain ready for the device/cache lifetime.
  if (!specializedReady) specializedReady = gfx::petari_specialized_ready(key);
  return !specializedReady;
}
bool petari_uber_select_pipeline(const DrawData& data, gfx::CompiledPipeline& pipeline, bool& uber) {
  uber = false;
  const bool enabled = petari_uber_enabled();
  const bool covered = data.uberUniformOffset != UINT32_MAX;
  const bool force = enabled && covered && petari_uber_all();
  if (!force && gfx::get_pipeline(data.pipeline, pipeline)) {
    if (enabled) petari_uber_draw(false, false, false, false);
    return true;
  }
  if (enabled && covered) {
    bool fast = false;
    const auto key = petari_uber_lookup(data.pipeline, &fast);
    if (petari_uber_ready(key, pipeline)) {
      uber = true;
      petari_uber_draw(true, force, false, fast);
      return true;
    }
    petari_uber_not_ready(key);
    // Forced mode retains the existing specialised wait/skip when fallback prep
    // is pending. A native pipeline can become ready between record and encode.
    if (force && gfx::get_pipeline(data.pipeline, pipeline)) {
      petari_uber_draw(false, false, true, false);
      return true;
    }
    petari_uber_not_ready(key, true);
  }
  return false;
}
const char* petari_uber_reason(const ShaderConfig& c) {
  if (c.lineMode) return "primitive";
  switch(c.fogType) {
    case GX_FOG_NONE:case GX_FOG_PERSP_LIN:case GX_FOG_PERSP_EXP:case GX_FOG_PERSP_EXP2:case GX_FOG_PERSP_REVEXP:case GX_FOG_PERSP_REVEXP2:
    case GX_FOG_ORTHO_LIN:case GX_FOG_ORTHO_EXP:case GX_FOG_ORTHO_EXP2:case GX_FOG_ORTHO_REVEXP:case GX_FOG_ORTHO_REVEXP2:break;
    default:return "fog_type";
  }
  if(c.numIndStages>4)return "indirect_count";
  if (!c.tevStageCount || c.tevStageCount > 16) return "stage_count";
  if (c.attrs[GX_VA_POS].attrType == GX_NONE) return "position";
  for (unsigned a=0;a<=GX_VA_TEX7;++a) {
    auto& v=c.attrs[a]; if(v.attrType==GX_NONE) continue;
    if(a<GX_VA_POS) { if(v.attrType!=GX_DIRECT) return "matrix_index"; continue; }
    if(v.attrType>GX_INDEX16 || v.nbt3) return "attribute";
    if(a==GX_VA_CLR0 || a==GX_VA_CLR1) { if(v.compType>GX_RGBA8) return "color_format"; }
    else if(v.compType>GX_F32 || (a==GX_VA_POS && v.cnt!=2 && v.cnt!=3) ||
      (a==GX_VA_NRM && v.cnt!=3) || (a>=GX_VA_TEX0 && v.cnt!=1 && v.cnt!=2)) return "attribute_format";
  }
  auto info=build_shader_info(c);
  for(unsigned i=0;i<8;++i) if(info.sampledTexCoords.test(i)) {
    const auto& t=c.tcgs[i];
    if(t.type!=GX_TG_MTX2x4 && t.type!=GX_TG_MTX3x4) return "texgen_type";
    if(t.src!=GX_TG_POS && t.src!=GX_TG_NRM && !(t.src>=GX_TG_TEX0 && t.src<=GX_TG_TEX7)) return "texgen_source";
  }
  for(unsigned i=0;i<c.tevStageCount;++i) {
    auto& s=c.tevStages[i];
    if(unsigned(s.tevSwapTex)>=c.tevSwapTable.size() || unsigned(s.tevSwapRas)>=c.tevSwapTable.size())return "swap_selector";
    if(s.alphaOp.op!=GX_TEV_ADD && s.alphaOp.op!=GX_TEV_SUB && s.alphaOp.op!=GX_TEV_COMP_A8_GT && s.alphaOp.op!=GX_TEV_COMP_A8_EQ) return "alpha_op";
    if((s.kcSel>=8 && s.kcSel<12) || (s.kaSel>=8 && s.kaSel<16)) return "konst_selector";
    if(unsigned(s.indTexStage)>3 || unsigned(s.indTexFormat)>3 || unsigned(s.indTexBiasSel)>7 || unsigned(s.indTexAlphaSel)>3 ||
       unsigned(s.indTexWrapS)>6 || unsigned(s.indTexWrapT)>6 ||
       !(s.indTexMtxId==GX_ITM_OFF || (s.indTexMtxId>=GX_ITM_0 && s.indTexMtxId<=GX_ITM_2) ||
         (s.indTexMtxId>=GX_ITM_S0 && s.indTexMtxId<=GX_ITM_S2) || (s.indTexMtxId>=GX_ITM_T0 && s.indTexMtxId<=GX_ITM_T2))) return "indirect_format";
  }
  for(unsigned i=0;i<4;++i) if(info.usedIndStages.test(i)) {
    const auto& ind=c.indStages[i];if(unsigned(ind.texCoordId)>7 || unsigned(ind.texMapId)>7 || unsigned(ind.scaleS)>8 || unsigned(ind.scaleT)>8)return "indirect_source";
  }
  for(unsigned i=0;i<4;++i) if(info.sampledColorChannels.test(i%2)) {
    const auto& cc=c.colorChannels[i];if(cc.lightingEnabled && unsigned(cc.attnFn)>GX_AF_NONE) return "lighting";
  }
  return "covered";
}
namespace {
std::mutex uberMutex;
std::map<gfx::PipelineRef,gfx::CompiledPipeline> uberPipelines;
enum class UberPreparation { Queued, Preparing, Ready, Failed };
std::map<gfx::PipelineRef,UberPreparation> uberPreparation;
std::atomic<uint64_t> uberPendingDraws{0},uberFailedDraws{0},uberUnknownDraws{0},uberSkippedUnready{0};
std::map<HashType,bool> uberConfigs;
std::map<gfx::PipelineRef,PetariUberKeys> uberAssociations;
thread_local uint64_t uberEncodingFrame=0;
thread_local bool uberFrameUsed=false;
thread_local bool uberFrameForced=false,uberFrameGeneric=false,uberFrameFast=false;
std::atomic<uint64_t> uberFrames{0},uberFallbackFrames{0},uberForcedFrames{0};
std::atomic<uint64_t> uberGenericFrames{0},uberFastFrames{0},uberMixedFrames{0},uberGenericDraws{0},uberFastDraws{0};
std::atomic<uint64_t> uberDraws{0},uberCovered{0},uberFallback{0},uberForced{0},uberSpecialized{0},uberMissing{0};
}
void petari_uber_note(const ShaderConfig& c) {
  if(!petari_uber_enabled() || !petari_uber_diagnostics()) return;
  ++uberDraws; const auto reason=petari_uber_reason(c); const bool covered=std::strcmp(reason,"covered")==0; uberCovered+=covered;
  const auto key=xxh3_hash(c); std::lock_guard lock(uberMutex);
  if(uberConfigs.emplace(key,covered).second) std::fprintf(stderr,"[gx uber config] config=%016llx covered=%u reason=%s\n",(unsigned long long)key,covered,reason);
}
static thread_local uint64_t petariUberRecordingFrame=0;
void petari_uber_set_recording_frame(uint64_t frame) {petariUberRecordingFrame=frame;}
uint64_t petari_uber_recording_frame() {return petariUberRecordingFrame;}
void petari_uber_begin_frame(uint64_t frame) {
  if(!petari_uber_enabled()) return;
  uberEncodingFrame=frame;uberFrameUsed=false;uberFrameForced=false;uberFrameGeneric=false;uberFrameFast=false;++uberFrames;
}
void petari_uber_end_frame() {
  if(!petari_uber_enabled() || !uberFrameUsed)return;
  if(uberFrameForced)++uberForcedFrames;else ++uberFallbackFrames;
  uberGenericFrames+=uberFrameGeneric;uberFastFrames+=uberFrameFast;uberMixedFrames+=uberFrameGeneric&&uberFrameFast;
  std::fprintf(stderr,"[gx uber frame] frame=%llu fallback=1 forced=%u generic=%u fast=%u\n",(unsigned long long)uberEncodingFrame,uberFrameForced,uberFrameGeneric,uberFrameFast);
  uberFrameUsed=false;
}
void petari_uber_draw(bool fallback,bool forced,bool missing,bool fast) {
  uberFallback+=fallback&&!forced;uberForced+=forced;uberSpecialized+=!fallback;
  (void)missing; // observed once before the draw's possible early return
  if(fallback) {
    uberFrameUsed=true;
    uberFrameForced|=forced;uberFrameFast|=fast;uberFrameGeneric|=!fast;
    if(fast)++uberFastDraws;else ++uberGenericDraws;
  }
}
void petari_uber_not_ready(gfx::PipelineRef key, bool skipped) {
  if (!petari_uber_enabled()) return;
  if (skipped) { ++uberSkippedUnready; return; }
  ++uberMissing;
  std::lock_guard lock(uberMutex);
  const auto it = uberPreparation.find(key);
  if (it == uberPreparation.end()) ++uberUnknownDraws;
  else if (it->second == UberPreparation::Failed) ++uberFailedDraws;
  else ++uberPendingDraws; // readiness was observed before taking this mutex
}
void petari_uber_scheduled(gfx::PipelineRef key) {
  std::lock_guard lock(uberMutex);
  uberPreparation.try_emplace(key, UberPreparation::Queued);
}
void petari_uber_release() {
  std::lock_guard lock(uberMutex);
  uberPipelines.clear();
  uberPreparation.clear();
  uberAssociations.clear();
}
void petari_uber_report() {
  if(!petari_uber_enabled()) return;
  std::lock_guard lock(uberMutex); size_t covered=0;for(auto [k,v]:uberConfigs) covered+=v;
  std::fprintf(stderr,"[gx uber frames] encoded_frames=%llu fallback_frames=%llu forced_frames=%llu generic_frames=%llu fast_frames=%llu mixed_frames=%llu\n",(unsigned long long)uberFrames.load(),(unsigned long long)uberFallbackFrames.load(),(unsigned long long)uberForcedFrames.load(),(unsigned long long)uberGenericFrames.load(),(unsigned long long)uberFastFrames.load(),(unsigned long long)uberMixedFrames.load());
  std::fprintf(stderr,"[gx uber tiers] generic_draws=%llu fast_draws=%llu\n",(unsigned long long)uberGenericDraws.load(),(unsigned long long)uberFastDraws.load());
  std::fprintf(stderr,"[gx uber summary] draws=%llu covered_draws=%llu fallback_draws=%llu forced_draws=%llu specialized_draws=%llu unsupported_draws=%llu missing_key_draws=%llu configs=%zu covered_configs=%zu\n",(unsigned long long)uberDraws.load(),(unsigned long long)uberCovered.load(),(unsigned long long)uberFallback.load(),(unsigned long long)uberForced.load(),(unsigned long long)uberSpecialized.load(),(unsigned long long)(uberDraws-uberCovered),(unsigned long long)uberMissing.load(),uberConfigs.size(),covered);
  std::fprintf(stderr,"[gx uber readiness] not_ready_draws=%llu pending_draws=%llu failed_draws=%llu unknown_draws=%llu skipped_unready_draws=%llu\n",
      (unsigned long long)uberMissing.load(),(unsigned long long)uberPendingDraws.load(),
      (unsigned long long)uberFailedDraws.load(),(unsigned long long)uberUnknownDraws.load(),
      (unsigned long long)uberSkippedUnready.load());
}
uint32_t petari_uber_arg_program(const ShaderConfig& c) {
  for(unsigned p=0;p<PetariUberArgPrograms.size();++p) {
    const auto& program=PetariUberArgPrograms[p];if(program.count!=c.tevStageCount)continue;bool match=true;
    for(unsigned i=0;i<c.tevStageCount && match;++i) {
      const auto& t=c.tevStages[i];const std::array<uint32_t,10> args={t.colorPass.a,t.colorPass.b,t.colorPass.c,t.colorPass.d,t.alphaPass.a,t.alphaPass.b,t.alphaPass.c,t.alphaPass.d,t.colorOp.outReg,t.alphaOp.outReg};
      match=std::equal(args.begin(),args.end(),program.stages[i].begin());
    }
    if(match)return p+1;
  }
  return 0;
}
// Match the specialised shader's static discard capability. Keeping a kill
// instruction in an always-passing shader exposed sparse lava edges on Metal;
// preserve the native shader's capability rather than a runtime no-op test.
// Limit the extra variant to indirect shaders: those need the derivative-sensitive
// sampling fix, while a global split delayed cold stage-entry fallback readiness.
static bool petari_uber_needs_discard(const ShaderConfig& c) {
  if (!c.alphaCompare) return false;
  const auto a=alpha_compare(c.alphaCompare.comp0,c.alphaCompare.ref0);
  const auto b=alpha_compare(c.alphaCompare.comp1,c.alphaCompare.ref1);
  AlphaCompareExpr pass;
  switch(c.alphaCompare.op) {
  case GX_AOP_AND: pass=alpha_compare_and(a,b);break;
  case GX_AOP_OR: pass=alpha_compare_or(a,b);break;
  case GX_AOP_XOR: pass=alpha_compare_xor(a,b);break;
  case GX_AOP_XNOR: pass=alpha_compare_xnor(a,b);break;
  default: std::abort();
  }
  return alpha_compare_not(pass).constant != 0;
}
uint32_t petari_uber_variant(const ShaderConfig& c,PetariUberTier tier) {
  const auto info=build_shader_info(c);uint32_t extent=0;
  for(unsigned i=0;i<8;++i)if(info.sampledTexCoords.test(i))extent=i+1;
  const auto generic=c.tevStageCount | (extent<<5) | (c.numIndStages<<9) | (uint32_t(c.fogType!=GX_FOG_NONE)<<12) | (uint32_t(c.numIndStages && !petari_uber_needs_discard(c))<<14);
  // Share the correct discard-free Generic module instead of preparing a second
  // upgrade for these indirect shaders; cold entry readiness takes precedence.
  if(tier==PetariUberTier::Generic || (generic&(1u<<14)))return generic;
  bool identity=true;
  const auto isIdentity=[](const auto& s){return s.red==GX_CH_RED && s.green==GX_CH_GREEN && s.blue==GX_CH_BLUE && s.alpha==GX_CH_ALPHA;};
  for(unsigned i=0;i<c.tevStageCount;++i){const auto& t=c.tevStages[i];identity&=isIdentity(c.tevSwapTable[t.tevSwapTex]) && isIdentity(c.tevSwapTable[t.tevSwapRas]);}
  return generic | (uint32_t(identity)<<13) | (petari_uber_arg_program(c)<<24);
}
uint32_t petari_uber_variant(const ShaderConfig& c) {return petari_uber_variant(c,PetariUberTier::Fast);}
gfx::PipelineRef petari_uber_key(const PipelineConfig& c,const gfx::RenderTargetLayout& l,PetariUberTier tier) {
  auto key=c;key.shaderConfig={};
  return xxh3_hash(l.key,xxh3_hash(petari_uber_variant(c.shaderConfig,tier),xxh3_hash(key,0x55424552)));
}
gfx::PipelineRef petari_uber_key(const PipelineConfig& c,const gfx::RenderTargetLayout& l) {return petari_uber_key(c,l,PetariUberTier::Fast);}
PetariUberKeys petari_uber_keys(const PipelineConfig& c,const gfx::RenderTargetLayout& l) {return {petari_uber_key(c,l,PetariUberTier::Generic),petari_uber_key(c,l,PetariUberTier::Fast)};}
void petari_uber_associate(gfx::PipelineRef key,PetariUberKeys uber) { std::lock_guard lock(uberMutex);uberAssociations.emplace(key,uber); }
gfx::PipelineRef petari_uber_lookup(gfx::PipelineRef key,bool* fast) {
  std::lock_guard lock(uberMutex);if(fast)*fast=false;
  const auto it=uberAssociations.find(key);if(it==uberAssociations.end())return 0;
  const auto& keys=it->second;
  const auto preferred=uberPipelines.find(keys.fast);
  if(keys.fast!=keys.generic && preferred!=uberPipelines.end() && preferred->second.main){if(fast)*fast=true;return keys.fast;}
  return keys.generic;
}
bool petari_uber_ready(gfx::PipelineRef key,gfx::CompiledPipeline& out) { std::lock_guard lock(uberMutex);auto it=uberPipelines.find(key);if(it==uberPipelines.end() || !it->second.main) return false;out=it->second;return true; }
void petari_uber_prepare(const PipelineConfig& c,const gfx::RenderTargetLayout& l,PetariUberTier tier) {
  if(!petari_uber_enabled() || std::strcmp(petari_uber_reason(c.shaderConfig),"covered")) return;
  const auto key=petari_uber_key(c,l,tier);
  {
    std::lock_guard lock(uberMutex);
    // Claim before compiling: concurrent callers see Preparing, never build twice.
    if (!uberPipelines.try_emplace(key).second) return;
    uberPreparation.insert_or_assign(key, UberPreparation::Preparing);
  }
  const auto start=std::chrono::steady_clock::now();
  const bool previousBuilding=petariBuildingUber;
  const auto previousTier=petariBuildingUberTier;
  const double previousShaderMs=petariUberShaderMs;
  petariUberShaderMs=0;petariBuildingUber=true;petariBuildingUberTier=tier;
  gfx::CompiledPipeline p;
  try { p=create_pipeline(c,l); }
  catch (const std::exception& e) { std::fprintf(stderr,"[gx uber async error] key=%016llx message=%s\n",(unsigned long long)key,e.what()); }
  catch (...) { std::fprintf(stderr,"[gx uber async error] key=%016llx message=unknown_exception\n",(unsigned long long)key); }
  const double shaderMs=petariUberShaderMs;
  petariBuildingUber=previousBuilding;petariUberShaderMs=previousShaderMs;petariBuildingUberTier=previousTier;
  auto ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
  const bool success=bool(p.main);
  {
    std::lock_guard lock(uberMutex);
    uberPipelines.at(key)=std::move(p);
    uberPreparation.at(key)=success ? UberPreparation::Ready : UberPreparation::Failed;
  }
  std::fprintf(stderr,"[gx uber compile] key=%016llx layout=%016llx shader_ms=%.3f pipeline_ms=%.3f success=%u tier=%s\n",(unsigned long long)key,(unsigned long long)l.key,shaderMs,std::max(0.0,ms-shaderMs),success,tier==PetariUberTier::Generic?"generic":"fast");
}
void petari_uber_prepare(const PipelineConfig& c,const gfx::RenderTargetLayout& l) {petari_uber_prepare(c,l,PetariUberTier::Fast);}
// Fixed layout: 4768 bytes, cfg 1024 packed bytes, projection, matrices, TEV regs, lights, channel state,
// konst colors, post matrices, texture size/bias. All arrays have 16-byte strides.
static Vec4<float> petari_uber_texture_bias(const gfx::TextureBind& tex) {
  float bias=0;
  if(tex.ref && tex.ref->hasArbitraryMips) {
    float scale=std::min(g_gxState.renderViewport.width/std::max(g_gxState.logicalViewport.width,1.f),g_gxState.renderViewport.height/std::max(g_gxState.logicalViewport.height,1.f));
    float repl=float(tex.ref->size.width)/std::max(float(tex.texObj.width()),1.f);bias=std::log2(scale/std::max(repl,0.001f));
  }
  return {float(tex.texObj.width()),float(tex.texObj.height()),tex.texObj.lod_bias()+bias,0.f};
}
gfx::Range petari_uber_uniform(const ShaderConfig& c) {
  ByteBuffer b; std::array<u8,1024> w{};
  for(unsigned i=0;i<16;++i) { auto& s=c.tevStages[i];unsigned o=i*32;
    w[o]=s.colorPass.a;w[o+1]=s.colorPass.b;w[o+2]=s.colorPass.c;w[o+3]=s.colorPass.d;
    w[o+4]=s.alphaPass.a;w[o+5]=s.alphaPass.b;w[o+6]=s.alphaPass.c;w[o+7]=s.alphaPass.d;
    w[o+8]=s.colorOp.op;w[o+9]=s.colorOp.bias;w[o+10]=s.colorOp.scale;w[o+11]=s.colorOp.clamp;w[o+12]=s.colorOp.outReg;
    w[o+13]=s.alphaOp.op;w[o+14]=s.alphaOp.bias;w[o+15]=s.alphaOp.scale;w[o+16]=s.alphaOp.clamp;w[o+17]=s.alphaOp.outReg;
    w[o+18]=s.kcSel;w[o+19]=s.kaSel;w[o+20]=s.texCoordId;w[o+21]=s.texMapId;w[o+22]=s.channelId;w[o+23]=s.tevSwapRas;w[o+24]=s.tevSwapTex;
    w[o+25]=unsigned(s.indTexStage)|(unsigned(s.indTexFormat)<<2)|(unsigned(s.indTexAlphaSel)<<4);
    w[o+26]=s.indTexBiasSel;w[o+27]=s.indTexMtxId;w[o+28]=s.indTexWrapS;w[o+29]=s.indTexWrapT;w[o+30]=s.indTexAddPrev;w[o+31]=uses_texture_sample(s);
  }
  for(unsigned i=0;i<=GX_VA_TEX7;++i) {auto& a=c.attrs[i];unsigned o=512+i*8;w[o]=a.attrType;w[o+1]=a.cnt;w[o+2]=a.compType;w[o+3]=a.offset;w[o+4]=a.stride;w[o+5]=a.frac;w[o+6]=a.le;}
  for(unsigned i=0;i<8;++i) {auto& t=c.tcgs[i];unsigned o=704+i*8;w[o]=t.type;w[o+1]=t.src;w[o+2]=t.mtx;w[o+3]=t.postMtx;w[o+4]=t.normalize;}
  for(unsigned i=0;i<4;++i) {auto& x=c.colorChannels[i];unsigned o=768+i*8;w[o]=x.matSrc;w[o+1]=x.ambSrc;w[o+2]=x.diffFn;w[o+3]=x.attnFn;w[o+4]=x.lightingEnabled;w[o+5]=g_gxState.colorChannelState[i].lightMask.to_ulong();}
  w[800]=c.alphaCompare.comp0;w[801]=c.alphaCompare.ref0;w[802]=c.alphaCompare.comp1;w[803]=c.alphaCompare.ref1;w[820]=c.alphaCompare.op;
  for(unsigned i=0;i<4;++i) {auto& s=c.tevSwapTable[i];w[804+i*4]=s.red;w[805+i*4]=s.green;w[806+i*4]=s.blue;w[807+i*4]=s.alpha;}
  w[821]=c.vtxStride;w[822]=c.tevStageCount;w[823]=c.attrs[GX_VA_NRM].attrType!=GX_NONE;
  auto info=build_shader_info(c);w[824]=info.sampledTexCoords.to_ulong();
  w[825]=c.numIndStages;w[826]=c.fogType;w[827]=c.fogRangeEnabled;w[829]=info.usedIndStages.to_ulong();
  for(unsigned i=0;i<4;++i){const auto& ind=c.indStages[i];unsigned o=832+i*4;w[o]=ind.texCoordId;w[o+1]=ind.texMapId;w[o+2]=ind.scaleS;w[o+3]=ind.scaleT;}
  b.append(w);auto proj=g_gxState.proj;if constexpr(UseReversedZ) proj.m2=proj.m2*Vec4{-1.f,-1.f,-1.f,-1.f};else proj.m2=proj.m2+proj.m3;b.append(proj);
  for(auto& x:g_gxState.pnMtx)b.append(x.pos);for(auto& x:g_gxState.texMtxs)b.append(x);for(auto& x:g_gxState.pnMtx)b.append(x.nrm);
  b.append(g_gxState.colorRegs);b.append(g_gxState.lights);
  for(auto& x:g_gxState.colorChannelState){b.append(x.matColor);b.append(x.ambColor);}
  b.append(g_gxState.kcolors);b.append(g_gxState.ptTexMtxs);
  for(unsigned i=0;i<8;++i) b.append(petari_uber_texture_bias(get_texture(static_cast<GXTexMapID>(i))));
  b.append(g_gxState.fog.color);b.append(Vec4{g_gxState.fog.a,g_gxState.fog.b,g_gxState.fog.c,0.f});
  for(const auto& scale:g_gxState.texCoordScales)b.append(Vec4{float(scale.scaleS)+1.f,float(scale.scaleT)+1.f,0.f,0.f});
  for(const auto& m:g_gxState.indTexMtxs){b.append(Vec4{m.mtx.m0.x,m.mtx.m0.y,m.mtx.m1.x,m.mtx.m1.y});b.append(Vec4{m.mtx.m2.x,m.mtx.m2.y,std::exp2f(m.scaleExp),0.f});}
  return gfx::push_uniform(b.data(),b.size());
}
}
