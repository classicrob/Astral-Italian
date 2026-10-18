#include "Game/MapObj/CaveInCube.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/CaveInParts.hpp"
#include "Game/Util.hpp"
#include "math_types.hpp"
#include <revolution/types.h>

extern "C" const char lbl_8069D0C8[];
extern "C" const s32 lbl_806491B8[];
extern "C" const char lbl_806491D0[];

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
    mRotVecs = static_cast< TVec3f* >(operator new(0x18));
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
    const char* pBase = lbl_8069D0C8;
    mDebris0 = new CaveInParts*[20];
    mDebris1 = new CaveInParts*[20];
    mDebris2 = new CaveInParts*[20];
    for (s32 i = 0; i < 20; i++) {
        mDebris0[i] = new CaveInParts(this, pBase + 0x0, pBase + 0x10);
        mDebris1[i] = new CaveInParts(this, pBase + 0x20, pBase + 0x30);
        mDebris2[i] = new CaveInParts(this, pBase + 0x44, pBase + 0x54);
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
    if (kind == 0) {
        pArr = mDebris0;
    } else if (kind == 1) {
        pArr = mDebris1;
    } else if (kind == 2) {
        pArr = mDebris2;
    }

    for (s32 i = 0; i < 20; i++) {
        if (MR::isDead(pArr[i])) {
            *ppOut = pArr[i];
            return true;
        }
    }
    return false;
}

void CaveInCube::exeGenerate() {
    if (mMaxNum != -1 && mMaxNum <= mNumGenerated) {
        return;
    }

    if (getNerveStep() % mInterval != 0) {
        return;
    }

    CaveInParts* pParts = nullptr;
    if (getUnusedDebris(lbl_806491B8[mTypeIdx], &pParts)) {
        calcDebrisMtx(pParts);
        f32 speed;
        switch (lbl_806491B8[mTypeIdx]) {
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

        mNumGenerated++;
        mTypeIdx = (mTypeIdx + 1) % 5;
    }
}
