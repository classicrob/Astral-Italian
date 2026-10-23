#include "gx/gx.hpp"
#include "gx/pipeline.hpp"
#include "gx/shader_info.hpp"
#include "gx/regs.hpp"
#include "gfx/resources.hpp"
#include "../gx/pipeline_postmatrix.hpp"
#include "../gx/pipeline_texgen_count.hpp"
#include "pipeline_cache_identity.hpp"
#include "../gx/pipeline_texgen_validity.hpp"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <cstdlib>
#include <exception>
#include <csignal>
#include <map>

namespace aurora::gx {
std::string petari_uber_source(DstAlphaMode, uint32_t);
const char* petari_uber_reason(const ShaderConfig&);
}
using namespace aurora::gx;

namespace {
int failures = 0;
void expect(bool value, const char* message) {
    if (!value) {
        std::fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

ShaderConfig indirect_config() {
    ShaderConfig config{};
    config.attrs[GX_VA_POS] = {.attrType = GX_DIRECT, .cnt = 3, .compType = GX_F32, .offset = 0};
    config.vtxStride = 12;
    config.tcgs[0].src = GX_TG_POS;
    config.tevStageCount = 1;
    config.numIndStages = 1;
    config.indStages[0] = {GX_TEXCOORD1, GX_TEXMAP1, GX_ITS_2, GX_ITS_4};
    auto& stage = config.tevStages[0];
    stage.texCoordId = GX_TEXCOORD0;
    stage.texMapId = GX_TEXMAP0;
    stage.colorPass.d = GX_CC_TEXC;
    stage.alphaPass.d = GX_CA_TEXA;
    stage.indTexMtxId = GX_ITM_0;
    return config;
}

void cache_policy_tests() {
    sqlite3* db = nullptr;
    expect(sqlite3_open(":memory:", &db) == SQLITE_OK, "cache fixture opens");
    expect(PetariPipelineCache::identity(db, true), "new same-build cache acquires identity");
    expect(PetariPipelineCache::identity(db), "same-build cache is reusable");
    sqlite3_exec(db, "CREATE TABLE pipeline_cache(hash INTEGER,config BLOB)", nullptr, nullptr, nullptr);
    PipelineConfig good{};
    good.shaderConfig = indirect_config();
    auto poison = good;
    poison.shaderConfig.tcgs[0].src = GX_MAX_TEXGENSRC;
    const auto unchangedPoison = poison;
    sqlite3_stmt* insert = nullptr;
    sqlite3_prepare_v2(db, "INSERT INTO pipeline_cache VALUES(?,?)", -1, &insert, nullptr);
    unsigned key = 1;
    for (const auto* config : {&good, &poison}) {
        sqlite3_bind_int(insert, 1, key++);
        sqlite3_bind_blob(insert, 2, config, sizeof(*config), SQLITE_TRANSIENT);
        expect(sqlite3_step(insert) == SQLITE_DONE, "cache fixture row written");
        sqlite3_reset(insert);
    }
    sqlite3_finalize(insert);
    sqlite3_stmt* query = nullptr;
    sqlite3_prepare_v2(db, "SELECT hash,config FROM pipeline_cache", -1, &query, nullptr);
    unsigned accepted = 0, dropped = 0;
    while (sqlite3_step(query) == SQLITE_ROW) {
        PipelineConfig config;
        std::memcpy(&config, sqlite3_column_blob(query, 1), sizeof(config));
        if (!petari_cached_shader_valid(config.shaderConfig, sqlite3_column_int64(query, 0))) { ++dropped; continue; }
        ++accepted;
        expect(!build_shader_source(config.shaderConfig, DstAlphaMode::None).empty(), "retained cache row builds shader");
    }
    sqlite3_finalize(query);
    expect(accepted == 1 && dropped == 1, "poisoned-row DB loads without abort and drops exactly the invalid row");
    expect(std::memcmp(&poison, &unchangedPoison, sizeof(poison)) == 0, "cache admission never repairs poisoned bytes");
    sqlite3_exec(db, "UPDATE petari_cache_build SET build_key='foreign-build'", nullptr, nullptr, nullptr);
    expect(!PetariPipelineCache::identity(db) && !PetariPipelineCache::identity(db, true),
           "foreign-build DB ignored without relabelling");
    expect(PetariPipelineCache::filename("pipeline_cache").find(PETARI_PIPELINE_BUILD_KEY) != std::string::npos &&
           PetariPipelineCache::filename("dawn_cache").find(PETARI_PIPELINE_BUILD_KEY) != std::string::npos,
           "SQLite pipeline and Dawn filenames share the deterministic build identity");
    sqlite3_close(db);
    auto defaultNormal = good.shaderConfig;
    defaultNormal.numIndStages = 0;
    defaultNormal.tevStages[0].indTexMtxId = GX_ITM_OFF;
    defaultNormal.tcgs[0].src = GX_TG_NRM;
    defaultNormal.attrs[GX_VA_NRM] = {};
    expect(petari_cached_shader_valid(defaultNormal, 3), "runtime default normal remains valid for cached texgen");
    expect(!build_shader_source(defaultNormal, DstAlphaMode::None).empty(), "default normal cache row generates shader");
    auto missingPosition = good.shaderConfig;
    missingPosition.attrs[GX_VA_POS] = {};
    expect(!petari_cached_shader_valid(missingPosition, 4), "missing required position rejected before runtime fatal");
    auto badAttribute = good.shaderConfig;
    badAttribute.attrs[GX_POS_MTX_ARRAY] = {.attrType = GX_DIRECT, .cnt = 1, .compType = GX_U8};
    expect(!petari_cached_shader_valid(badAttribute, 5), "non-vertex array attribute rejected before runtime fatal");
}

void texgen_count_tests() {
    // Exercise the real snapshot builder: count must come from live GX state,
    // not a sentinel inferred from a possibly invalid enabled generator.
    const auto saved = g_gxState;
    g_gxState.vtxDesc[GX_VA_POS] = GX_DIRECT;
    g_gxState.vtxFmts[0].attrs[GX_VA_POS].cnt = GX_POS_XYZ;
    g_gxState.vtxFmts[0].attrs[GX_VA_POS].type = GX_F32;
    g_gxState.vtxFmts[0].attrs[GX_VA_POS].frac = 0;
    g_gxState.numTexGens = 0;
    g_gxState.numTevStages = 2;
    g_gxState.numIndStages = 1;
    g_gxState.indStages[0] = {GX_TEXCOORD1, GX_TEXMAP1, GX_ITS_2, GX_ITS_4};
    g_gxState.tevStages[0] = indirect_config().tevStages[0];
    auto& first = g_gxState.tevStages[0];
    first.colorPass = {GX_CC_TEXC, GX_CC_TEXA, GX_CC_TEXC, GX_CC_TEXA};
    first.alphaPass = {GX_CA_TEXA, GX_CA_TEXA, GX_CA_TEXA, GX_CA_TEXA};
    first.tevSwapTex = GX_TEV_SWAP1;
    g_gxState.tevSwapTable[1] = {GX_CH_ALPHA, GX_CH_BLUE, GX_CH_RED, GX_CH_GREEN};
    g_gxState.tevStages[1] = first;
    // Disabled TEV orders also read black when numtexgens is zero in Dolphin.
    g_gxState.tevStages[1].texMapId = GX_TEXMAP_NULL;
    PipelineConfig pipeline{};
    populate_pipeline_config(pipeline, GX_TRIANGLES, GX_VTXFMT0);
    auto& config = pipeline.shaderConfig;
    for (unsigned i = 0; i < 2; ++i) {
        const auto& stage = config.tevStages[i];
        expect(stage.colorPass.a == GX_CC_ZERO && stage.colorPass.b == GX_CC_ZERO &&
               stage.colorPass.c == GX_CC_ZERO && stage.colorPass.d == GX_CC_ZERO,
               "all color TEXC/TEXA operands equal swapped zero textemp in every stage");
        expect(stage.alphaPass.a == GX_CA_ZERO && stage.alphaPass.b == GX_CA_ZERO &&
               stage.alphaPass.c == GX_CA_ZERO && stage.alphaPass.d == GX_CA_ZERO,
               "all alpha TEXA operands equal swapped zero textemp in every stage");
    }
    const auto info = build_shader_info(config);
    expect(info.sampledTexCoords.none(), "zero texgens leave inactive sentinel generators unsampled");
    expect(info.sampledIndTextures.test(1), "zero texgens preserve indirect texture sampling");
    const auto source = build_shader_source(config, DstAlphaMode::None);
    expect(source.find("var sampled0 =") == std::string::npos && source.find("var sampled1 =") == std::string::npos,
           "zero texgens skip direct texture samples rather than sample coordinate zero");
    expect(source.find("textureSampleBias(tex1, tex1_samp, vec2f(0.0)") != std::string::npos,
           "indirect lookup retains pinned Dolphin zero-coordinate behavior");
    expect(config.tcgs[0].src == GX_MAX_TEXGENSRC, "inactive sentinel remains intact");

    g_gxState.numTexGens = 1;
    g_gxState.numIndStages = 0;
    g_gxState.numTevStages = 1;
    g_gxState.tcgs[0].src = GX_TG_POS;
    g_gxState.tevStages[0] = indirect_config().tevStages[0];
    g_gxState.tevStages[0].indTexMtxId = GX_ITM_OFF;
    g_gxState.tevStages[0].texCoordId = GX_TEXCOORD7;
    populate_pipeline_config(pipeline, GX_TRIANGLES, GX_VTXFMT0);
    expect(config.tevStages[0].texCoordId == GX_TEXCOORD0 && config.tevStages[0].texMapId == GX_TEXMAP0,
           "positive texgen count maps ungenerated direct coordinate to zero without disabling texture");
    expect(build_shader_info(config).sampledTexCoords.to_ulong() == 1,
           "positive fallback analyzes only coordinate zero");
    const auto positiveSource = build_shader_source(config, DstAlphaMode::None);
    expect(positiveSource.find("var sampled0 =") != std::string::npos && positiveSource.find("tex7_uv") == std::string::npos,
           "positive fallback emits direct sampling using generated coordinate zero");
    g_gxState.numTexGens = 2;
    g_gxState.tcgs[1].src = GX_TG_POS;
    g_gxState.tevStages[0].texCoordId = GX_TEXCOORD1;
    populate_pipeline_config(pipeline, GX_TRIANGLES, GX_VTXFMT0);
    expect(config.tevStages[0] == g_gxState.tevStages[0], "valid direct stage stays byte-equivalent");
    const auto validStage = g_gxState.tevStages[0];
    g_gxState.tevStages[0].texCoordId = GX_TEXCOORD_NULL;
    g_gxState.tevStages[0].texMapId = GX_TEXMAP_NULL;
    populate_pipeline_config(pipeline, GX_TRIANGLES, GX_VTXFMT0);
    expect(config.tevStages[0] == g_gxState.tevStages[0],
           "positive count leaves a disabled NULL-coordinate stage byte-equivalent");
    expect(build_shader_info(config).sampledTexCoords.none(),
           "disabled NULL-coordinate stage does not mark coordinate zero sampled");
    g_gxState.tevStages[0] = validStage;
    g_gxState.tcgs[1].src = GX_MAX_TEXGENSRC;
    populate_pipeline_config(pipeline, GX_TRIANGLES, GX_VTXFMT0);
    expect(config.tcgs[1].src == GX_MAX_TEXGENSRC && config.tevStages[0].texCoordId == GX_TEXCOORD1,
           "genuinely invalid enabled texgen is not repaired or hidden");
    g_gxState = saved;
}

void edge_tests() {
    std::array<float, 12> matrix{1,2,3,4,5,6,7,8,9,10,11,12};
    expect(fifo::copy_xf_data(0x5f0, reinterpret_cast<const u8*>(matrix.data()), 12, std::endian::native),
           "XF load accepts matrix starting at row60 (selector124)");
    expect(petariPostMatrixRows[60].x() == 1 && petariPostMatrixRows[62].w() == 12,
           "tail rows are preserved rather than dropped");
    const float partial = 37;
    fifo::copy_xf_data(0x5f5, reinterpret_cast<const u8*>(&partial), 1, std::endian::native);
    expect(petariPostMatrixRows[61].x() == 5 && petariPostMatrixRows[61].y() == 37 && petariPostMatrixRows[61].z() == 7,
           "partial unaligned XF write preserves neighboring components");
    expect(petari_postmatrix_row(127) == 63 && petari_postmatrix_row(127, 1) == 0 && petari_postmatrix_row(127, 2) == 1,
           "selector127 wraps from row63 to rows0 and1");
    const std::array<u8, 4> beOne{0x3f, 0x80, 0, 0};
    fifo::copy_xf_data(0x500, beOne.data(), 1, std::endian::big);
    expect(petariPostMatrixRows[0].x() == 1 && g_gxState.ptTexMtxs[0].m0.x() == 1,
           "big-endian XF write updates row bank and legacy matrix view");
    const auto oldTev = g_gxState.numTevStages;
    const auto oldInd = g_gxState.numIndStages;
    const auto oldTex = g_gxState.numTexGens;
    g_gxState.numTevStages = 255;
    g_gxState.numIndStages = 255;
    g_gxState.numTexGens = 255;
    PipelineConfig built{};
    populate_pipeline_config(built, GX_TRIANGLES, static_cast<GXVtxFmt>(255));
    expect(built.shaderConfig.tevStageCount == MaxTevStages && built.shaderConfig.numIndStages == MaxIndStages,
           "config builder bounds copy counts before writing arrays");
    g_gxState.numTevStages = oldTev;
    g_gxState.numIndStages = oldInd;
    g_gxState.numTexGens = oldTex;
    auto base = indirect_config();
    for (auto& tcg : base.tcgs) tcg.src = GX_TG_POS;
    auto null = base;
    null.indStages[0].texMapId = GX_TEXMAP_NULL;
    null.indStages[0].texCoordId = GX_TEXCOORD_NULL;
    auto info = build_shader_info(null);
    expect(info.sampledIndTextures.to_ulong() == 1, "SDK NULL indirect texture selects slot zero");
    auto source = build_shader_source(null, DstAlphaMode::None);
    expect(source.find("textureSampleBias(tex0, tex0_samp") != std::string::npos,
           "NULL indirect order generates slot-zero sample");
    for (unsigned value : {0u, 63u, 64u, 121u, 124u, 125u, 126u, 127u, 255u, 0xffffffffu}) {
        auto config = base;
        config.tcgs[0].postMtx = static_cast<GXPTTexMtx>(value);
        info = build_shader_info(config);
        source = build_shader_source(config, DstAlphaMode::None);
        expect(!source.empty(), "edge post-matrix generates source");
        expect(info.usesPTTexMtx.any(), "all post-matrix selectors use the row bank");
        for (unsigned row = 0; row < 3; ++row)
            expect(source.find("ubuf.postmtx[" + std::to_string(petari_postmatrix_row(value, row)) + "]") != std::string::npos,
                   "post-matrix shader wraps each row independently");
    }
    unsigned random = 0x7f4a7c15u;
    for (unsigned iteration = 0; iteration < 512; ++iteration) {
        random ^= random << 13; random ^= random >> 17; random ^= random << 5;
        unsigned value = iteration < 256 ? iteration : random;
        auto config = base;
        auto& stage = config.tevStages[0];
        stage.texMapId = static_cast<GXTexMapID>(value);
        stage.texCoordId = static_cast<GXTexCoordID>(value);
        stage.colorOp.outReg = static_cast<GXTevRegID>(value);
        stage.alphaOp.outReg = static_cast<GXTevRegID>(value >> 1);
        stage.tevSwapTex = static_cast<GXTevSwapSel>(value);
        stage.tevSwapRas = static_cast<GXTevSwapSel>(value);
        stage.channelId = static_cast<GXChannelID>(value);
        config.indStages[0].texMapId = static_cast<GXTexMapID>(value);
        config.indStages[0].texCoordId = static_cast<GXTexCoordID>(value);
        config.tcgs[0].postMtx = static_cast<GXPTTexMtx>(value);
        info = build_shader_info(config);
        source = build_shader_source(config, DstAlphaMode::None);
        expect(info.uniformSize <= MaxUniformSize && !source.empty(), "randomized shader indices remain bounded");
        config = base;
        config.tevStages[0].indTexStage = static_cast<GXIndTexStageID>(value);
        config.numIndStages = value;
        config.tevStageCount = value;
        build_shader_info(config);
        expect(!build_shader_source(config, DstAlphaMode::None).empty(), "stage count/index edges remain bounded");
    }
    for (unsigned sourceId : {0u, 7u, 8u, 255u}) {
        auto config = base;
        config.attrs[GX_VA_NRM] = {.attrType = GX_DIRECT, .cnt = 9, .compType = GX_F32, .offset = 12};
        config.vtxStride = 48;
        config.tcgs[0].type = GX_TG_BUMP0;
        config.tcgs[0].embossSrc = sourceId;
        build_shader_info(config);
        source = build_shader_source(config, DstAlphaMode::None);
        expect(source.find("out.tex0_uv = vec2f(0.0)") != std::string::npos,
               "forward/self emboss reference has defined zero base");
    }
}

void regression_tests() {
    // Observatory draw: one texgen, but indirect order still points at coord 1.
    auto config = indirect_config();
    auto info = build_shader_info(config);
    expect(info.sampledTexCoords.to_ulong() == 1, "inactive indirect coord must select coord 0");
    expect(info.sampledTextures.test(0) && info.sampledTextures.test(1), "both texture maps remain bound");
    expect(info.usedIndStages.test(0) && info.usedIndTexMtxs.test(0), "indirect stage and matrix remain active");
    auto source = build_shader_source(config, DstAlphaMode::None);
    expect(source.find("tex0_uv * ubuf.texcoord_scale[0].xy * vec2f((1.0 / 2.0), (1.0 / 4.0)) / ubuf.tex1_size_bias.xy") != std::string::npos,
           "fallback sample uses coordinate 0 scale, indirect scales, and texture map 1 dimensions");
    expect(source.find("tex1_uv") == std::string::npos, "inactive coordinate is never emitted");

    config.tcgs[1].src = GX_TG_POS;
    info = build_shader_info(config);
    expect(info.sampledTexCoords.to_ulong() == 3, "valid indirect coord 1 remains distinct");
    source = build_shader_source(config, DstAlphaMode::None);
    expect(source.find("tex1_uv * ubuf.texcoord_scale[1].xy") != std::string::npos,
           "valid indirect coordinate keeps its own scale");

    config.tcgs[0] = {};
    config.tcgs[1] = {};
    auto& stage = config.tevStages[0];
    stage.texCoordId = GX_TEXCOORD_NULL;
    stage.texMapId = GX_TEXMAP_NULL;
    stage.colorPass.d = GX_CC_ZERO;
    stage.alphaPass.d = GX_CA_ZERO;
    info = build_shader_info(config);
    expect(info.sampledTexCoords.none(), "zero texgens do not read inactive generators");
    source = build_shader_source(config, DstAlphaMode::None);
    expect(source.find("textureSampleBias(tex1, tex1_samp, vec2f(0.0)") != std::string::npos,
           "zero-texgen indirect read follows Dolphin's zero-coordinate approximation");
}
}

int main(int argc, char** argv) {
    aurora::gfx::detail::resources().limits.minUniformBufferOffsetAlignment = 256;
    if (argc == 2 && std::string(argv[1]) == "--validate-uber-wgsl") {
        const wgpu::InstanceFeatureName timed = wgpu::InstanceFeatureName::TimedWaitAny;
        wgpu::InstanceDescriptor id{.requiredFeatureCount=1,.requiredFeatures=&timed};
        const char* unsafe = "allow_unsafe_apis";
        wgpu::DawnTogglesDescriptor toggle(wgpu::DawnTogglesDescriptor::Init{.enabledToggleCount=1,.enabledToggles=&unsafe});
        id.nextInChain=&toggle;
        auto instance=wgpu::CreateInstance(&id);
        wgpu::Adapter adapter;
        wgpu::RequestAdapterOptions ao{.backendType=wgpu::BackendType::Null};
        auto af=instance.RequestAdapter(&ao,wgpu::CallbackMode::WaitAnyOnly,[&](wgpu::RequestAdapterStatus status,wgpu::Adapter a,wgpu::StringView message){
            if(status==wgpu::RequestAdapterStatus::Success) adapter=std::move(a);
            else std::fprintf(stderr,"Null adapter: %.*s\n",int(message.length),message.data);
        });
        if(instance.WaitAny(af,5000000000)!=wgpu::WaitStatus::Success || !adapter) return 2;
        wgpu::Device device;
        wgpu::Limits limits; limits.maxImmediateSize=sizeof(DrawImmediateData);
        std::vector<wgpu::FeatureName> features;
        for(auto f:{wgpu::FeatureName::CoreFeaturesAndLimits,wgpu::FeatureName::DualSourceBlending}) if(adapter.HasFeature(f))features.push_back(f);
        wgpu::DeviceDescriptor dd({.requiredFeatureCount=features.size(),.requiredFeatures=features.data(),.requiredLimits=&limits});
        unsigned errors=0;
        dd.SetUncapturedErrorCallback([](const wgpu::Device&,wgpu::ErrorType,wgpu::StringView message,unsigned* count){++*count;std::fprintf(stderr,"WGSL: %.*s\n",int(message.length),message.data);}, &errors);
        auto df=adapter.RequestDevice(&dd,wgpu::CallbackMode::WaitAnyOnly,[&](wgpu::RequestDeviceStatus status,wgpu::Device d,wgpu::StringView message){
            if(status==wgpu::RequestDeviceStatus::Success)device=std::move(d);
            else std::fprintf(stderr,"Null device: %.*s\n",int(message.length),message.data);
        });
        if(instance.WaitAny(df,5000000000)!=wgpu::WaitStatus::Success || !device) return 3;
        unsigned checked=0;
        for(auto mode:{DstAlphaMode::None,DstAlphaMode::Replace,DstAlphaMode::DualSource}) for(auto normal:{UINT32_MAX,1u}) {
            if(mode==DstAlphaMode::DualSource && (normal!=UINT32_MAX || !device.HasFeature(wgpu::FeatureName::DualSourceBlending)))continue;
            const auto source=petari_uber_source(mode,normal);
            wgpu::ShaderSourceWGSL wgsl{};wgsl.code=source.c_str();wgpu::ShaderModuleDescriptor sd{.nextInChain=&wgsl};
            auto shader=device.CreateShaderModule(&sd);
            auto sf=shader.GetCompilationInfo(wgpu::CallbackMode::WaitAnyOnly,[&](wgpu::CompilationInfoRequestStatus status,const wgpu::CompilationInfo* info){
                if(status!=wgpu::CompilationInfoRequestStatus::Success || !info){++errors;return;}
                for(size_t i=0;i<info->messageCount;++i){const auto& m=info->messages[i];if(m.type==wgpu::CompilationMessageType::Error){++errors;std::fprintf(stderr,"WGSL %llu:%llu %.*s\n",(unsigned long long)m.lineNum,(unsigned long long)m.linePos,int(m.message.length),m.message.data);}}
            });
            if(instance.WaitAny(sf,5000000000)!=wgpu::WaitStatus::Success)++errors;
            ++checked;
        }
        std::printf("uber_wgsl variants=%u errors=%u\n",checked,errors);
        return errors?1:0;
    }
    if (argc == 3 && std::string(argv[1]) == "--dump-uber") {
        std::ofstream out(argv[2]);
        out << petari_uber_source(DstAlphaMode::None, UINT32_MAX);
        return out.good() ? 0 : 1;
    }
    if (argc == 2 && std::string(argv[1]) == "--test-uber-coverage") {
        auto c = indirect_config();
        expect(std::string(petari_uber_reason(c)) == "indirect", "indirect draws cannot enter generic path");
        c.numIndStages = 0; c.tevStages[0].indTexMtxId = GX_ITM_OFF;
        expect(std::string(petari_uber_reason(c)) == "covered", "matrix texgen with direct float position is covered");
        c.fogType = GX_FOG_PERSP_LIN;
        expect(std::string(petari_uber_reason(c)) == "fog", "fog retains specialized shader behavior");
        c.fogType = GX_FOG_NONE; c.lineMode = 1;
        expect(std::string(petari_uber_reason(c)) == "primitive", "line expansion retains specialized shader behavior");
        c.lineMode = 0; c.tcgs[0].type = GX_TG_BUMP0;
        expect(std::string(petari_uber_reason(c)) == "texgen_type", "emboss texgen cannot enter generic path");
        c.tcgs[0].type = GX_TG_MTX2x4; c.attrs[GX_VA_NRM] = {.attrType=GX_INDEX16,.cnt=9,.compType=GX_S16,.nbt3=true};
        expect(std::string(petari_uber_reason(c)) != "covered", "NBT3 decode cannot enter generic path");
        return failures ? 1 : 0;
    }
    if (argc == 3 && std::string(argv[1]) == "--audit-uber-cache") {
        sqlite3* db=nullptr;
        if(sqlite3_open_v2(argv[2],&db,SQLITE_OPEN_READONLY,nullptr)!=SQLITE_OK)return 2;
        sqlite3_stmt* q=nullptr;
        if(sqlite3_prepare_v2(db,"SELECT config FROM pipeline_cache WHERE type=1 AND config_version=13",-1,&q,nullptr)!=SQLITE_OK)return 3;
        std::map<std::string,unsigned> reasons;
        std::map<std::pair<unsigned,unsigned>,unsigned> counts;
        while(sqlite3_step(q)==SQLITE_ROW) {
            if(sqlite3_column_bytes(q,0)!=sizeof(PipelineConfig))return 4;
            PipelineConfig c;std::memcpy(&c,sqlite3_column_blob(q,0),sizeof(c));
            if(!petari_cached_shader_valid(c.shaderConfig,0)){++reasons["invalid"];continue;}
            ++reasons[petari_uber_reason(c.shaderConfig)];
            if(std::string(petari_uber_reason(c.shaderConfig))=="covered") {
                const auto info=build_shader_info(c.shaderConfig);unsigned texgens=0;
                for(unsigned i=0;i<8;++i) if(info.sampledTexCoords.test(i))texgens=i+1;
                ++counts[{c.shaderConfig.tevStageCount,texgens}];
            }
        }
        sqlite3_finalize(q);sqlite3_close(db);
        for(auto& [reason,count]:reasons)std::printf("reason=%s configs=%u\n",reason.c_str(),count);
        for(auto& [key,count]:counts)std::printf("tev=%u texgens=%u configs=%u\n",key.first,key.second,count);
        return 0;
    }
    if (argc == 3 && std::string(argv[1]) == "--validate-cache") {
        sqlite3* db = nullptr;
        if (sqlite3_open_v2(argv[2], &db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK) return 2;
        sqlite3_stmt* q = nullptr;
        if (sqlite3_prepare_v2(db, "SELECT hash,config FROM pipeline_cache WHERE type=1 AND config_version=13 AND config_size=2772", -1, &q, nullptr) != SQLITE_OK) return 3;
        unsigned accepted = 0, dropped = 0;
        while (sqlite3_step(q) == SQLITE_ROW) {
            if (sqlite3_column_bytes(q, 1) != sizeof(PipelineConfig)) return 4;
            PipelineConfig config;
            std::memcpy(&config, sqlite3_column_blob(q, 1), sizeof(config));
            if (petari_cached_shader_valid(config.shaderConfig, sqlite3_column_int64(q, 0))) {
                if (build_shader_source(config.shaderConfig, DstAlphaMode::None).empty()) return 5;
                ++accepted;
            } else ++dropped;
        }
        sqlite3_finalize(q); sqlite3_close(db);
        std::printf("cache_admission accepted=%u dropped=%u build=%s\n", accepted, dropped, PETARI_PIPELINE_BUILD_KEY);
        return 0;
    }
    if (argc == 3 && std::string(argv[1]) == "--audit-texgen-count") {
        // Input: consecutive native-layout PipelineConfig records from a private
        // RO SQLite snapshot. No assets or config bytes are printed.
        std::ifstream input(argv[2], std::ios::binary);
        if (!input) return 2;
        PipelineConfig before{};
        unsigned rows = 0;
        while (input.read(reinterpret_cast<char*>(&before), sizeof(before))) {
            if (before.version != GXPipelineConfigVersion) return 3;
            // The cached snapshot lacks the live count: validate its contiguous
            // enabled-slot representation before deriving the captured count.
            unsigned count = 0;
            bool inactive = false;
            for (const auto& tcg : before.shaderConfig.tcgs) {
                if (tcg.src == GX_MAX_TEXGENSRC) inactive = true;
                else {
                    if (inactive || unsigned(tcg.src) > GX_MAX_TEXGENSRC) return 4;
                    ++count;
                }
            }
            auto after = before;
            petari_tev_texgen_count(after.shaderConfig, count);
            const auto seed = static_cast<aurora::HashType>(aurora::gfx::ShaderType::GX);
            const auto oldHash = aurora::xxh3_hash(before, seed);
            const auto newHash = aurora::xxh3_hash(after, seed);
            std::printf("{\"row\":%u,\"derived_numTexGens\":%u,\"changed\":%s,\"before_hash\":\"%016llx\",\"after_hash\":\"%016llx\"}\n",
                        rows++, count, std::memcmp(&before, &after, sizeof(before)) == 0 ? "false" : "true",
                        static_cast<unsigned long long>(oldHash), static_cast<unsigned long long>(newHash));
        }
        return input.eof() && input.gcount() == 0 && rows ? 0 : 5;
    }
    if (argc == 1) {
        std::signal(SIGSEGV, [](int) { std::_Exit(88); });
        std::signal(SIGABRT, [](int) { std::_Exit(89); });
        std::set_terminate([] { std::fputs("Unexpected noexcept termination in shader regression\n", stderr); std::_Exit(87); });
        cache_policy_tests();
        texgen_count_tests();
        regression_tests();
        edge_tests();
        return failures ? 1 : 0;
    }
    if (argc == 2 && std::string(argv[1]) == "--probe-null-indirect") {
        std::set_terminate([] { std::fputs("NULL indirect config terminated in shader analysis\n", stderr); std::_Exit(86); });
        auto config = indirect_config();
        config.indStages[0].texMapId = GX_TEXMAP_NULL;
        config.indStages[0].texCoordId = GX_TEXCOORD_NULL;
        build_shader_info(config);
        build_shader_source(config, DstAlphaMode::None);
        return 0;
    }
    if (argc != 2) return 2;
    PipelineConfig config{};
    std::ifstream in(argv[1], std::ios::binary);
    if (!in.read(reinterpret_cast<char*>(&config), sizeof(config))) return 2;
    const auto& s = config.shaderConfig;
    const auto info = build_shader_info(s);
    for (unsigned i = 0; i < s.tcgs.size(); ++i)
        expect(!info.sampledTexCoords[i] || s.tcgs[i].src != GX_MAX_TEXGENSRC,
               "cached config samples inactive generator");
    if (!failures) {
        const auto source = build_shader_source(s, DstAlphaMode::None);
        expect(!source.empty(), "cached config generates shader source");
    }
    return failures ? 1 : 0;
}
