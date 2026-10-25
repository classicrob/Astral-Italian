#pragma auto_inline off
#include "Game/Boss/KoopaJrRobot.hpp"
#include <algorithm>
#include "Game/MapObj/StarPieceGroup.hpp"
#include "Game/MapObj/StarPiece.hpp"
#include "Game/Enemy/HomingKiller.hpp"
#include "Game/Util.hpp"
#include "Game/Util/FixedPosition.hpp"
#include "Game/LiveActor/Nerve.hpp"
#ifdef GALAXY_NATIVE
#include <petari/game_compat.hpp>
#else
#include <functional.hpp>
#endif

template < typename It, typename Fn >
static inline void forEachCallInline(It it, It end, Fn fn) {
    It last = end;
    for (It first = it; first != last; ++first) {
        fn(*first);
    }
}

template < typename It, typename Fn >
static Fn forEachFn(It it, It end, Fn fn) {
    for (; it != end; ++it) {
        fn(*it);
    }
    return fn;
}

namespace NrvKoopaJrRobotSeq {
    class KoopaJrRobotSeqNrvA : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            reinterpret_cast< KoopaJrRobotSeq* >(pSpine->mExecutor)->exeA();
        }
        static KoopaJrRobotSeqNrvA sInstance;
    };
    class KoopaJrRobotSeqNrvB : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            reinterpret_cast< KoopaJrRobotSeq* >(pSpine->mExecutor)->exeA();
        }
        static KoopaJrRobotSeqNrvB sInstance;
    };
    class KoopaJrRobotSeqNrvC : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            reinterpret_cast< KoopaJrRobotSeq* >(pSpine->mExecutor)->exeA();
        }
        static KoopaJrRobotSeqNrvC sInstance;
    };
    class KoopaJrRobotSeqNrvD : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            reinterpret_cast< KoopaJrRobotSeq* >(pSpine->mExecutor)->exeB();
        }
        static KoopaJrRobotSeqNrvD sInstance;
    };
    class KoopaJrRobotSeqNrvE : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            reinterpret_cast< KoopaJrRobotSeq* >(pSpine->mExecutor)->exeC();
        }
        static KoopaJrRobotSeqNrvE sInstance;
    };
    class KoopaJrRobotSeqNrvF : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrRobotSeq* pSeq = reinterpret_cast< KoopaJrRobotSeq* >(pSpine->mExecutor);
            if (MR::isFirstStep(pSeq)) {
                pSeq->startRearLauncherShots();
                pSeq->setNerve(&KoopaJrRobotSeqNrvC::sInstance);
            }
        }
        static KoopaJrRobotSeqNrvF sInstance;
    };
    class KoopaJrRobotSeqNrvG : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static KoopaJrRobotSeqNrvG sInstance;
    };
    KoopaJrRobotSeqNrvA KoopaJrRobotSeqNrvA::sInstance ATTRIBUTE_WEAK;
    KoopaJrRobotSeqNrvB KoopaJrRobotSeqNrvB::sInstance ATTRIBUTE_WEAK;
    KoopaJrRobotSeqNrvC KoopaJrRobotSeqNrvC::sInstance ATTRIBUTE_WEAK;
    KoopaJrRobotSeqNrvD KoopaJrRobotSeqNrvD::sInstance ATTRIBUTE_WEAK;
    KoopaJrRobotSeqNrvE KoopaJrRobotSeqNrvE::sInstance ATTRIBUTE_WEAK;
    KoopaJrRobotSeqNrvF KoopaJrRobotSeqNrvF::sInstance ATTRIBUTE_WEAK;
    KoopaJrRobotSeqNrvG KoopaJrRobotSeqNrvG::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvKoopaJrRobotSeq

KoopaJrRobotSeq::KoopaJrRobotSeq(KoopaJrRobot* pHost) : NerveExecutor("\x83L\x83\x89\x81[\x90\xa7\x8c\xe4"), mHost(pHost), mPool(nullptr), mMagnumLauncher(nullptr), mLaunchersDisabled(false), mLauncherSide(false) {}

// NONMATCHING: regalloc only (begin pointer r26 vs p r23 permuted); string-pool offsets differ
void KoopaJrRobotSeq::initLaunchers() {
    mPool = new KoopaJrRobotKillerPool(mHost, 48);
    mMagnumLauncher = new KoopaJrRobotKillerLauncher(mHost, 1, new FixedPosition(mHost, "MagnumKillerPoint", TVec3f(0.0f), TVec3f(0.0f, 90.0f, 0.0f)));
    mMagnumLauncher->initWithoutIter();
    mMagnumLauncher->makeActorDead();
    KoopaJrRobotKillerLauncher** pBegin = mLaunchers;
    for (KoopaJrRobotKillerLauncher** p = pBegin; p != mLaunchers + 8; p++) {
        const char* pName = nullptr;
        switch (p - pBegin) {
        case 0:
            pName = "KillerPointB";
            break;
        case 1:
            pName = "KillerPointA";
            break;
        case 2:
            pName = "KillerPointC";
            break;
        case 3:
            pName = "KillerPointD";
            break;
        case 4:
            pName = "KillerPointE";
            break;
        case 5:
            pName = "KillerPointF";
            break;
        case 6:
            pName = "KillerPointG";
            break;
        case 7:
            pName = "KillerPointH";
            break;
        }
        *p = new KoopaJrRobotKillerLauncher(mHost, 0, new FixedPosition(mHost, pName, TVec3f(0.0f), TVec3f(0.0f, 90.0f, 0.0f)));
        (*p)->initWithoutIter();
        (*p)->makeActorDead();
    }
    initNerve(&NrvKoopaJrRobotSeq::KoopaJrRobotSeqNrvB::sInstance, 0);
}

