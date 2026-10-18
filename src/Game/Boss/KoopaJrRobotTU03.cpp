#pragma auto_inline off
#include "Game/Boss/KoopaJrRobot.hpp"
#include "Game/Enemy/HomingKiller.hpp"
#include "Game/Util.hpp"
#include "Game/Util/FixedPosition.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include <cmath>

namespace NrvKoopaJrRobotKillerLauncher {
    class KoopaJrRobotKillerLauncherNrvHide : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static KoopaJrRobotKillerLauncherNrvHide sInstance;
    };
    KoopaJrRobotKillerLauncherNrvHide KoopaJrRobotKillerLauncherNrvHide::sInstance ATTRIBUTE_WEAK;

    class KoopaJrRobotKillerLauncherNrvShot : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static KoopaJrRobotKillerLauncherNrvShot sInstance;
    };

    class KoopaJrRobotKillerLauncherNrvWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrRobotKillerLauncher* actor = reinterpret_cast< KoopaJrRobotKillerLauncher* >(pSpine->mExecutor);
            if (actor->getKiller() != nullptr && actor->getKiller()->isMoveStart()) {
                actor->setNerve(&KoopaJrRobotKillerLauncherNrvShot::sInstance);
            }
        }
        static KoopaJrRobotKillerLauncherNrvWait sInstance;
    };
    KoopaJrRobotKillerLauncherNrvWait KoopaJrRobotKillerLauncherNrvWait::sInstance ATTRIBUTE_WEAK;

    inline void KoopaJrRobotKillerLauncherNrvShot::execute(Spine* pSpine) const {
        KoopaJrRobotKillerLauncher* actor = reinterpret_cast< KoopaJrRobotKillerLauncher* >(pSpine->mExecutor);
        actor->exeShot();
    }
    KoopaJrRobotKillerLauncherNrvShot KoopaJrRobotKillerLauncherNrvShot::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvKoopaJrRobotKillerLauncher

// ---- killer pool ----
KoopaJrRobotKillerPool::KoopaJrRobotKillerPool(LiveActor* pHost, s32 num) : mHost(pHost), mEntries(nullptr), mNum(0), mCursor(0) {
    mEntries = new Entry[num];
    mNum = num;
    for (Entry* p = mEntries; p != mEntries + mNum; p++) {
        p->mKiller = new HomingKiller("\x83z\x81[\x83~\x83\x93\x83O\x83L\x83\x89\x81[");
        p->mKiller->setMovementType(-1);
        p->mKiller->mShotTime = 20;
        p->mOwner = nullptr;
        p->mKiller->initWithoutIter();
        p->mKiller->makeActorDead();
    }
}

void KoopaJrRobotKillerPool::movementAll() {
    for (Entry* p = mEntries; p != mEntries + mNum; p++) {
        p->mKiller->movement();
    }
    releaseFinished();
}

void KoopaJrRobotKillerPool::killUnused() {
    for (Entry* p = mEntries; p != mEntries + mNum; p++) {
        if (!MR::isDead(p->mKiller) && !p->mKiller->isEaten()) {
            p->mKiller->kill();
            p->mOwner = nullptr;
        }
    }
}

// NONMATCHING: regalloc of the wrapped cursor increment (next in r3 vs r0, mNum in r0 vs r4)
HomingKiller* KoopaJrRobotKillerPool::allocate(LiveActor* pOwner) {
    Entry* p = &mEntries[mCursor];
    if (MR::isDead(p->mKiller) && !isReserved(p->mKiller)) {
        p->mOwner = pOwner;
        s32 next = mCursor + 1;
        mCursor = next;
        if (mNum <= next) {
            mCursor = 0;
        }
        return p->mKiller;
    }
    s32 next = mCursor + 1;
    mCursor = next;
    if (mNum <= next) {
        mCursor = 0;
    }
    return allocate(pOwner);
}

bool KoopaJrRobotKillerPool::release(LiveActor* pOwner, HomingKiller* pKiller) {
    if (getOwner(pKiller) != pOwner) {
        return false;
    }
    for (Entry* p = mEntries; p != mEntries + mNum; p++) {
        if (p->mOwner == pOwner && p->mKiller == pKiller && MR::isDead(p->mKiller)) {
            p->mOwner = nullptr;
            return true;
        }
    }
    return false;
}

bool KoopaJrRobotKillerPool::isShotEnd(HitSensor* pSensor) {
    for (Entry* p = mEntries; p != mEntries + mNum; p++) {
        if (!MR::isDead(p->mKiller) && p->mKiller->getSensor("Body") == pSensor) {
            return p->mKiller->isShotEnd();
        }
    }
    return false;
}

bool KoopaJrRobotKillerPool::isReserved(HomingKiller* pKiller) const {
    Entry* end = mEntries + mNum;
    for (Entry* p = mEntries; p != end; p++) {
        if (p->mKiller == pKiller) {
            return p->mOwner != nullptr;
        }
    }
    return false;
}

LiveActor* KoopaJrRobotKillerPool::getOwner(HomingKiller* pKiller) const {
    Entry* end = mEntries + mNum;
    for (Entry* p = mEntries; p != end; p++) {
        if (p->mKiller == pKiller) {
            return p->mOwner;
        }
    }
    return nullptr;
}

void KoopaJrRobotKillerPool::releaseFinished() {
    for (Entry* p = mEntries; p != mEntries + mNum; p++) {
        if ((MR::isDead(p->mKiller) || p->mKiller->isShotEnd()) && p->mOwner != nullptr) {
            p->mOwner = nullptr;
        }
    }
}

