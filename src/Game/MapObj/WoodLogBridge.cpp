#include "Game/MapObj/WoodLogBridge.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/Util.hpp"
#include "Game/Util/Color.hpp"
#include "math_types.hpp"
#include <revolution/gx.h>
#include <revolution/types.h>
#include <cstdio>
#include <JSystem/JUtility/JUTTexture.hpp>

static const f32 sLongBridgeLength = 358.0f;
static const f32 sShortBridgeLength = 190.0f;
static const s32 sLongBridgeStep = 3;
static const s32 sShortBridgeStep = 4;
// Minimum random kick interval: a zero const the target loads from .sbss2 instead of folding it.
const volatile s32 sKickIntervalMin = 0;

// NOTE: the target symbol is __ct__18WoodLogBridgePartsFPCc in config, but the code takes (host, name): 2 args (config name is wrong)
WoodLogBridgeParts::WoodLogBridgeParts(WoodLogBridge* pHost, const char* pName) : LiveActor("\212\333\221\276\203v\203\214\201[\203g\225\224\225i") {
    mHost = pHost;
    mName = pName;
    mRestPos.x = 0.0f;
    mRestPos.y = 0.0f;
    mRestPos.z = 0.0f;
    mKickTimer = 0;
}

void WoodLogBridgeParts::init(const JMapInfoIter& rIter) {
    char name[0x40];
    snprintf(name, 0x40, "%sParts", mName);
    MR::initActor(this, rIter, name, false);
    mRestPos.set<f32>(mPosition);
    makeActorAppeared();
}

void WoodLogBridgeParts::updateRandomKick() NO_INLINE {
    if (mKickTimer > 0) {
        mKickTimer--;
    } else {
        f32 r = MR::getRandom(0.2f, 0.8f);
        TVec3f kick(mGravity);
        kick.scale(r);
        mVelocity.add(kick);
        mKickTimer = MR::getRandom(sKickIntervalMin, 30);
    }
}

// NONMATCHING: 96%; stack slot order of the kick/diff temporaries is swapped (stack offsets only)
// NONMATCHING: target negates the 2.5 pool constant at runtime (fneg) and reloads it per use; named locals get hoisted into f31
// NONMATCHING: target negates the loaded 2.5 constant at runtime (fneg) instead of a folded -2.5f pool entry; arg setup order of the temp copy
void WoodLogBridgeParts::control() {
    mVelocity.scale(0.95f);
    TVec3f kick = TVec3f(mRestPos - mPosition);
    kick.scale(0.01f);
    mVelocity.add(kick);

    s32 vol;
    s32 k = (s32)(200.0f * (__fabsf(mVelocity.y) - 2.5f)) + 100;
    if (k < 200) {
        vol = 200;
    } else {
        vol = 1000;
        if (k <= 1000) {
            vol = k;
        }
    }

    if (2.5f <= mVelocity.y) {
        MR::startActionSound(this, "Sway1", vol, -1, -1);
    }
    if (-2.5f >= mVelocity.y) {
        MR::startActionSound(this, "Sway2", vol, -1, -1);
    }
}

bool WoodLogBridgeParts::receiveMsgPlayerAttack(u32 msg, HitSensor* pA, HitSensor* pB) {
    return mHost->receiveMessage(msg, pA, pB);
}

WoodLogBridge::WoodLogBridge(const char* pName) : LiveActor(pName) {
    mParts = nullptr;
    mIsShort = false;
    mDirZ.x = 0.0f;
    mDirZ.y = 0.0f;
    mDirZ.z = 1.0f;
    mDirX.x = 1.0f;
    mDirX.y = 0.0f;
    mDirX.z = 0.0f;
    mCooldown = 0;
    mPtrE4 = &sLongBridgeLength;
    mPtrE8 = &sLongBridgeStep;
    mTex = nullptr;
    mBaseMtx.identity();
}

