
#include "Game/Enemy/BegomanLauncher.hpp"
#include "Game/Enemy/BegomanBaby.hpp"
#include "Game/Enemy/BegomanSpike.hpp"
#include "Game/Enemy/BegomanSpring.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/GravityUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StringUtil.hpp"

namespace NrvBegomanLauncher {
    NEW_NERVE(HostTypeNrvWaitAfterSwitchOn, BegomanLauncher, WaitAfterSwitchOn);
    NEW_NERVE(HostTypeNrvWait, BegomanLauncher, Wait);
    NEW_NERVE(HostTypeNrvWaitForLaunch, BegomanLauncher, WaitForLaunch);
    NEW_NERVE(HostTypeNrvLaunch, BegomanLauncher, Launch);
    NEW_NERVE(HostTypeNrvAppear, BegomanLauncher, Appear);
    NEW_NERVE(HostTypeNrvDisappear, BegomanLauncher, Disappear);
};  // namespace NrvBegomanLauncher

namespace {
    const s32 sLaunchTime = 0;
};  // namespace

BegomanLauncher::BegomanLauncher(const char* pName) : LiveActor(pName), mBegomanCount(), mAppearDelay(), mBegomanArray(), mLaunchType(3) {
}

void BegomanLauncher::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    s32 count = 1;
    MR::getJMapInfoArg3NoInit(rIter, &count);
    mBegomanCount = count;
    MR::getJMapInfoArg2NoInit(rIter, &mAppearDelay);
    const char* pObjectName = nullptr;
    MR::getObjectName(&pObjectName, rIter);
    mBegomanArray = new BegomanBase*[mBegomanCount];
    if (MR::isEqualString(pObjectName, "TogeBegomanLauncher")) {
        setName("トゲベーゴマンランチャー");
        for (s32 i = 0; i < mBegomanCount; i++) {
            mBegomanArray[i] = new BegomanSpike("トゲベーゴマン");
            MR::setPosition(mBegomanArray[i], mPosition);
            mBegomanArray[i]->init(rIter);
            mBegomanArray[i]->makeActorDead();
        }
    } else if (MR::isEqualString(pObjectName, "BegomanBabyLauncher")) {
        setName("ベビーベーゴマンランチャー");
        mLaunchType = 2;
        MR::declareCoin(this, mBegomanCount);
        for (s32 i = 0; i < mBegomanCount; i++) {
            mBegomanArray[i] = new BegomanBaby(this, "ランチャーベビー");
            MR::setPosition(mBegomanArray[i], mPosition);
            mBegomanArray[i]->init(rIter);
            mBegomanArray[i]->makeActorDead();
        }
    } else {
        for (s32 i = 0; i < mBegomanCount; i++) {
            mBegomanArray[i] = new BegomanSpring("バネベーゴマン");
            MR::setPosition(mBegomanArray[i], mPosition);
            mBegomanArray[i]->init(rIter);
            mBegomanArray[i]->makeActorDead();
        }
    }

    MR::initActor(this, rIter, "BegomanLauncher", false);
    MR::connectToSceneEnemy(this);
    initNerve(GET_NERVE(BegomanLauncher, HostTypeNrvWait));
    initEffectKeeper(0, "BegomanLauncher", false);
    initSound(8, "BegomanLauncher", nullptr, TVec3f(0.0f));
    MR::initShadowVolumeSphere(this, 100.0f);
    if (MR::initUseStageSwitchReadAppear(this, rIter)) {
        MR::listenStageSwitchOnAppear(this, MR::Functor(this, &BegomanLauncher::onSwitchAppear));
        makeActorDead();
    } else {
        appear();
    }

    if (MR::initUseStageSwitchReadB(this, rIter)) {
        MR::listenStageSwitchOnB(this, MR::Functor(this, &BegomanLauncher::kill));
    }
}

void BegomanLauncher::initAfterPlacement() {
    MR::calcGravity(this);
}

void BegomanLauncher::kill() {
    LiveActor::kill();
    for (s32 i = 0; i < mBegomanCount; i++) {
        if (!MR::isDead(mBegomanArray[i])) {
            mBegomanArray[i]->kill();
        }
    }

    MR::deleteEffect(this, "Launcher");
}

