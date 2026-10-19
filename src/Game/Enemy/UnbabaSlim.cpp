#define COLOR8_GX_CTOR_INLINE
#include "Game/Util.hpp"
#include "Game/Enemy/UnbabaSlim.hpp"
#include "Game/MapObj/MapPartsRotator.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/VolumeDrawer.hpp"
#include "Game/Util/MapPartsUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"

namespace NrvUnbabaSlim {
    UnbabaSlimNrvWait UnbabaSlimNrvWait::sInstance ATTRIBUTE_WEAK;
    UnbabaSlimNrvEatStart UnbabaSlimNrvEatStart::sInstance ATTRIBUTE_WEAK;
    UnbabaSlimNrvEat UnbabaSlimNrvEat::sInstance ATTRIBUTE_WEAK;
    UnbabaSlimNrvEatEnd UnbabaSlimNrvEatEnd::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvUnbabaSlim

// NONMATCHING: static initialization differs in scheduling and literal-base register setup.
TVec3f sUnbabaVec[4] = {TVec3f(0.0f, 4500.0f, 600.0f), TVec3f(1.1f, 1.25f, 1.1f), TVec3f(100.0f, -150.0f, 0.0f), TVec3f(0.08f, 0.08f, 0.08f)};

void NrvUnbabaSlim::UnbabaSlimNrvWait::execute(Spine* pSpine) const {
    static_cast< UnbabaSlim* >(pSpine->mExecutor)->exeWait();
}
void NrvUnbabaSlim::UnbabaSlimNrvEatStart::execute(Spine* pSpine) const {
    static_cast< UnbabaSlim* >(pSpine->mExecutor)->exeEatStart();
}
void NrvUnbabaSlim::UnbabaSlimNrvEat::execute(Spine* pSpine) const {
    static_cast< UnbabaSlim* >(pSpine->mExecutor)->exeEat();
}
void NrvUnbabaSlim::UnbabaSlimNrvEatEnd::execute(Spine* pSpine) const {
    static_cast< UnbabaSlim* >(pSpine->mExecutor)->exeEatEnd();
}

bool UnbabaSlim::probeGroundAt(const TVec3f* pOffset) {
    TVec3f hit(0.0f);
    TPos3f mtx;
    PSMTXCopy(getBaseMtx(), mtx);
    MR::addTransMtxLocal(mtx, *pOffset);
    TVec3f pos(0.0f);
    TVec3f up(0.0f, 1.0f, 0.0f);
    MR::extractMtxTrans(mtx, &pos);
    MR::extractMtxYDir(mtx, &up);
    return MR::getFirstPolyOnLineToMap(&hit, nullptr, pos, -up * 1300.0f);
}

UnbabaSlim::UnbabaSlim(const char* pName) : LiveActor(pName) {
    mRotator = nullptr;
    mRotateSpeed = 0.0f;
    mVolumeDrawer = nullptr;
    mEatCount = 0;
    mMtx.identity();
}

void UnbabaSlim::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "UnbabaSlim", false);
    initRotator(rIter);
    mVolumeDrawer = MR::createVolumeDrawer("マグマ範囲描画", "VolumeSphere", mMtx);
    mVolumeDrawer->setHost(this);
    initNerve(&NrvUnbabaSlim::UnbabaSlimNrvWait::sInstance, 0);
    s32 rotateAxis = 0;
    MR::getMapPartsArgRotateAxis(&rotateAxis, rIter);
    if (rotateAxis == 2) {
        mRotateSpeed = 90.0f;
    }
    makeActorAppeared();
    MR::fn_80008890(this, "UnbabaSlim");
}

// NONMATCHING: same inlined TVec3 operator- pointer-register residue as Unbaba::control (regalloc only)
void UnbabaSlim::control() {
    if (!MR::isValidSwitchB(this) || MR::isOnSwitchB(this)) {
        if (mRotator != nullptr) mRotator->movement();
    }
    bool scaleUp = false;
    TVec3f diff(0.0f);
    TVec3f pos(0.0f);
    MR::extractMtxTrans(mMtx, &pos);
    diff = *MR::getPlayerPos() - pos;
    s32 distance = 3500;
    if (diff.length() < f32(distance)) scaleUp = true;
    if (scaleUp) mVolumeDrawer->setScaleUpRate(120);
    else mVolumeDrawer->setScaleDownRate(120);
    calcHeadDrawMtx();
}

