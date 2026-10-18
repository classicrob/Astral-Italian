#include "Game/Enemy/HomingKiller.hpp"
#include "Game/Util.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/GravityUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"

namespace NrvHomingKillerLauncher {
    NEW_NERVE(HomingKillerLauncherNrvAppearKiller, HomingKillerLauncher, AppearKiller);
    NEW_NERVE(HomingKillerLauncherNrvDeadKiller, HomingKillerLauncher, DeadKiller);
};  // namespace NrvHomingKillerLauncher

HomingKillerLauncher::HomingKillerLauncher(const char* pName) : LiveActor("ホーミングキラー発生源"), mKillers(nullptr), mKillerNum(1), mAppearInterval(180) {
}

void HomingKillerLauncher::init(const JMapInfoIter& rIter) {
    MR::connectToSceneEnemyMovement(this);
    MR::initDefaultPos(this, rIter);
    MR::invalidateClipping(this);
    initNerve(GET_NERVE(HomingKillerLauncher, HomingKillerLauncherNrvAppearKiller), 0);
    MR::syncStageSwitchAppearAndSync(this, rIter);

    MR::getJMapInfoArg4NoInit(rIter, &mKillerNum);
    if (mKillerNum <= 0) {
        mKillerNum = 1;
    }

    MR::getJMapInfoArg5NoInit(rIter, &mAppearInterval);
    if (mAppearInterval < 180) {
        mAppearInterval = 180;
    }

    mKillers = new HomingKiller[mKillerNum];

    for (s32 i = 0; i < mKillerNum; i++) {
        mKillers[i].init(rIter);
    }

    makeActorDead();
}

void HomingKillerLauncher::appear() {
    LiveActor::appear();
    setNerve(GET_NERVE(HomingKillerLauncher, HomingKillerLauncherNrvAppearKiller));
}

bool HomingKillerLauncher::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    return MR::tryForceKillIfMsgStartPowerStarGet(this, msg);
}

HomingKiller* HomingKillerLauncher::fn_80198540() {
    for (s32 i = 0; i < this->mKillerNum; i++) {
        HomingKiller* pKiller = &this->mKillers[i];
        if (MR::isDead(pKiller)) {
            return pKiller;
        }
    }

    return nullptr;
}

void HomingKillerLauncher::exeAppearKiller() {
    if (MR::isFirstStep(this) && this->fn_80198540()) {
        this->fn_80198540()->appear();
    }

    if (mKillerNum == 1) {
        if (this->fn_80198540()) {
            setNerve(GET_NERVE(HomingKillerLauncher, HomingKillerLauncherNrvDeadKiller));
        }
    } else {
        setNerve(GET_NERVE(HomingKillerLauncher, HomingKillerLauncherNrvDeadKiller));
    }
}

void HomingKillerLauncher::exeDeadKiller() {
    if (MR::isGreaterStep(this, mAppearInterval) && this->fn_80198540()) {
        setNerve(GET_NERVE(HomingKillerLauncher, HomingKillerLauncherNrvAppearKiller));
    }
}
