#pragma once
// Included AFTER the patched production GX/Clear declarations, never their
// unpatched duplicate definitions. The ledger makes silent layout drift a build error.
#include "pipeline_recipe_cache.hpp"
#include "pipeline_texgen_validity.hpp"
#include <cstddef>

namespace PetariPipelineCache {
using GXRecipe=aurora::gx::PipelineConfig;
using ClearRecipe=aurora::gfx::clear::PipelineConfig;
using ShaderRecipe=aurora::gx::ShaderConfig;
static_assert(sizeof(GXRecipe)==PETARI_CACHE_GX_CONFIG_SIZE && sizeof(ClearRecipe)==PETARI_CACHE_CLEAR_CONFIG_SIZE);
static_assert(aurora::gx::GXPipelineConfigVersion==PETARI_CACHE_GX_CONFIG_VERSION && aurora::gfx::clear::ClearPipelineConfigVersion==PETARI_CACHE_CLEAR_CONFIG_VERSION);
static_assert(sizeof(ShaderRecipe)==2720 && offsetof(GXRecipe,shaderConfig)==8);
static_assert(offsetof(ShaderRecipe,attrs)==4 && offsetof(ShaderRecipe,tevSwapTable)==212 && offsetof(ShaderRecipe,tevStages)==276);
static_assert(offsetof(ShaderRecipe,tevStageCount)==2388 && offsetof(ShaderRecipe,colorChannels)==2392 && offsetof(ShaderRecipe,tcgs)==2472);
static_assert(offsetof(ShaderRecipe,alphaCompare)==2632 && offsetof(ShaderRecipe,indStages)==2652 && offsetof(ShaderRecipe,numIndStages)==2716);
static_assert(sizeof(aurora::gx::AttrConfig)==8 && sizeof(aurora::gx::TevStage)==132 && sizeof(aurora::gx::TevOp)==20 && sizeof(aurora::gx::TcgConfig)==20 && sizeof(aurora::gx::ColorChannelConfig)==20);
static_assert(offsetof(GXRecipe,depthFunc)==2728 && offsetof(GXRecipe,cullMode)==2732 && offsetof(GXRecipe,blendMode)==2736 && offsetof(GXRecipe,blendFacSrc)==2740 && offsetof(GXRecipe,blendFacDst)==2744 && offsetof(GXRecipe,blendOp)==2748 && offsetof(GXRecipe,dstAlpha)==2752);
static_assert(offsetof(GXRecipe,polygonOffsetBits)==2756 && offsetof(GXRecipe,polygonOffsetScaleBits)==2760 && offsetof(GXRecipe,polygonOffsetClampBits)==2764 && offsetof(GXRecipe,depthCompare)==2768 && offsetof(GXRecipe,colorUpdate)==2771);
static_assert(GX_IDENTITY==60 && GX_PTIDENTITY==125 && GX_TEXMTX0==30 && GX_PTTEXMTX0==64);
static_assert(std::endian::native==std::endian::little);
static_assert(offsetof(aurora::gx::AttrConfig,attrType)==0 && offsetof(aurora::gx::AttrConfig,cnt)==1 && offsetof(aurora::gx::AttrConfig,compType)==2 && offsetof(aurora::gx::AttrConfig,offset)==3 && offsetof(aurora::gx::AttrConfig,stride)==4 && offsetof(aurora::gx::AttrConfig,frac)==5 && offsetof(aurora::gx::AttrConfig,le)==6 && offsetof(aurora::gx::AttrConfig,nbt3)==7);
static_assert(offsetof(aurora::gx::TevOp,op)==0 && offsetof(aurora::gx::TevOp,bias)==4 && offsetof(aurora::gx::TevOp,scale)==8 && offsetof(aurora::gx::TevOp,outReg)==12 && offsetof(aurora::gx::TevOp,clamp)==16 && offsetof(aurora::gx::TevOp,_p1)==17 && offsetof(aurora::gx::TevOp,_p2)==18 && offsetof(aurora::gx::TevOp,_p3)==19);
static_assert(offsetof(aurora::gx::TevStage,colorPass)==0 && offsetof(aurora::gx::TevStage,alphaPass)==16 && offsetof(aurora::gx::TevStage,colorOp)==32 && offsetof(aurora::gx::TevStage,alphaOp)==52 && offsetof(aurora::gx::TevStage,kcSel)==72 && offsetof(aurora::gx::TevStage,kaSel)==76 && offsetof(aurora::gx::TevStage,texCoordId)==80 && offsetof(aurora::gx::TevStage,texMapId)==84 && offsetof(aurora::gx::TevStage,channelId)==88 && offsetof(aurora::gx::TevStage,tevSwapRas)==92 && offsetof(aurora::gx::TevStage,tevSwapTex)==96 && offsetof(aurora::gx::TevStage,indTexStage)==100 && offsetof(aurora::gx::TevStage,indTexFormat)==104 && offsetof(aurora::gx::TevStage,indTexBiasSel)==108 && offsetof(aurora::gx::TevStage,indTexAlphaSel)==112 && offsetof(aurora::gx::TevStage,indTexMtxId)==116 && offsetof(aurora::gx::TevStage,indTexWrapS)==120 && offsetof(aurora::gx::TevStage,indTexWrapT)==124 && offsetof(aurora::gx::TevStage,indTexUseOrigLOD)==128 && offsetof(aurora::gx::TevStage,indTexAddPrev)==129 && offsetof(aurora::gx::TevStage,_p1)==130 && offsetof(aurora::gx::TevStage,_p2)==131);
static_assert(offsetof(aurora::gx::TcgConfig,type)==0 && offsetof(aurora::gx::TcgConfig,src)==4 && offsetof(aurora::gx::TcgConfig,mtx)==8 && offsetof(aurora::gx::TcgConfig,postMtx)==12 && offsetof(aurora::gx::TcgConfig,normalize)==16 && offsetof(aurora::gx::TcgConfig,embossSrc)==17 && offsetof(aurora::gx::TcgConfig,_p2)==18 && offsetof(aurora::gx::TcgConfig,_p3)==19);
static_assert(offsetof(aurora::gx::ColorChannelConfig,matSrc)==0 && offsetof(aurora::gx::ColorChannelConfig,ambSrc)==4 && offsetof(aurora::gx::ColorChannelConfig,diffFn)==8 && offsetof(aurora::gx::ColorChannelConfig,attnFn)==12 && offsetof(aurora::gx::ColorChannelConfig,lightingEnabled)==16 && offsetof(aurora::gx::ColorChannelConfig,_p1)==17 && offsetof(aurora::gx::ColorChannelConfig,_p2)==18 && offsetof(aurora::gx::ColorChannelConfig,_p3)==19);
static_assert(offsetof(aurora::gx::AlphaCompare,comp0)==0 && offsetof(aurora::gx::AlphaCompare,ref0)==4 && offsetof(aurora::gx::AlphaCompare,op)==8 && offsetof(aurora::gx::AlphaCompare,comp1)==12 && offsetof(aurora::gx::AlphaCompare,ref1)==16);
static_assert(offsetof(aurora::gx::IndStage,texCoordId)==0 && offsetof(aurora::gx::IndStage,texMapId)==4 && offsetof(aurora::gx::IndStage,scaleS)==8 && offsetof(aurora::gx::IndStage,scaleT)==12);
static_assert(offsetof(ClearRecipe,version)==0 && offsetof(ClearRecipe,clearColor)==4 && offsetof(ClearRecipe,clearAlpha)==5 && offsetof(ClearRecipe,clearDepth)==6 && offsetof(ClearRecipe,_pad)==7);
static_assert(offsetof(GXRecipe,version)==0 && offsetof(GXRecipe,msaaSamples)==4 && offsetof(GXRecipe,depthUpdate)==2769 && offsetof(GXRecipe,alphaUpdate)==2770);
inline bool recipeValid(int type,uint64_t hash,int version,const void* data,size_t size) {
    if(!recipeShape(type,version,data,size)) return false;
    // Stored config hash is an index only: recompute from the validated bytes.
    if(aurora::xxh3_hash_s(data,size,static_cast<aurora::HashType>(type))!=hash) return false;
    if(type==1) { GXRecipe config;std::memcpy(&config,data,sizeof(config));
        return aurora::gx::petari_cached_shader_valid(config.shaderConfig,hash); }
    return type==0;
}
inline bool admitRecipe(RecipeRows& rows,sqlite3_stmt* s) {
    if(!rowEnvelope(s)) return false;
    const int type=sqlite3_column_int(s,0),version=sqlite3_column_int(s,2);
    const auto hash=static_cast<uint64_t>(sqlite3_column_int64(s,1));
    const auto* data=sqlite3_column_blob(s,4); const auto size=static_cast<size_t>(sqlite3_column_bytes(s,4));
    return rows.valid(type,hash,version,data,size) && recipeValid(type,hash,version,data,size);
}
} // namespace PetariPipelineCache