// NONMATCHING: 93%; the loop keeps &mGravity/&mPosition in r29/r28 and the loop guard is hoisted differently (regalloc/branch layout only)
// NONMATCHING: loop register assignment (i/part/pos/gravity pointers shuffled) and missing count>0 loop guard
// NONMATCHING: saved-register assignment in the part loop (pPart/i/pos/grav) and fmuls/fadds operand order
void WoodLogBridge::init(const JMapInfoIter& rIter) {
    const char* objName = nullptr;
    MR::getObjectName(&objName, rIter);
    if (MR::isEqualString(objName, "WoodLogBridgeShort")) {
        mIsShort = true;
        mPtrE4 = &sShortBridgeLength;
        mPtrE8 = &sShortBridgeStep;
    }

    MR::initActor(this, rIter, "WoodLogBridge", false);
    MR::makeMtxTR(mBaseMtx.toMtxPtr(), this);
    mDirZ.set<f32>(mBaseMtx.mMtx[0][2], mBaseMtx.mMtx[1][2], mBaseMtx.mMtx[2][2]);
    TVec3f upVec(mBaseMtx.mMtx[0][1], mBaseMtx.mMtx[1][1], mBaseMtx.mMtx[2][1]);
    mDirX.set<f32>(mBaseMtx.mMtx[0][0], mBaseMtx.mMtx[1][0], mBaseMtx.mMtx[2][0]);
    TVec3f down(-upVec);
    mGravity.set<f32>(down);

    s32 count = 0x20;
    MR::getJMapInfoArg0NoInit(rIter, &count);
    mParts = new LiveActorGroup("丸太パーツグループ", count);
    mParts->initWithoutIter();

    for (s32 i = 0; i < count; i++) {
        const TVec3f& rGrav = mGravity;
        const TVec3f& rPos = mPosition;
        WoodLogBridgeParts* pPart = new WoodLogBridgeParts(this, objName);

        f32 t = (f32)i / (f32)(count - 1);
        f32 s = MR::sinDegree(180.0f * t);

        TVec3f g = rPos + upVec.scaleInline(20.0f) + mDirZ.scaleInline(50.0f + 2.0f * (50.0f * (f32)i)) +
                   rGrav.scaleInline(100.0f).scaleInline(s);

        MR::setPosition(pPart, g);
        MR::setRotation(pPart, mRotation);
        pPart->initWithoutIter();
        mParts->registerActor(pPart);
    }

    s32 half = count / 2;
    f32 w = 50.0f + 2.0f * (50.0f * (f32)half);
    mClipPos.set<f32>(mPosition + mDirZ.scaleInline(w));
    MR::setClippingTypeSphere(this, w + 500.0f, &mClipPos);
    mTex = MR::loadTexFromArc(this, "WoodLogBridge.arc", "Rope.bti");
    makeActorAppeared();
}

void WoodLogBridge::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    for (s32 i = 0; i < mParts->getObjNum(); i++) {
        mParts->getActor(i)->makeActorAppeared();
    }
}

void WoodLogBridge::makeActorDead() {
    LiveActor::makeActorDead();
    for (s32 i = 0; i < mParts->getObjNum(); i++) {
        mParts->getActor(i)->makeActorDead();
    }
}

// NONMATCHING: 95%; stack slot order / reg alloc of the temporaries (fn_8032BB00 in target)
// NONMATCHING: loop-invariant FPR allocation order (f21-f31) differs; instruction stream otherwise identical
void applySway(LiveActorGroup* pGroup, const TVec3f* pVecA, const TVec3f* pVecB, f32 min, f32 max) NO_INLINE {
    f32 cNear = 100.0f;
    f32 cFar = 1500.0f;
    for (s32 i = 0; i < pGroup->getObjNum(); i++) {
        LiveActor* pActor = pGroup->getActor(i);
        TVec3f diff = *MR::getPlayerPos() - pActor->mPosition;
        MR::vecKillElement(diff, *pVecA, &diff);
        f32 sinVal = MR::sinDegree(180.0f * ((f32)i / (f32)(pGroup->getObjNum() - 1)));
        f32 amp = min + sinVal * (max - min);
        f32 mag = PSVECMag(&diff);
        f32 t = (cFar - (mag - cNear)) / (cFar - cNear);
        if (t < 0.0f) {
            t = 0.0f;
        } else if (t > 1.0f) {
            t = 1.0f;
        }
        f32 ease = MR::getEaseInValue(t, 0.0f, 1.0f, 1.0f);
        TVec3f e(*pVecB);
        e.scale(ease);
        TVec3f g(e);
        g.scale(amp);
        pActor->mVelocity.add(g);
    }
}