void BegomanLauncher::onSwitchAppear() {
    if (MR::isDead(this)) {
        setNerve(GET_NERVE(BegomanLauncher, HostTypeNrvWaitAfterSwitchOn));
        appear();
    }
}

void BegomanLauncher::exeWaitAfterSwitchOn() {
    if (MR::isFirstStep(this)) {
        for (s32 i = 0; i < mBegomanCount; i++) {
            mBegomanArray[i]->makeActorDead();
        }
        return;
    }

    if (MR::isGreaterStep(this, mAppearDelay)) {
        setNerve(GET_NERVE(BegomanLauncher, HostTypeNrvLaunch));
    }
}

void BegomanLauncher::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::hideModel(this);
    }

    if (mLaunchType == 2) {
        for (s32 i = 0; i < mBegomanCount; i++) {
            if (!MR::isDead(mBegomanArray[i])) {
                return;
            }
        }

        setNerve(GET_NERVE(BegomanLauncher, HostTypeNrvWaitForLaunch));
    } else {
        for (s32 i = 0; i < mBegomanCount; i++) {
            if (MR::isDead(mBegomanArray[i])) {
                setNerve(GET_NERVE(BegomanLauncher, HostTypeNrvWaitForLaunch));
                return;
            }
        }
    }
}

void BegomanLauncher::exeWaitForLaunch() {
    if (MR::isGreaterStep(this, 180)) {
        setNerve(GET_NERVE(BegomanLauncher, HostTypeNrvAppear));
    }
}

// NONMATCHING: compiler reuses r31 for the child index instead of recomputing it into r0.
void BegomanLauncher::exeLaunch() {
    if (MR::isFirstStep(this)) {
        MR::showModel(this);
        MR::emitEffect(this, "Launcher");
        MR::startActionSound(this, "EmBegomanlaunLaunch");
    }

    if (MR::isGreaterStep(this, ::sLaunchTime)) {
        if (mLaunchType == 2) {
            BegomanBase::launchBegomanBabyLauncher(this, reinterpret_cast< BegomanBaby** >(mBegomanArray), mBegomanCount, 100.0f, 10.0f, 30.0f,
                                                   nullptr);
            setNerve(GET_NERVE(BegomanLauncher, HostTypeNrvDisappear));
        } else {
            for (s32 i = 0; i < mBegomanCount; i++) {
                if (MR::isDead(mBegomanArray[i])) {
                    MR::calcGravity(this);
                    MR::setPosition(mBegomanArray[i], mPosition);
                    TVec3f& velocity = mBegomanArray[i]->mVelocity;
                    TVec3f launchVelocity = mGravity * -30.0f;
                    velocity.x = launchVelocity.x;
                    velocity.y = launchVelocity.y;
                    velocity.z = launchVelocity.z;
                    mBegomanArray[i]->appear();
                    setNerve(GET_NERVE(BegomanLauncher, HostTypeNrvDisappear));
                    return;
                }
            }
        }
    }
}

void BegomanLauncher::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::showModel(this);
        MR::emitEffect(this, "Launcher");
        MR::startAction(this, "BegomanLauncher");
    }

    MR::startActionSound(this, "EmLvBegomanlaunAppear");
    if (MR::isGreaterStep(this, 90)) {
        MR::stopBck(this);
        setNerve(GET_NERVE(BegomanLauncher, HostTypeNrvLaunch));
    }
}

void BegomanLauncher::exeDisappear() {
    if (MR::isFirstStep(this)) {
        MR::showModel(this);
        MR::startAction(this, "BegomanLauncher");
        MR::setBckFrame(this, 90.0f);
        MR::deleteEffect(this, "Launcher");
    }

    if (MR::isGreaterStep(this, 90)) {
        setNerve(GET_NERVE(BegomanLauncher, HostTypeNrvWait));
    }
}

bool BegomanLauncher::fn_801631C0(const LiveActor* actor) const {
    for (s32 i = 0; i < mBegomanCount; ++i) {
        if (mBegomanArray[i] == actor) {
            return true;
        }
    }
    return false;
}

LiveActor* BegomanLauncher::fn_80163200(s32 index) const {
    return mBegomanArray[index];
}