// ---- launcher ----
KoopaJrRobotKillerLauncher::KoopaJrRobotKillerLauncher(KoopaJrRobot* pHost, s32 type, FixedPosition* pFixed)
    : LiveActor("\x83\x8d\x83{\x83L\x83\x89\x81[\x83\x89\x83\x93\x83`\x83\x83\x81["), mHost(pHost), mType(type), mMagnumKiller(nullptr), mKiller(nullptr),
      mFixedPosition(pFixed) {}

void KoopaJrRobotKillerLauncher::init(const JMapInfoIter& rIter) {
    MR::invalidateClipping(this);
    initNerve(&NrvKoopaJrRobotKillerLauncher::KoopaJrRobotKillerLauncherNrvWait::sInstance, 0);
    if (mType == 1) {
        mMagnumKiller = new HomingKiller("\x83z\x81[\x83~\x83\x93\x83O\x83L\x83\x89\x81[");
        mMagnumKiller->setTypeMagnumKiller();
        mMagnumKiller->initWithoutIter();
        mMagnumKiller->mSpeed = 18.0f;
        mMagnumKiller->makeActorDead();
    }
    makeActorAppeared();
}

void KoopaJrRobotKillerLauncher::appear() {
    LiveActor::appear();
    setNerve(&NrvKoopaJrRobotKillerLauncher::KoopaJrRobotKillerLauncherNrvHide::sInstance);
}

void KoopaJrRobotKillerLauncher::kill() {
    if (mMagnumKiller != nullptr && !MR::isDead(mMagnumKiller)) {
        mMagnumKiller->kill();
    } else if (mMagnumKiller == nullptr) {
        if (mKiller != nullptr && !MR::isDead(mKiller)) {
            mKiller->kill();
        }
    }
    releaseKiller();
    LiveActor::kill();
}

bool KoopaJrRobotKillerLauncher::releaseKiller() {
    if (mKiller != nullptr) {
        return mHost->releaseKiller(this, mKiller);
    }
    return false;
}

void KoopaJrRobotKillerLauncher::startShot() {
    if (MR::isDead(this)) {
        appear();
    }
    if (mType == 1) {
        mMagnumKiller->appear();
    } else {
        mKiller = mHost->allocKiller(this);
        if (mKiller == nullptr) {
            return;
        }
        mKiller->appear();
        mKiller->mSpeed = 20.0f;
        mKiller->setChaseRotateSpeed(0.9998f);
    }
    setNerve(&NrvKoopaJrRobotKillerLauncher::KoopaJrRobotKillerLauncherNrvWait::sInstance);
}

void KoopaJrRobotKillerLauncher::updateTransform() {
    mFixedPosition->calc();
    TPos3f mtx(&mFixedPosition->mMtx);
    TVec3f euler;
    mtx.getEuler(euler);
    euler.set(euler * _180_PI);
    TVec3f pos;
    mtx.getTrans(pos);
    if (mType == 1) {
        TVec3f up;
        up.negate(mHost->mGravity);
        TVec3f front;
        mtx.getZDir(front);
        MR::vecKillElement(front, up, &front);
        MR::normalize(&front);
        MR::makeMtxFrontUpPos(&mtx, front, up, pos);
    }
    MR::setPosition(this, pos);
    mtx.getEuler(mRotation);
    mRotation.set< f32 >(_180_PI * normalizeAngle(mRotation.x), _180_PI * normalizeAngle(mRotation.y), _180_PI * normalizeAngle(mRotation.z));
    launch(mPosition, mRotation);
}

static inline f32 wrapAngle(f32 v, f32 base) {
    return base + (f32)fmod(6.2831855f + (v - base), 6.2831855f);
}

f32 KoopaJrRobotKillerLauncher::normalizeAngle(f32 angle) {
    return wrapAngle(angle, 0.0f);
}

bool KoopaJrRobotKillerLauncher::isShootable() {
    if (getKiller() != nullptr) {
        return isNerve(&NrvKoopaJrRobotKillerLauncher::KoopaJrRobotKillerLauncherNrvWait::sInstance);
    }
    return false;
}

bool KoopaJrRobotKillerLauncher::launch(const TVec3f& rPos, const TVec3f& rRot) {
    if (getKiller() == nullptr) {
        return false;
    }
    if (MR::isDead(getKiller())) {
        return false;
    }
    if (!getKiller()->isNerveAppear()) {
        return false;
    }
    if (isNerve(&NrvKoopaJrRobotKillerLauncher::KoopaJrRobotKillerLauncherNrvHide::sInstance)) {
        return false;
    }
    if (mType == 0 && mHost->getKillerOwner(mKiller) != this) {
        return false;
    }
    TVec3f axis;
    MR::getRotatedAxisZ(&axis, rRot);
    MR::setPosition(getKiller(), mPosition);
    MR::setRotation(getKiller(), mRotation);
    HomingKiller* pA = getKiller();
    HomingKiller* pB = getKiller();
    pA->mRotation.z = 0.0f;
    pB->mRotation.x = 0.0f;
    HomingKiller* pKiller = getKiller();
    pKiller->mBasePos.set(rPos);
    pKiller->mBaseFront.set(axis);
    return true;
}

HomingKiller* KoopaJrRobotKillerLauncher::getKiller() const {
    if (mType == 1) {
        return mMagnumKiller;
    }
    return mKiller;
}

void KoopaJrRobotKillerLauncher::exeShot() {
    if (MR::isFirstStep(this) && mType == 1) {
        MR::shakeCameraNormalStrong();
        MR::tryRumblePadMiddle(this, 0);
    }
}

bool KoopaJrRobotKillerLauncher::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    return MR::tryForceKillIfMsgStartPowerStarGet(this, msg);
}

KoopaJrRobotKillerLauncher::~KoopaJrRobotKillerLauncher() {}
