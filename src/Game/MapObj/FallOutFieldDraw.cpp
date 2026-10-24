#include "Game/MapObj/FallOutFieldDraw.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/AreaObj/AreaObj.hpp"
#include "Game/Map/StageSwitch.hpp"
#include "Game/Util/AreaObjUtil.hpp"
#include "Game/MapObj/ClipAreaHolder.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Screen/ImageEffectLocalUtil.hpp"
#include "Game/Screen/ScreenAlphaCapture.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/Color.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"

namespace {
    inline void drawFillScreen(GXColor color) {
        GXSetTevColor(GX_TEVREG0, color);
        ImageEffectLocalUtil::sendTextureVertex(1, 0);
    }
};  // namespace

namespace {
    static GXTevOp sAlphaTevOperater = GX_TEV_COMP_A8_GT;
    static GXBlendMode sFillBlendMode = GX_BM_BLEND;
    static GXBlendFactor sFillSrcFactor = GX_BL_SRCALPHA;
    static GXBlendFactor sFillDstFactor = GX_BL_INVSRCALPHA;
    static GXLogicOp sFillLogicOp = GX_LO_NOOP;
    // static const s32 sBlurCount = _;
    // static const f32 sBlurRadius = _;
    // static const f32 sBlurIntensity = _;
    static Color8 sFillColor = Color8(0, 0, 0, 255);

    // Per-area edge color and blur radius, indexed by the ClipDrawArea's Obj_arg1.
    struct EdgeParam {
        GXColor mColor;
        f32 mBlurRadius;
    };
    static EdgeParam sEdgeParams[6] = {
        {{0x00, 0x00, 0x00, 0x00}, 0.0f},   {{0x00, 0x2E, 0xC8, 0x80}, 0.004f}, {{0x25, 0x0E, 0x00, 0x8A}, 0.004f},
        {{0xFF, 0xF2, 0xC4, 0xE0}, 0.01f},  {{0xFF, 0x3A, 0xF5, 0xA9}, 0.0083f}, {{0xFF, 0xF6, 0xF4, 0xE0}, 0.005f},
    };
};  // namespace

// NONMATCHING: initializing the canonical GXColor emits four byte stores instead of one word store.
// Evidence: team scratch-objects6/REPORT-solA-FallOutFieldDraw.md (approved 164-byte unit loss).
FallOutFieldDraw::FallOutFieldDraw() : NameObj("クリップエリア描画[抜き]"), mEdgeIndex(0), mUseTexAlpha(false), mIsEdgeColorOverridden(false) {
    mEdgeColor.r = 0;
    mEdgeColor.g = 0;
    mEdgeColor.b = 0;
    mEdgeColor.a = 0;
    MR::createClipAreaHolder();
    MR::connectToScene(this, MR::MovementType_Environment, MR::CalcAnimType_None, MR::DrawBufferType_None, MR::DrawType_FallOutFieldDraw);
    MR::createScreenAlphaSceneObj(2, 1.0f);
    MR::createScreenAlphaSceneObj(3, 0.5f);
    MR::createScreenAlphaSceneObj(4, 0.25f);
    MR::createScreenAlphaSceneObj(5, 0.25f);
}

void FallOutFieldDraw::setUpFillScreen() const {
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_C0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevKColor(GX_KCOLOR0, Color8(0, 0, 0, !mUseTexAlpha));
    GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);

    GXTevAlphaArg alphaA = mUseTexAlpha ? GX_CA_TEXA : GX_CA_KONST;
    GXTevAlphaArg alphaB = mUseTexAlpha ? GX_CA_KONST : GX_CA_TEXA;

    GXSetTevAlphaIn(GX_TEVSTAGE0, alphaA, alphaB, GX_CA_A0, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, ::sAlphaTevOperater, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetZMode(GX_TRUE, GX_ALWAYS, GX_TRUE);
    GXSetZCompLoc(GX_FALSE);
    GXSetColorUpdate(GX_TRUE);
    GXSetAlphaUpdate(GX_FALSE);
    GXSetZScaleOffset(0.0f, 1.0f);
    GXSetBlendMode(::sFillBlendMode, ::sFillSrcFactor, ::sFillDstFactor, ::sFillLogicOp);
}

void FallOutFieldDraw::setUpEdgeAndClearAlpha() const {
    GXSetTevColor(GX_TEVREG1, mEdgeColor);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_C1, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_A1, GX_CA_TEXA, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetColorUpdate(GX_TRUE);
    GXSetAlphaUpdate(GX_TRUE);
    GXSetDstAlpha(GX_TRUE, 0);
}

void FallOutFieldDraw::init(const JMapInfoIter& rIter) {
    NameObj::init(rIter);
}