void KoopaJrRobotSeq::appearLaunchers() {
    mLaunchersDisabled = false;
    mMagnumLauncher->appear();
    std::for_each(mLaunchers, mLaunchers + 8, std::mem_fun(&KoopaJrRobotKillerLauncher::appear));
}

void KoopaJrRobotSeq::killLaunchers() {
    mLaunchersDisabled = true;
    mMagnumLauncher->kill();
    mPool->killUnused();
    std::for_each(mLaunchers, mLaunchers + 8, std::mem_fun(&KoopaJrRobotKillerLauncher::kill));
}

void KoopaJrRobotSeq::updateLauncherTransforms() {
    if (!mLaunchersDisabled) {
        mMagnumLauncher->updateTransform();
        std::for_each(mLaunchers, mLaunchers + 8, std::mem_fun(&KoopaJrRobotKillerLauncher::updateTransform));
    }
}

void KoopaJrRobotSeq::movementLaunchers() {
    if (!mLaunchersDisabled) {
        forEachCallInline(mLaunchers, mLaunchers + 8, std::mem_fun(&LiveActor::movement));
    }
}

void KoopaJrRobotSeq::movementKillerPool() {
    if (!mLaunchersDisabled) {
        mPool->movementAll();
    }
}

namespace KoopaJrRobotSeqData {
    // Original unused member-function pointer at 0x80675D18.
    void (KoopaJrRobotKillerLauncher::*sStartShot)() = &KoopaJrRobotKillerLauncher::startShot;
}

void KoopaJrRobotSeq::startMagnumShot() {
    mMagnumLauncher->startShot();
}

void KoopaJrRobotSeq::startRearLauncherShots() {
    for (s32 i = 2; i < 8; i++) {
        mLaunchers[i]->startShot();
    }
}

// NONMATCHING: regalloc only (loop vars r28..r31 permuted)
void KoopaJrRobotSeq::attackShootableKillers() {
    HitSensor* pSensor;
    KoopaJrRobotKillerLauncher** p;
    if (!MR::isDead(mMagnumLauncher) && mMagnumLauncher->isShootable()) {
        pSensor = mHost->getSensor("Body");
        MR::sendMsgEnemyAttackExplosion(mMagnumLauncher->getKiller()->getSensor("Body"), pSensor);
    }
    for (p = mLaunchers; p != mLaunchers + 8; p++) {
        if (!MR::isDead(*p) && (*p)->isShootable()) {
            pSensor = mHost->getSensor("Body");
            MR::sendMsgEnemyAttackExplosion((*p)->getKiller()->getSensor("Body"), pSensor);
        }
    }
}

void KoopaJrRobotSeq::startFastFrontLauncherShots() {
    setNerve(&NrvKoopaJrRobotSeq::KoopaJrRobotSeqNrvD::sInstance);
}

void KoopaJrRobotSeq::startRearLauncherSequence() {
    setNerve(&NrvKoopaJrRobotSeq::KoopaJrRobotSeqNrvF::sInstance);
}

void KoopaJrRobotSeq::startSlowFrontLauncherShots() {
    setNerve(&NrvKoopaJrRobotSeq::KoopaJrRobotSeqNrvE::sInstance);
}

void KoopaJrRobotSeq::stopLauncherSequence() {
    setNerve(&NrvKoopaJrRobotSeq::KoopaJrRobotSeqNrvG::sInstance);
}

void KoopaJrRobotSeq::exeA() {
    if (isNerve(&NrvKoopaJrRobotSeq::KoopaJrRobotSeqNrvA::sInstance)) {
        if (MR::isGreaterStep(this, 360)) {
            setNerve(&NrvKoopaJrRobotSeq::KoopaJrRobotSeqNrvD::sInstance);
        }
    } else if (isNerve(&NrvKoopaJrRobotSeq::KoopaJrRobotSeqNrvB::sInstance)) {
        if (MR::isGreaterStep(this, 360)) {
            setNerve(&NrvKoopaJrRobotSeq::KoopaJrRobotSeqNrvE::sInstance);
        }
    } else if (isNerve(&NrvKoopaJrRobotSeq::KoopaJrRobotSeqNrvC::sInstance) && MR::isGreaterStep(this, 300)) {
        setNerve(&NrvKoopaJrRobotSeq::KoopaJrRobotSeqNrvF::sInstance);
    }
}

void KoopaJrRobotSeq::exeB() {
    if (MR::isFirstStep(this)) {
        mLaunchers[!mLauncherSide]->startShot();
    }
    if (MR::isStep(this, 70)) {
        mLaunchers[mLauncherSide != 0]->startShot();
        mLauncherSide = !mLauncherSide;
        setNerve(&NrvKoopaJrRobotSeq::KoopaJrRobotSeqNrvB::sInstance);
    }
}

void KoopaJrRobotSeq::exeC() {
    if (MR::isFirstStep(this)) {
        mLaunchers[!mLauncherSide]->startShot();
    }
    if (MR::isStep(this, 120)) {
        mLaunchers[mLauncherSide != 0]->startShot();
        mLauncherSide = !mLauncherSide;
        setNerve(&NrvKoopaJrRobotSeq::KoopaJrRobotSeqNrvB::sInstance);
    }
}

KoopaJrRobotSeq::~KoopaJrRobotSeq() {}
