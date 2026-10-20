#include "gx/gx.hpp"
#include "gx/pipeline.hpp"
#include "gx/shader_info.hpp"
#include "gx/regs.hpp"
#include "gfx/resources.hpp"
#include "../gx/pipeline_postmatrix.hpp"
#include "../gx/pipeline_texgen_count.hpp"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <cstdlib>
#include <exception>
#include <csignal>

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
