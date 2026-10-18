#pragma auto_inline off
#include "Game/Boss/KoopaJrRobot.hpp"
#include "Game/Util.hpp"
#include "Game/Util/FixedPosition.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include <cstdio>

extern "C" void fn_800348C0(LiveActor*);

namespace NrvKoopaJrRobotSeal {
    class KoopaJrRobotSealNrvNormal : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static KoopaJrRobotSealNrvNormal sInstance;
    };
    KoopaJrRobotSealNrvNormal KoopaJrRobotSealNrvNormal::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(KoopaJrRobotSealNrvBreak, KoopaJrRobotSeal, Break);
    class KoopaJrRobotSealNrvBroken : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static KoopaJrRobotSealNrvBroken sInstance;
    };
    KoopaJrRobotSealNrvBroken KoopaJrRobotSealNrvBroken::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvKoopaJrRobotSeal

KoopaJrRobotSeal::KoopaJrRobotSeal(const char* pName) : LiveActor(pName), mFixedPosition(nullptr), mBreakModel(nullptr), mChains(nullptr), mNumChains(0) {}

void KoopaJrRobotSeal::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "KoopaSeal", false);
    mBreakModel = MR::createBreakModel(this, getBaseMtx());
    initNerve(&NrvKoopaJrRobotSeal::KoopaJrRobotSealNrvNormal::sInstance, 0);
    makeActorDead();
}

void KoopaJrRobotSeal::makeActorAppeared() {
    if (!isNerve(&NrvKoopaJrRobotSeal::KoopaJrRobotSealNrvBroken::sInstance)) {
        LiveActor::makeActorAppeared();
    }
    for (s32 i = 0; i < mNumChains; i++) {
        mChains[i]->makeActorAppeared();
    }
}

void KoopaJrRobotSeal::makeActorDead() {
    LiveActor::makeActorDead();
    for (s32 i = 0; i < mNumChains; i++) {
        mChains[i]->makeActorDead();
    }
}

void KoopaJrRobotSeal::appear() {
    LiveActor::appear();
    setNerve(&NrvKoopaJrRobotSeal::KoopaJrRobotSealNrvNormal::sInstance);
}

void KoopaJrRobotSeal::makeChainsAppeared() {
    LiveActor::makeActorAppeared();
    for (s32 i = 0; i < mNumChains; i++) {
        mChains[i]->makeActorAppeared();
    }
}

void KoopaJrRobotSeal::calcAndSetBaseMtx() {
    if (mFixedPosition != nullptr) {
        mFixedPosition->calc();
        MR::extractMtxTrans(mFixedPosition->mMtx.toMtxPtr(), &mPosition);
        MR::setBaseTRMtx(this, mFixedPosition->mMtx);
        return;
    }
    LiveActor::calcAndSetBaseMtx();
}

void KoopaJrRobotSeal::setup(LiveActor* pHost, const char* pJoint, s32 num) {
    mFixedPosition = new FixedPosition(pHost, pJoint, this);
    mNumChains = num;
    mChains = new KoopaJrRobotChain*[num];
    for (s32 i = 0; i < mNumChains; i++) {
        mChains[i] = new KoopaJrRobotChain("\x8d\xbd\x83\x82\x83""f\x83\x8b");
        mChains[i]->initWithoutIter();
        char buf[64];
        if (num == 1) {
            snprintf(buf, 64, "%sChain", pJoint);
        } else {
            snprintf(buf, 64, "%sChain_%d", pJoint, i);
        }
        mChains[i]->setFixedPosition(this, buf);
    }
}

bool KoopaJrRobotSeal::receiveOtherMsg(u32 msg, HitSensor*, HitSensor*) {
    if (MR::isMsgAttackRockReflect(msg) && !isNerve(&NrvKoopaJrRobotSeal::KoopaJrRobotSealNrvBreak::sInstance)) {
        setNerve(&NrvKoopaJrRobotSeal::KoopaJrRobotSealNrvBreak::sInstance);
        MR::hideModel(this);
        MR::invalidateClipping(this);
        MR::invalidateCollisionParts(this);
        return true;
    }
    return false;
}

void KoopaJrRobotSeal::exeBreak() {
    if (MR::isFirstStep(this)) {
        mBreakModel->appear();
        startDisappearAll();
        MR::startActionSound(this, "Break", -1, -1, -1);
        fn_800348C0(this);
    }
    if (MR::isDead(mBreakModel) && isDisappearEndAll()) {
        setNerve(&NrvKoopaJrRobotSeal::KoopaJrRobotSealNrvBroken::sInstance);
    }
}

void KoopaJrRobotSeal::startDisappearAll() {
    for (s32 i = 0; i < mNumChains; i++) {
        mChains[i]->startDisappear();
    }
}

void KoopaJrRobotSeal::startDisappearEndAll() {
    for (s32 i = 0; i < mNumChains; i++) {
        mChains[i]->startDisappearEnd();
    }
}

bool KoopaJrRobotSeal::isDisappearEndAll() const {
    for (s32 i = 0; i < mNumChains; i++) {
        if (!mChains[i]->isDisappearEnd()) {
            return false;
        }
    }
    return true;
}

bool KoopaJrRobotSeal::isBroken() const {
    return isNerve(&NrvKoopaJrRobotSeal::KoopaJrRobotSealNrvBroken::sInstance);
}

void KoopaJrRobotSeal::breakImmediately() {
    startDisappearEndAll();
    makeActorDead();
    setNerve(&NrvKoopaJrRobotSeal::KoopaJrRobotSealNrvBroken::sInstance);
}

void KoopaJrRobotSeal::setClipping(const TVec3f* pPos, f32 radius) {
    MR::setClippingTypeSphere(this, radius, pPos);
    for (s32 i = 0; i < mNumChains; i++) {
        MR::setClippingTypeSphere(mChains[i], radius, pPos);
    }
}

KoopaJrRobotSeal::~KoopaJrRobotSeal() {}