void FallOutFieldDraw::movement() {
    AreaObj* pArea;
    if (MR::isStageStateScenarioOpeningCamera()) {
        TVec3f camPos = MR::getCamPos();
        pArea = MR::getAreaObj("ClipDrawArea", camPos);
    } else {
        pArea = MR::getAreaObjAtPlayerPos("ClipDrawArea");
    }

    if (pArea != nullptr) {
        MR::activateClipArea();
        mUseTexAlpha = (pArea->mObjArg0 + 1) == 0;
        u32 idx = pArea->mObjArg1;
        if (idx > 5) {
            idx = 1;
        }
        mEdgeIndex = idx;
        if (!mIsEdgeColorOverridden) {
            GXColor color;
            GXColor tableColor = ::sEdgeParams[idx].mColor;
            color = tableColor;
            mEdgeColor = color;
        }
    } else {
        MR::deactivateClipArea();
    }
}

void FallOutFieldDraw::resetEdgeColor() {
    mIsEdgeColorOverridden = false;
    GXColor color;
    GXColor tableColor = ::sEdgeParams[mEdgeIndex].mColor;
    color = tableColor;
    mEdgeColor = color;
}

void FallOutFieldDraw::draw() const {
    if (!MR::isActiveClipArea()) {
        return;
    }

    ImageEffectLocalUtil::setupDrawTexture();

    JUTTexture* pScreenAlphaTexture2 = MR::getScreenAlphaTexture(2);
    JUTTexture* pScreenAlphaTexture3 = MR::getScreenAlphaTexture(3);
    JUTTexture* pScreenAlphaTexture4 = MR::getScreenAlphaTexture(4);
    JUTTexture* pScreenAlphaTexture5 = MR::getScreenAlphaTexture(5);

    ImageEffectLocalUtil::capture(pScreenAlphaTexture2, 0, 0, GX_CTF_A8, false, 0);
    ImageEffectLocalUtil::capture(pScreenAlphaTexture3, 0, 1, GX_CTF_A8, true, 0);

    GXSetTevKColor(GX_KCOLOR0, Color8(0, 0, 0, 1));
    GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
    GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0_A);
    GXSetTevColor(GX_TEVREG0, Color8(255, 255, 255, 255));

    GXTevColorArg colorA = mUseTexAlpha ? GX_CC_TEXA : GX_CC_KONST;
    GXTevColorArg colorB = mUseTexAlpha ? GX_CC_KONST : GX_CC_TEXA;

    GXSetTevColorIn(GX_TEVSTAGE0, colorA, colorB, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_COMP_RGB8_GT, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

    GXTevAlphaArg alphaA = mUseTexAlpha ? GX_CA_TEXA : GX_CA_KONST;
    GXTevAlphaArg alphaB = mUseTexAlpha ? GX_CA_KONST : GX_CA_TEXA;

    GXSetTevAlphaIn(GX_TEVSTAGE0, alphaA, alphaB, GX_CA_A0, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_COMP_A8_GT, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetColorUpdate(GX_FALSE);
    GXSetAlphaUpdate(GX_TRUE);
    GXSetDstAlpha(GX_FALSE, 0);
    GXSetZScaleOffset(0.0f, 0.0f);

    ImageEffectLocalUtil::drawTexture(pScreenAlphaTexture3, 2, 0, 255, ImageEffectLocalUtil::TexDrawType_0);
    ImageEffectLocalUtil::capture(pScreenAlphaTexture4, 2, 0, GX_CTF_A8, true, 0);

    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_TEXA, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_TEXA, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

    ImageEffectLocalUtil::blurTexture(pScreenAlphaTexture4, 4, 4, 5, ::sEdgeParams[mEdgeIndex].mBlurRadius, 1.0f);
    ImageEffectLocalUtil::capture(pScreenAlphaTexture5, 4, 4, GX_CTF_A8, false, 0);
    setUpEdgeAndClearAlpha();
    ImageEffectLocalUtil::drawTexture(pScreenAlphaTexture5, 1, 0, 255, ImageEffectLocalUtil::TexDrawType_2);
    MR::loadScreenAlphaTexture(2, GX_TEXMAP0);
    setUpFillScreen();

    ::drawFillScreen(::sFillColor);

    MR::loadViewMtx();
    MR::loadProjectionMtx();

    GXSetColorUpdate(GX_TRUE);
    GXSetAlphaUpdate(GX_TRUE);
    GXSetDstAlpha(GX_TRUE, 0);
    GXSetZScaleOffset(1.0f, 0.0f);
}

namespace MR {
    NameObj* createFallOutFieldDraw(const char*) {
        return MR::createSceneObj(SceneObj_FallOutFieldDraw);
    }
};  // namespace MR

namespace MR {
    void setFallOutFieldDrawEdgeColor(const GXColor& rColor) {
        if (MR::isExistSceneObj(SceneObj_FallOutFieldDraw)) {
            FallOutFieldDraw* pDraw = MR::getSceneObj< FallOutFieldDraw >(SceneObj_FallOutFieldDraw);
            GXColor color = rColor;
            pDraw->mIsEdgeColorOverridden = true;
            pDraw->mEdgeColor = color;
        }
    }

    void resetFallOutFieldDrawEdgeColor() {
        if (MR::isExistSceneObj(SceneObj_FallOutFieldDraw)) {
            MR::getSceneObj< FallOutFieldDraw >(SceneObj_FallOutFieldDraw)->resetEdgeColor();
        }
    }
};  // namespace MR