void UnbabaSlim::calcAndSetBaseMtx() {
    updateRotatingBaseMtx();
}

void UnbabaSlim::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    startRotator();
    mVolumeDrawer->makeActorAppeared();
    mVolumeDrawer->setScaleDownRate(0);
}

void UnbabaSlim::makeActorDead() {
    mVolumeDrawer->makeActorDead();
    endRotator();
    LiveActor::makeActorDead();
}

void UnbabaSlim::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "UnbabaSlim");
    }
    if (probeGroundAt(&sUnbabaVec[0])) {
        setNerve(&NrvUnbabaSlim::UnbabaSlimNrvEatStart::sInstance);
    }
}

void UnbabaSlim::exeEatStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "EatStart");
    }
    if (MR::isActionEnd(this)) {
        setNerve(&NrvUnbabaSlim::UnbabaSlimNrvEat::sInstance);
    }
}

void UnbabaSlim::exeEat() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Eat");
        mEatCount = 1;
    }
    if (MR::isGreaterStep(this, 15)) {
        TVec3f sp8(sUnbabaVec[0]);
        sp8.z = -sp8.z;
        if (!probeGroundAt(&sp8)) {
            if (mEatCount == 0) {
                MR::tryHitReaction(this, "小惑星抜け");
                MR::startActionSound(this, "EatEnd", -1, -1, -1);
            }
            mEatCount++;
        } else {
            if (mEatCount > 0) {
                MR::tryHitReaction(this, "小惑星衝突");
                MR::startActionSound(this, "Eat", -1, -1, -1);
            }
            mEatCount = 0;
        }
    }
    if (mEatCount > 60) {
        setNerve(&NrvUnbabaSlim::UnbabaSlimNrvEatEnd::sInstance);
    }
}

void UnbabaSlim::exeEatEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "EatEnd");
    }
    if (MR::isActionEnd(this)) {
        setNerve(&NrvUnbabaSlim::UnbabaSlimNrvWait::sInstance);
    }
}

void UnbabaSlim::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemyAttack(pSender) && MR::isSensorPlayer(pReceiver)) {
        if (MR::sendMsgEnemyAttackFireStrong(pReceiver, pSender)) return;
    }
}

bool UnbabaSlim::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    return false;
}

// NONMATCHING: field/constant load order and multiply operand registers differ.
void UnbabaSlim::updateRotatingBaseMtx() {
    TPos3f mtx;
    mtx.identity();
    if (mRotator != nullptr) mtx.concat(mRotator->getRotateMtx(), mtx);
    MR::rotateMtxLocalY(mtx, mRotateSpeed * 0.017453292f);
    mtx.setTrans(mPosition);
    MR::setBaseTRMtx(this, mtx);
}

// NONMATCHING: sUnbabaVec base address is materialized before the getJointMtx call in the target (instruction order only)
void UnbabaSlim::calcHeadDrawMtx() {
    mMtx.setInline(MR::getJointMtx(this, "Head"));
    MR::addTransMtxLocal(mMtx, sUnbabaVec[2]);
    MR::preScaleMtx(mMtx, sUnbabaVec[1] * 5.0f);
    static const GXColor color = {0x8C, 0x28, 0x00, 0x87};
    mVolumeDrawer->setColor(color);
    mVolumeDrawer->setDrawPasses(2);
    mVolumeDrawer->setScale(sUnbabaVec[3]);
}

void UnbabaSlim::initRotator(const JMapInfoIter& rIter) {
    mRotator = new MapPartsRotator(this);
    mRotator->init(rIter);
}

void UnbabaSlim::startRotator() {
    if (mRotator != nullptr) {
        mRotator->start();
    }
}

void UnbabaSlim::endRotator() {
    if (mRotator != nullptr) {
        mRotator->end();
    }
}


UnbabaSlim::~UnbabaSlim() {}
