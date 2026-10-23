// Included in the GX shader TU, where the existing decode/TEV helpers are visible.
#include "ubershader.hpp"
#include "../gfx/recording.hpp"
#include "../gfx/encoding.hpp"

#include <atomic>
#include <cmath>
#include <chrono>
#include <map>
#include <sqlite3.h>
#include <filesystem>
namespace aurora::gx {
thread_local bool petariBuildingUber = false;
thread_local double petariUberShaderMs = 0;
bool petari_uber_enabled() { static const bool b = [] { auto p = std::getenv("PETARI_UBERSHADER"); return p && (std::strcmp(p,"1")==0 || std::strcmp(p,"all")==0); }(); return b; }
bool petari_uber_all() { static const bool b = [] { auto p = std::getenv("PETARI_UBERSHADER"); return p && std::strcmp(p,"all")==0; }(); return b; }
const char* petari_uber_reason(const ShaderConfig& c) {
  if (c.lineMode) return "primitive";
  if (c.fogType != GX_FOG_NONE || c.fogRangeEnabled) return "fog";
  if (c.numIndStages) return "indirect";
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
    if(s.alphaOp.op!=GX_TEV_ADD && s.alphaOp.op!=GX_TEV_SUB && s.alphaOp.op!=GX_TEV_COMP_A8_GT && s.alphaOp.op!=GX_TEV_COMP_A8_EQ) return "alpha_op";
    if((s.kcSel>=8 && s.kcSel<12) || (s.kaSel>=8 && s.kaSel<16)) return "konst_selector";
    if(s.indTexMtxId!=GX_ITM_OFF || s.indTexWrapS!=GX_ITW_OFF || s.indTexWrapT!=GX_ITW_OFF || s.indTexAddPrev || s.channelId==GX_ALPHA_BUMP || s.channelId==GX_ALPHA_BUMPN) return "indirect";
  }
  for(unsigned i=0;i<4;++i) if(info.sampledColorChannels.test(i%2)) {
    const auto& cc=c.colorChannels[i];if(cc.lightingEnabled && unsigned(cc.attnFn)>GX_AF_NONE) return "lighting";
  }
  return "covered";
}
namespace {
std::mutex uberMutex;
std::map<gfx::PipelineRef,gfx::CompiledPipeline> uberPipelines;
std::map<HashType,bool> uberConfigs;
std::map<gfx::PipelineRef,gfx::PipelineRef> uberAssociations;
std::atomic<uint64_t> uberDraws{0},uberCovered{0},uberFallback{0},uberForced{0},uberSpecialized{0},uberMissing{0};
}
void petari_uber_note(const ShaderConfig& c) {
  if(!petari_uber_enabled()) return;
  ++uberDraws; const auto reason=petari_uber_reason(c); const bool covered=std::strcmp(reason,"covered")==0; uberCovered+=covered;
  const auto key=xxh3_hash(c); std::lock_guard lock(uberMutex);
  if(uberConfigs.emplace(key,covered).second) std::fprintf(stderr,"[gx uber config] config=%016llx covered=%u reason=%s\n",(unsigned long long)key,covered,reason);
}
void petari_uber_draw(bool fallback,bool forced,bool missing) { uberFallback+=fallback&&!forced; uberForced+=forced; uberSpecialized+=!fallback; uberMissing+=missing; }
void petari_uber_report() {
  if(!petari_uber_enabled()) return;
  std::lock_guard lock(uberMutex); size_t covered=0;for(auto [k,v]:uberConfigs) covered+=v;
  std::fprintf(stderr,"[gx uber summary] draws=%llu covered_draws=%llu fallback_draws=%llu forced_draws=%llu specialized_draws=%llu unsupported_draws=%llu missing_key_draws=%llu configs=%zu covered_configs=%zu\n",(unsigned long long)uberDraws.load(),(unsigned long long)uberCovered.load(),(unsigned long long)uberFallback.load(),(unsigned long long)uberForced.load(),(unsigned long long)uberSpecialized.load(),(unsigned long long)(uberDraws-uberCovered),(unsigned long long)uberMissing.load(),uberConfigs.size(),covered);
}
gfx::PipelineRef petari_uber_key(const PipelineConfig& c,const gfx::RenderTargetLayout& l) { auto key=c; key.shaderConfig={}; return xxh3_hash(l.key,xxh3_hash(key,0x55424552)); }
void petari_uber_associate(gfx::PipelineRef key,gfx::PipelineRef uber) { std::lock_guard lock(uberMutex);uberAssociations.emplace(key,uber); }
gfx::PipelineRef petari_uber_lookup(gfx::PipelineRef key) {std::lock_guard lock(uberMutex);auto it=uberAssociations.find(key);return it==uberAssociations.end()?0:it->second;}
bool petari_uber_ready(gfx::PipelineRef key,gfx::CompiledPipeline& out) { std::lock_guard lock(uberMutex);auto it=uberPipelines.find(key);if(it==uberPipelines.end() || !it->second.main) return false;out=it->second;return true; }
void petari_uber_prepare(const PipelineConfig& c,const gfx::RenderTargetLayout& l) {
  if(!petari_uber_enabled() || std::strcmp(petari_uber_reason(c.shaderConfig),"covered")) return;
  auto key=petari_uber_key(c,l); { std::lock_guard lock(uberMutex);if(uberPipelines.contains(key)) return; }
  auto start=std::chrono::steady_clock::now(); petariUberShaderMs=0;petariBuildingUber=true;auto p=create_pipeline(c,l);petariBuildingUber=false;
  auto ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
  std::fprintf(stderr,"[gx uber compile] key=%016llx layout=%016llx shader_ms=%.3f pipeline_ms=%.3f success=%u\n",(unsigned long long)key,(unsigned long long)l.key,petariUberShaderMs,std::max(0.0,ms-petariUberShaderMs),(bool)p.main);
  std::lock_guard lock(uberMutex);uberPipelines.emplace(key,std::move(p));
}
// Fixed layout: cfg 1024 packed bytes, projection, matrices, TEV regs, lights, channel state,
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
  }
  for(unsigned i=0;i<=GX_VA_TEX7;++i) {auto& a=c.attrs[i];unsigned o=512+i*8;w[o]=a.attrType;w[o+1]=a.cnt;w[o+2]=a.compType;w[o+3]=a.offset;w[o+4]=a.stride;w[o+5]=a.frac;w[o+6]=a.le;}
  for(unsigned i=0;i<8;++i) {auto& t=c.tcgs[i];unsigned o=704+i*8;w[o]=t.type;w[o+1]=t.src;w[o+2]=t.mtx;w[o+3]=t.postMtx;w[o+4]=t.normalize;}
  for(unsigned i=0;i<4;++i) {auto& x=c.colorChannels[i];unsigned o=768+i*8;w[o]=x.matSrc;w[o+1]=x.ambSrc;w[o+2]=x.diffFn;w[o+3]=x.attnFn;w[o+4]=x.lightingEnabled;w[o+5]=g_gxState.colorChannelState[i].lightMask.to_ulong();}
  w[800]=c.alphaCompare.comp0;w[801]=c.alphaCompare.ref0;w[802]=c.alphaCompare.comp1;w[803]=c.alphaCompare.ref1;w[820]=c.alphaCompare.op;
  for(unsigned i=0;i<4;++i) {auto& s=c.tevSwapTable[i];w[804+i*4]=s.red;w[805+i*4]=s.green;w[806+i*4]=s.blue;w[807+i*4]=s.alpha;}
  w[821]=c.vtxStride;w[822]=c.tevStageCount;w[823]=c.attrs[GX_VA_NRM].attrType!=GX_NONE;
  auto info=build_shader_info(c);w[824]=info.sampledTexCoords.to_ulong();
  b.append(w);auto proj=g_gxState.proj;if constexpr(UseReversedZ) proj.m2=proj.m2*Vec4{-1.f,-1.f,-1.f,-1.f};else proj.m2=proj.m2+proj.m3;b.append(proj);
  for(auto& x:g_gxState.pnMtx)b.append(x.pos);for(auto& x:g_gxState.texMtxs)b.append(x);for(auto& x:g_gxState.pnMtx)b.append(x.nrm);
  b.append(g_gxState.colorRegs);b.append(g_gxState.lights);
  for(auto& x:g_gxState.colorChannelState){b.append(x.matColor);b.append(x.ambColor);}
  b.append(g_gxState.kcolors);b.append(g_gxState.ptTexMtxs);
  for(unsigned i=0;i<8;++i) b.append(petari_uber_texture_bias(get_texture(static_cast<GXTexMapID>(i))));
  return gfx::push_uniform(b.data(),b.size());
}
}
