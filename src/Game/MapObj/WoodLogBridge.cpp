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
extern "C" s32 lbl_807E30D8;

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
        mKickTimer = MR::getRandom(lbl_807E30D8, 30);
    }
}

// NONMATCHING: 96%; stack slot order of the kick/diff temporaries is swapped (stack offsets only)
void WoodLogBridgeParts::control() {
    mVelocity.scale(0.95f);
    TVec3f diff = mRestPos - mPosition;
    TVec3f kick(diff);
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
    if (mVelocity.y <= -2.5f) {
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

// NONMATCHING: 89%; target negates the up vector with paired-single psq_l/ps_neg and keeps this-member addresses in regs across the loop (regalloc/stack only)
void WoodLogBridge::init(const JMapInfoIter& rIter) {
    const char* pStrs = "\212\333\221\276\203v\203\214\201[\203g\225\224\225i";
    const char* objName;
    MR::getObjectName(&objName, rIter);
    if (MR::isEqualString(objName, pStrs + 0x30)) {
        mIsShort = true;
        mPtrE4 = &sShortBridgeLength;
        mPtrE8 = &sShortBridgeStep;
    }

    MR::initActor(this, rIter, pStrs + 0x44, false);
    MR::makeMtxTR(mBaseMtx.toMtxPtr(), this);
    mDirZ.set<f32>(mBaseMtx.mMtx[0][2], mBaseMtx.mMtx[1][2], mBaseMtx.mMtx[2][2]);
    TVec3f upVec(mBaseMtx.mMtx[0][1], mBaseMtx.mMtx[1][1], mBaseMtx.mMtx[2][1]);
    mDirX.set<f32>(mBaseMtx.mMtx[0][0], mBaseMtx.mMtx[1][0], mBaseMtx.mMtx[2][0]);
    TVec3f down(-upVec.x, -upVec.y, -upVec.z);
    mGravity.set<f32>(down);

    s32 count = 0x20;
    MR::getJMapInfoArg0NoInit(rIter, &count);
    mParts = new LiveActorGroup(pStrs + 0x54, count);
    mParts->initWithoutIter();

    for (s32 i = 0; i < count; i++) {
        WoodLogBridgeParts* pPart = new WoodLogBridgeParts(this, objName);

        f32 t = (f32)i / (f32)(count - 1);
        f32 s = MR::sinDegree(180.0f * t);

        TVec3f a(mGravity);
        a.scale(100.0f);
        TVec3f b(a);
        b.scale(s);
        TVec3f c(mDirZ);
        c.scale(50.0f + 2.0f * ((f32)i * 50.0f));
        TVec3f d(upVec);
        d.scale(20.0f);
        TVec3f e(mPosition);
        e.add(d);
        TVec3f f(e);
        f.add(c);
        TVec3f g(f);
        g.add(b);

        MR::setPosition(pPart, g);
        MR::setRotation(pPart, mRotation);
        pPart->initWithoutIter();
        mParts->registerActor(pPart);
    }

    s32 half = count / 2;
    f32 w = 50.0f + 2.0f * (50.0f * (f32)half);
    TVec3f h(mDirZ);
    h.scale(w);
    TVec3f i2(mPosition);
    i2.add(h);
    mClipPos.set<f32>(i2);
    MR::setClippingTypeSphere(this, w + 500.0f, &mClipPos);
    mTex = MR::loadTexFromArc(this, pStrs + 0x68, pStrs + 0x7C);
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
void applySway(LiveActorGroup* pGroup, const TVec3f* pVecA, const TVec3f* pVecB, f32 min, f32 max) NO_INLINE {
    for (s32 i = 0; i < pGroup->getObjNum(); i++) {
        LiveActor* pActor = pGroup->getActor(i);
        TVec3f diff = *MR::getPlayerPos() - pActor->mPosition;
        MR::vecKillElement(diff, *pVecA, &diff);
        f32 sinVal = MR::sinDegree(180.0f * ((f32)i / (f32)(pGroup->getObjNum() - 1)));
        f32 amp = min + sinVal * (max - min);
        f32 mag = PSVECMag(&diff);
        f32 t = (1500.0f - (mag - 100.0f)) / 1400.0f;
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
    *(volatile f32*)0xCC008000 = pPos->x;
    *(volatile f32*)0xCC008000 = pPos->y;
    *(volatile f32*)0xCC008000 = pPos->z;
    *(volatile u32*)0xCC008000 = *pColor;
    *(volatile f32*)0xCC008000 = s;
    *(volatile f32*)0xCC008000 = t;
}

// fn_8032BFA0: emits one quad strip (4 vertices) of a log segment.
void drawLogQuad(const TVec3f* pA, const TVec3f* pB, const TVec3f* pC, const TVec3f* pD, const Color8* pCol1,
                        const Color8* pCol2, f32 s1, f32 s2, f32 s3, f32 s4, f32 s5) NO_INLINE {
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);

    TVec3f v0(*pC);
    v0.scale(s2);
    TVec3f v1(*pD);
    v1.scale(s1);
    TVec3f v2(*pA);
    v2.add(v1);
    TVec3f v3(v2);
    v3.add(v0);
    writeLogVertex(&v3, pCol1, 0.0f, 0.0f);

    TVec3f v4(*pC);
    v4.scale(s4);
    TVec3f v5(*pD);
    v5.scale(s3);
    TVec3f v6(*pA);
    v6.add(v5);
    TVec3f v7(v6);
    v7.add(v4);
    writeLogVertex(&v7, pCol2, 1.0f, 0.0f);

    TVec3f v8(*pC);
    v8.scale(s2);
    TVec3f v9(*pD);
    v9.scale(s1);
    TVec3f v10(*pB);
    v10.add(v9);
    TVec3f v11(v10);
    v11.add(v8);
    writeLogVertex(&v11, pCol1, 0.0f, s5);

    TVec3f v12(*pC);
    v12.scale(s4);
    TVec3f v13(*pD);
    v13.scale(s3);
    TVec3f v14(*pB);
    v14.add(v13);
    TVec3f v15(v14);
    v15.add(v12);
    writeLogVertex(&v15, pCol2, 1.0f, s5);
}

// fn_8032C230: draws one log section as four quad strips.
// NONMATCHING: 71%; target loads the two color words once into r29/r30 and negates the 6.0 constants (fneg) instead of using -6.0 literals
void drawLogSection(const TVec3f* pA, const TVec3f* pB, const TVec3f* pC, const TVec3f* pD, const TVec3f* pE) NO_INLINE {
    u32 white = 0xFFFFFFFF;
    u32 blue = 0x808080FF;

    Color8 cWhite0(*reinterpret_cast<const GXColor*>(&white));
    Color8 cBlue0(*reinterpret_cast<const GXColor*>(&blue));
    drawLogQuad(pA, pB, pC, pD, &cBlue0, &cWhite0, 0.0f, 6.0f, 6.0f, -6.0f, 10.0f);

    Color8 cWhite1(*reinterpret_cast<const GXColor*>(&white));
    Color8 cBlue1(*reinterpret_cast<const GXColor*>(&blue));
    drawLogQuad(pA, pB, pC, pD, &cBlue1, &cWhite1, -6.0f, -6.0f, 0.0f, 6.0f, 10.0f);

    Color8 cWhite2(*reinterpret_cast<const GXColor*>(&white));
    Color8 cBlue2(*reinterpret_cast<const GXColor*>(&blue));
    drawLogQuad(pA, pB, pC, pD, &cBlue2, &cWhite2, 6.0f, -6.0f, -6.0f, -6.0f, 10.0f);
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
    GXSetChanCtrl(GX_ALPHA0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXColor col = {0xFF, 0x00, 0x00, 0xFF};
    GXSetChanMatColor(GX_COLOR0A0, col);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, 0x3C, GX_FALSE, 0x7D);
    pBridge->mTex->load(GX_TEXMAP0);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_C0, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_TEXA, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_OR, GX_GREATER, 0);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GXSetZCompLoc(GX_FALSE);
    GXSetCullMode(GX_CULL_BACK);
}

// NONMATCHING: 98%; stack slot order of the temporaries (stack offsets only)
void WoodLogBridge::draw() const {
    setupLogDraw(this);

    f32 h = mIsShort ? -15.0f : 50.0f;

    TVec3f up0(0.0f, 1.0f, 0.0f);
    TVec3f up1(up0);
    up1.scale(880.0f);
    TVec3f dz(mDirZ);
    dz.scale(h);
    TVec3f p1(mPosition);
    p1.add(dz);
    TVec3f p2(p1);
    p2.add(up1);

    TVec3f up2(0.0f, 1.0f, 0.0f);
    TVec3f up3(up2);
    up3.scale(880.0f);

    f32 k = 100.0f * (f32)mParts->getObjNum() - h;
    TVec3f dz2(mDirZ);
    dz2.scale(k);
    TVec3f p3(mPosition);
    p3.add(dz2);
    TVec3f p4(p3);
    p4.add(up3);

    for (s32 i = 0; i < mParts->getObjNum(); i += *mPtrE8) {
        LiveActor* pActor = mParts->getActor(i);
        if (i < mParts->getObjNum() / 2) {
            f32 k_a0 = *mPtrE4;
            TVec3f a0(mDirX);
            a0.scale(k_a0);
            TVec3f a1(p2);
            a1.add(a0);
            f32 k_a2 = *mPtrE4;
            TVec3f a2(mDirX);
            a2.scale(k_a2);
            TVec3f a3(pActor->mPosition);
            a3.add(a2);
            drawLogSection(&a1, &a3, &mDirZ, &mDirX, &mGravity);

            f32 k_a4 = *mPtrE4;

            TVec3f a4(mDirX);

            a4.scale(k_a4);
            f32 k_a5 = *mPtrE4;
            TVec3f a5(mDirX);
            a5.scale(k_a5);
            TVec3f a6 = pActor->mPosition - a5;
            TVec3f a7 = p2 - a4;
            drawLogSection(&a7, &a6, &mDirZ, &mDirX, &mGravity);
        } else {
            f32 k_b0 = *mPtrE4;
            TVec3f b0(mDirX);
            b0.scale(k_b0);
            TVec3f b1(p4);
            b1.add(b0);
            f32 k_b2 = *mPtrE4;
            TVec3f b2(mDirX);
            b2.scale(k_b2);
            TVec3f b3(pActor->mPosition);
            b3.add(b2);
            drawLogSection(&b1, &b3, &mDirZ, &mDirX, &mGravity);

            f32 k_b4 = *mPtrE4;

            TVec3f b4(mDirX);

            b4.scale(k_b4);
            f32 k_b5 = *mPtrE4;
            TVec3f b5(mDirX);
            b5.scale(k_b5);
            TVec3f b6 = pActor->mPosition - b5;
            TVec3f b7 = p4 - b4;
            drawLogSection(&b7, &b6, &mDirZ, &mDirX, &mGravity);
        }
    }
}

MtxPtr WoodLogBridge::getBaseMtx() const {
    return const_cast< TPos3f& >(mBaseMtx).toMtxPtr();
}

WoodLogBridge::~WoodLogBridge() {
}