void WoodLogBridge::startClipped() {
    LiveActor::startClipped();
    for (s32 i = 0; i < mParts->getObjNum(); i++) {
        mParts->getActor(i)->startClipped();
    }
}

void WoodLogBridge::endClipped() {
    LiveActor::endClipped();
    for (s32 i = 0; i < mParts->getObjNum(); i++) {
        mParts->getActor(i)->endClipped();
    }
}

void WoodLogBridge::control() {
    bool onPlayer = false;
    for (s32 i = 0; i < mParts->getObjNum(); i++) {
        if (MR::isActorOnPlayer(mParts->getActor(i))) {
            onPlayer = true;
        }
    }

    if (onPlayer) {
        applySway(mParts, &mDirX, &mGravity, 0.2f, 1.0f);
    } else {
        if (mCooldown > 0) {
            mCooldown--;
        }
        for (s32 i = 0; i < mParts->getObjNum(); i++) {
            static_cast<WoodLogBridgeParts*>(mParts->getActor(i))->updateRandomKick();
        }
    }
}

bool WoodLogBridge::receiveMsgPlayerAttack(u32 msg, HitSensor* pA, HitSensor* pB) {
    if (MR::isMsgPlayerHipDropFloor(msg) && mCooldown <= 0) {
        applySway(mParts, &mDirX, &mGravity, 50.0f, 50.0f);
        mCooldown = 15;
        MR::sendMsgAwayJump(pA, pB);
        return true;
    }
    return false;
}

// fn_8032BF70: writes one GX vertex (position, color, texcoord) to the WGPIPE.
void writeLogVertex(const TVec3f* pPos, const Color8* pColor, f32 s, f32 t) NO_INLINE {
#ifdef GALAXY_NATIVE
    GXPosition3f32(pPos->x, pPos->y, pPos->z);
    GXColor1u32(*pColor);
    GXTexCoord2f32(s, t);
#else
    *(volatile f32*)0xCC008000 = pPos->x;
    *(volatile f32*)0xCC008000 = pPos->y;
    *(volatile f32*)0xCC008000 = pPos->z;
    *(volatile u32*)0xCC008000 = *pColor;
    *(volatile f32*)0xCC008000 = s;
    *(volatile f32*)0xCC008000 = t;
#endif
}

// Accept computed positions without taking the address of a temporary.
inline void writeLogVertex(const TVec3f& pos, const Color8* pColor, f32 s, f32 t) {
    writeLogVertex(&pos, pColor, s, t);
}

// fn_8032BFA0: emits one quad strip (4 vertices) of a log segment.
void drawLogQuad(const TVec3f* pA, const TVec3f* pB, const TVec3f* pC, const TVec3f* pD, const Color8* pCol1,
                        const Color8* pCol2, f32 s1, f32 s2, f32 s3, f32 s4, f32 s5) NO_INLINE {
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);

    writeLogVertex(*pA + pD->scaleInline(s1) + pC->scaleInline(s2), pCol1, 0.0f, 0.0f);
    writeLogVertex(*pA + pD->scaleInline(s3) + pC->scaleInline(s4), pCol2, 1.0f, 0.0f);
    writeLogVertex(*pB + pD->scaleInline(s1) + pC->scaleInline(s2), pCol1, 0.0f, s5);
    writeLogVertex(*pB + pD->scaleInline(s3) + pC->scaleInline(s4), pCol2, 1.0f, s5);
}

