#pragma auto_inline off


#include "Game/NPC/SupportTico.hpp"
#include "Game/Boss/DinoPackun.hpp"
#include "Game/Enemy/EnemyUtilAll.hpp"
#include "Game/Scene/GameScene.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/AreaObjUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/SupportPlayUtil.hpp"

#include "Game/LiveActor/ActorPoseTQ.hpp"
#include "Game/MapObj/SpinCloudBlock.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Screen/StarPointerTarget.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Util.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/SupportPlayUtil.hpp"





#define DECL_NRV(name)                                                                                                                              \
    class name : public Nerve {                                                                                                                      \
    public:                                                                                                                                          \
        virtual void execute(Spine* pSpine) const;                                                                                                   \
        static name sInstance;                                                                                                                       \
    };                                                                                                                                               \
    name name::sInstance ATTRIBUTE_WEAK;

namespace NrvSupportTicoExecutor {
    DECL_NRV(SupportTicoExecutorNrvAppear);
    DECL_NRV(SupportTicoExecutorNrvDemoHide);
    DECL_NRV(SupportTicoExecutorNrvFollowPlayer);
    DECL_NRV(SupportTicoExecutorNrvChase);
    DECL_NRV(SupportTicoExecutorNrvBack);
};  // namespace NrvSupportTicoExecutor

SupportTicoExecutor::SupportTicoExecutor(SupportTico* pHost)
    : SupportTicoStateBase("\203T\203|\201[\203g\203`\203R\203A\203N\203e\203B\203u\217\363\221\324", pHost) {
    mStateDemoHide = nullptr;
    mStateFollowPlayer = nullptr;
    mStateChase = nullptr;
    mStateBack = nullptr;
    mIsFirstAppear = true;
    initNerve(&NrvSupportTicoExecutor::SupportTicoExecutorNrvFollowPlayer::sInstance, 4);
    mStateDemoHide = new SupportTicoStateDemoHide(pHost);
    MR::initActorState(this, mStateDemoHide, &NrvSupportTicoExecutor::SupportTicoExecutorNrvDemoHide::sInstance, "DemoHide");
    mStateFollowPlayer = new SupportTicoStateFollowPlayer(pHost);
    MR::initActorState(this, mStateFollowPlayer, &NrvSupportTicoExecutor::SupportTicoExecutorNrvFollowPlayer::sInstance, "FollowPlayer");
    mStateChase = new SupportTicoStateChase(pHost);
    MR::initActorState(this, mStateChase, &NrvSupportTicoExecutor::SupportTicoExecutorNrvChase::sInstance, "Chase");
    mStateBack = new SupportTicoStateBack(pHost);
    MR::initActorState(this, mStateBack, &NrvSupportTicoExecutor::SupportTicoExecutorNrvBack::sInstance, "Back");
}

void SupportTicoExecutor::appear() {
    mIsDead = false;
    mIsFirstAppear = true;
    if (SupportTicoStateFunction::isHideDemo()) {
        mStateDemoHide->setSilentHide(true);
        setNerve(&NrvSupportTicoExecutor::SupportTicoExecutorNrvDemoHide::sInstance);
        return;
    }
    setNerve(&NrvSupportTicoExecutor::SupportTicoExecutorNrvAppear::sInstance);
}

void SupportTicoExecutor::kill() {
    mIsDead = true;
}

void SupportTicoExecutor::control() {
    if (!isNerve(&NrvSupportTicoExecutor::SupportTicoExecutorNrvDemoHide::sInstance) && SupportTicoStateFunction::isHideDemo()) {
        mStateDemoHide->setSilentHide(false);
        setNerve(&NrvSupportTicoExecutor::SupportTicoExecutorNrvDemoHide::sInstance);
    }
}

void SupportTicoExecutor::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvSupportTicoExecutor::SupportTicoExecutorNrvFollowPlayer::sInstance)) {
        mStateFollowPlayer->attackSensor(pSender, pReceiver);
    } else if (isNerve(&NrvSupportTicoExecutor::SupportTicoExecutorNrvBack::sInstance)) {
        mStateBack->attackSensor(pSender, pReceiver);
    } else if (isNerve(&NrvSupportTicoExecutor::SupportTicoExecutorNrvChase::sInstance)) {
        mStateChase->attackSensor(pSender, pReceiver);
    }
}

bool SupportTicoExecutor::receiveMessage(u32 msg, HitSensor*, HitSensor*) {
    if (MR::isMsgTouchPlantItem(msg)) {
        return isActive();
    }
    return false;
}

void SupportTicoExecutor::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::showModel(mHost);
        MR::startAction(mHost, mIsFirstAppear ? "AppearFirst" : "Appear");
        MR::startActionSound(mHost, mIsFirstAppear ? "AppearFirst" : "AppearAgain", -1, -1, -1);
        mIsFirstAppear = false;
    }
    MR::calcVecToPlayerH(mHost->getFollowPos(), mHost, nullptr);
    SupportTicoStateFunction::calcFollowOffset(mHost);
    if (MR::isActionEnd(mHost)) {
        mHost->resetTarget();
        setNerve(&NrvSupportTicoExecutor::SupportTicoExecutorNrvFollowPlayer::sInstance);
    }
}

void SupportTicoExecutor::exeBack() {
    if (MR::updateActorState(this)) {
        if (mHost->isTargeting()) {
            setNerve(&NrvSupportTicoExecutor::SupportTicoExecutorNrvChase::sInstance);
            return;
        }
        mStateFollowPlayer->requestJump();
        setNerve(&NrvSupportTicoExecutor::SupportTicoExecutorNrvFollowPlayer::sInstance);
    }
}

bool SupportTicoExecutor::isActive() const {
    if (isNerve(&NrvSupportTicoExecutor::SupportTicoExecutorNrvAppear::sInstance) ||
        isNerve(&NrvSupportTicoExecutor::SupportTicoExecutorNrvDemoHide::sInstance)) {
        return false;
    }
    return true;
}

namespace NrvSupportTicoExecutor {
    void SupportTicoExecutorNrvBack::execute(Spine* pSpine) const {
        reinterpret_cast< SupportTicoExecutor* >(pSpine->mExecutor)->exeBack();
    }

    void SupportTicoExecutorNrvChase::execute(Spine* pSpine) const {
        MR::updateActorStateAndNextNerve(reinterpret_cast< SupportTicoExecutor* >(pSpine->mExecutor), &SupportTicoExecutorNrvBack::sInstance);
    }

    void SupportTicoExecutorNrvFollowPlayer::execute(Spine* pSpine) const {
        MR::updateActorStateAndNextNerve(reinterpret_cast< SupportTicoExecutor* >(pSpine->mExecutor), &SupportTicoExecutorNrvChase::sInstance);
    }

    void SupportTicoExecutorNrvDemoHide::execute(Spine* pSpine) const {
        MR::updateActorStateAndNextNerve(reinterpret_cast< SupportTicoExecutor* >(pSpine->mExecutor), &SupportTicoExecutorNrvAppear::sInstance);
    }

    void SupportTicoExecutorNrvAppear::execute(Spine* pSpine) const {
        reinterpret_cast< SupportTicoExecutor* >(pSpine->mExecutor)->exeAppear();
    }
};  // namespace NrvSupportTicoExecutor

