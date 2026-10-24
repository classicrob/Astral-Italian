#include "Game/MapObj/CaveInCube.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/CaveInParts.hpp"
#include "Game/Util.hpp"
#include "math_types.hpp"
#include <revolution/types.h>

static const s32 sDebrisType[6] = {0, 1, 1, 2, 2, 0};

namespace NrvCaveInCube {
    NEW_NERVE(CaveInCubeNrvGenerate, CaveInCube, Generate);
};  // namespace NrvCaveInCube

CaveInCube::CaveInCube(const char* pName) : LiveActor(pName) {
    mRotVecs = nullptr;
    mMaxNum = -1;
    mInterval = 20;
    mRes = 0;
    mTypeIdx = 0;
    mDebris0 = nullptr;
    mDebris1 = nullptr;
    mDebris2 = nullptr;
    mRotMtx.identity();
}

CaveInCube::~CaveInCube() {}

void CaveInCube::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::connectToSceneMapObjMovement(this);
    initRotate();
    MR::invalidateClipping(this);
    MR::getJMapInfoArg0NoInit(rIter, &mInterval);
    MR::getJMapInfoArg1NoInit(rIter, &mMaxNum);
    initDebris();
    initNerve(&NrvCaveInCube::CaveInCubeNrvGenerate::sInstance, 0);
    MR::useStageSwitchSyncAppear(this, rIter);
}

void CaveInCube::initRotate() {
    MR::makeMtxRotate(mRotMtx.mMtx, mRotation.x, mRotation.y, mRotation.z);
    mRotMtx.mMtx[0][3] = mPosition.x;
    mRotMtx.mMtx[1][3] = mPosition.y;
    mRotMtx.mMtx[2][3] = mPosition.z;
    mRotVecs = static_cast< TVec3f* >(operator new(2 * sizeof(TVec3f)));
    TVec3f v0(0.5f * (100.0f * -mScale.x), 0.5f * (100.0f * -mScale.y), 0.5f * (100.0f * -mScale.z));
    TVec3f v1(0.5f * (100.0f * mScale.x), 0.5f * (100.0f * mScale.y), 0.5f * (100.0f * mScale.z));
    TVec3f* pVecs = mRotVecs;
    pVecs->set(v0);
    (pVecs + 1)->set(v1);
}

void CaveInCube::calcDebrisMtx(CaveInParts* pParts) {
    f32 rx = MR::getRandom((s32)-mScale.x, (s32)mScale.x);
    f32 rz = MR::getRandom((s32)-mScale.z, (s32)mScale.z);
    TVec3f pos(rx * 50.0f, mScale.y * 50.0f, rz * 50.0f);
    TVec3f out(0.0f);
    mRotMtx.mult(pos, out);
    MR::setPosition(pParts, out);
}

void CaveInCube::initDebris() {
    mDebris0 = new CaveInParts*[20];
    mDebris1 = new CaveInParts*[20];
    mDebris2 = new CaveInParts*[20];
    for (s32 i = 0; i < 20; i++) {
        mDebris0[i] = new CaveInParts(this, "\227\216\224\325\203p\201[\203c[\221\345]", "CaveInPartsBig");
        mDebris1[i] = new CaveInParts(this, "\227\216\224\325\203p\201[\203c[\222\206]", "CaveInPartsMiddle");
        mDebris2[i] = new CaveInParts(this, "\227\216\224\325\203p\201[\203c[\217\254]", "CaveInPartsSmall");
    }
    for (s32 i = 0; i < 20; i++) {
        mDebris0[i]->initWithoutIter();
    }
    for (s32 i = 0; i < 20; i++) {
        mDebris1[i]->initWithoutIter();
    }
    for (s32 i = 0; i < 20; i++) {
        mDebris2[i]->initWithoutIter();
    }
}

bool CaveInCube::getUnusedDebris(u32 kind, CaveInParts** ppOut) {
    CaveInParts** pArr = nullptr;
    switch (static_cast<s32>(kind)) {
    case 0:
        pArr = mDebris0;
        break;
    case 1:
        pArr = mDebris1;
        break;
    case 2:
        pArr = mDebris2;
        break;
    }

    for (s32 i = 0; i < 20; i++) {
        if (MR::isDead(pArr[i])) {
            *ppOut = pArr[i];
            return true;
        }
    }
    return false;
}

// NONMATCHING: regalloc of the (mTypeIdx + 1) % 5 / mNumGenerated++ pair (target keeps the divisor in r5).
void CaveInCube::exeGenerate() {
    if (mMaxNum != -1 && static_cast< u32 >(mMaxNum) <= static_cast< u32 >(mNumGenerated)) {
        return;
    }

    if (getNerveStep() % mInterval != 0) {
        return;
    }

    CaveInParts* pParts = nullptr;
    if (getUnusedDebris(sDebrisType[mTypeIdx], &pParts)) {
        calcDebrisMtx(pParts);
        f32 speed;
        switch (sDebrisType[mTypeIdx]) {
        case 2:
            speed = 3.0f;
            break;
        case 1:
            speed = 2.0f;
            break;
        case 0:
            speed = 1.0f;
            break;
        default:
            speed = 0.0f;
            break;
        }
        pParts->setLifeAndSpeed(1200, 25.0f, 0.1f, speed);
        pParts->appear();

        mTypeIdx = (mTypeIdx + 1) % 5;
        mNumGenerated++;
    }
}