// fn_8032C230: draws one log section as three quad strips.
// NONMATCHING: target loads two separate 6.0 pool constants per call and negates them at runtime (fneg); ours folds -6.0f or hoists the constant into f31
void drawLogSection(const TVec3f* pA, const TVec3f* pB, const TVec3f* pC, const TVec3f* pD, const TVec3f* pE) NO_INLINE {
    const GXColor white = {0xFF, 0xFF, 0xFF, 0xFF};
    const GXColor blue = {0x80, 0x80, 0x80, 0xFF};

#ifdef GALAXY_NATIVE
    const Color8 logBlue(blue);
    const Color8 logWhite(white);
    drawLogQuad(pA, pB, pC, pD, &logBlue, &logWhite, 0.0f, 6.0f, 6.0f, -6.0f, 10.0f);
    drawLogQuad(pA, pB, pC, pD, &logBlue, &logWhite, -6.0f, -6.0f, 0.0f, 6.0f, 10.0f);
    drawLogQuad(pA, pB, pC, pD, &logBlue, &logWhite, 6.0f, -6.0f, -6.0f, -6.0f, 10.0f);
#else
    drawLogQuad(pA, pB, pC, pD, &Color8(blue), &Color8(white), 0.0f, 6.0f, 6.0f, -6.0f, 10.0f);
    drawLogQuad(pA, pB, pC, pD, &Color8(blue), &Color8(white), -6.0f, -6.0f, 0.0f, 6.0f, 10.0f);
    drawLogQuad(pA, pB, pC, pD, &Color8(blue), &Color8(white), 6.0f, -6.0f, -6.0f, -6.0f, 10.0f);
#endif
}

// fn_8032C750: sets up the TEV/vertex state for the bridge draw.
void setupLogDraw(const WoodLogBridge* pBridge) NO_INLINE {
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXLoadPosMtxImm(MR::getCameraViewMtx(), GX_PNMTX0);
    GXSetCurrentMtx(GX_PNMTX0);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXColor col = {0xFF, 0x00, 0x00, 0xFF};
    GXSetChanMatColor(GX_COLOR0A0, col);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, 0x3C, GX_FALSE, 0x7D);
    pBridge->mTex->load(GX_TEXMAP0);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_TEXA, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_OR, GX_GREATER, 0);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GXSetZCompLoc(GX_FALSE);
    GXSetCullMode(GX_CULL_BACK);
}

// NONMATCHING: temporary vector stack offsets differ.
void WoodLogBridge::draw() const {
    setupLogDraw(this);

    f32 h = mIsShort ? -15.0f : 50.0f;

    TVec3f p2 = mPosition + mDirZ.scaleInline(h) + TVec3f(0.0f, 1.0f, 0.0f).scaleInline(880.0f);
    TVec3f p4 = mPosition + mDirZ.scaleInline(100.0f * (f32)mParts->getObjNum() - h) + TVec3f(0.0f, 1.0f, 0.0f).scaleInline(880.0f);

    for (s32 i = 0; i < mParts->getObjNum(); i += *mPtrE8) {
        LiveActor* pActor = mParts->getActor(i);
        if (i < mParts->getObjNum() / 2) {
            const TVec3f& a1 = p2 + mDirX.scaleInline(*mPtrE4);
            const TVec3f& a3 = pActor->mPosition + mDirX.scaleInline(*mPtrE4);
            drawLogSection(&a1, &a3, &mDirZ, &mDirX, &mGravity);
            const TVec3f& a4 = mDirX.scaleInline(*mPtrE4);
            const TVec3f& a5 = mDirX.scaleInline(*mPtrE4);
            const TVec3f& a6 = pActor->mPosition - a5;
            const TVec3f& a7 = p2 - a4;
            drawLogSection(&a7, &a6, &mDirZ, &mDirX, &mGravity);
        } else {
            const TVec3f& b1 = p4 + mDirX.scaleInline(*mPtrE4);
            const TVec3f& b3 = pActor->mPosition + mDirX.scaleInline(*mPtrE4);
            drawLogSection(&b1, &b3, &mDirZ, &mDirX, &mGravity);
            const TVec3f& b4 = mDirX.scaleInline(*mPtrE4);
            const TVec3f& b5 = mDirX.scaleInline(*mPtrE4);
            const TVec3f& b6 = pActor->mPosition - b5;
            const TVec3f& b7 = p4 - b4;
            drawLogSection(&b7, &b6, &mDirZ, &mDirX, &mGravity);
        }
    }
}

MtxPtr WoodLogBridge::getBaseMtx() const {
    return const_cast< TPos3f& >(mBaseMtx).toMtxPtr();
}

WoodLogBridge::~WoodLogBridge() {
}

