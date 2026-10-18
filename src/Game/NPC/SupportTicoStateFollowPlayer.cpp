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

namespace NrvSupportTicoStateFollowPlayer {
    DECL_NRV(SupportTicoStateFollowPlayerNrvBackEnd);
    DECL_NRV(SupportTicoStateFollowPlayerNrvWait);
    DECL_NRV(SupportTicoStateFollowPlayerNrvJump);
    DECL_NRV(SupportTicoStateFollowPlayerNrvSpin);
    DECL_NRV(SupportTicoStateFollowPlayerNrvDummy);
    DECL_NRV(SupportTicoStateFollowPlayerNrvItemDrop);
};  // namespace NrvSupportTicoStateFollowPlayer

SupportTicoStateFollowPlayer::SupportTicoStateFollowPlayer(SupportTico* pHost)
    : SupportTicoStateBase("\203T\203|\201[\203g\203`\203R\203}\203\212\203I\222\307\220\217\217\363\221\324", pHost) {
    mStateJump = nullptr;
    mStateSpin = nullptr;
    mFollowDir.set(0.0f, 0.0f, 0.0f);
    mJumpOffset.set(0.0f, 0.0f, 0.0f);
    mRequestJump = false;
    initNerve(&NrvSupportTicoStateFollowPlayer::SupportTicoStateFollowPlayerNrvWait::sInstance, 2);
    mStateJump = new SupportTicoStateJump(pHost, &mJumpOffset);
    MR::initActorState(this, mStateJump, &NrvSupportTicoStateFollowPlayer::SupportTicoStateFollowPlayerNrvJump::sInstance, "Jump");
    mStateSpin = new SupportTicoStateSpin(pHost);
    MR::initActorState(this, mStateSpin, &NrvSupportTicoStateFollowPlayer::SupportTicoStateFollowPlayerNrvSpin::sInstance, "Spin");
}

void SupportTicoStateFollowPlayer::requestJump() {
    mRequestJump = true;
}

void SupportTicoStateFollowPlayer::appear() {
    mJumpOffset.zero();
    mIsDead = false;
    if (mRequestJump) {
        mRequestJump = false;
        setNerve(&NrvSupportTicoStateFollowPlayer::SupportTicoStateFollowPlayerNrvBackEnd::sInstance);
        return;
    }
    setNerve(&NrvSupportTicoStateFollowPlayer::SupportTicoStateFollowPlayerNrvWait::sInstance);
}

void SupportTicoStateFollowPlayer::control() {
    SupportTico* host = mHost;
    host->mPosition.set(host->mPosition + mJumpOffset);
    SupportTicoStateFunction::startSupportTicoCSSound();
}

void SupportTicoStateFollowPlayer::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvSupportTicoStateFollowPlayer::SupportTicoStateFollowPlayerNrvSpin::sInstance)) {
        mStateSpin->attackSensor(pSender, pReceiver);
    }
}

// NONMATCHING: equivalent early-exit branch layout.
void SupportTicoStateFollowPlayer::updateFollow() {
    if (MR::isFirstStep(this)) {
        mFollowDir.set(mJumpOffset);
    }
    MR::calcVecToPlayerH(mHost->getFollowPos(), mHost, nullptr);
    SupportTicoStateFunction::calcFollowOffset(mHost);
    mHost->updateTarget();
    if (mHost->isTargeting()) {
        kill();
    }
    if (!tryItemDrop() && !trySpin()) {
        if (tryJumpStart()) {
            return;
        }
        if (MR::isLessEqualStep(this, 30)) {
            f32 scale;
            if (MR::isLessStep(this, 20)) {
                scale = MR::calcNerveEaseInValue(this, 20, 1.0f, 0.0f);
            } else {
                f32 t = MR::calcNerveRate(this, 20, 30);
                scale = (10.0f * (2.0f * -(t * (1.0f + (t * (t - 2.0f)))))) / 20.0f;
            }
            TVec3f vec(mFollowDir);
            vec.x *= scale;
            vec.y *= scale;
            vec.z *= scale;
            mJumpOffset.set(vec);
        }
    }
}

void SupportTicoStateFollowPlayer::exeBackEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "BackEnd");
    }
    updateFollow();
    if (MR::isActionEnd(mHost)) {
        setNerve(&NrvSupportTicoStateFollowPlayer::SupportTicoStateFollowPlayerNrvWait::sInstance);
    }
}

void SupportTicoStateFollowPlayer::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "Wait");
    }
    updateFollow();
}

void SupportTicoStateFollowPlayer::exeJump() {
    MR::updateActorStateAndNextNerve(this, &NrvSupportTicoStateFollowPlayer::SupportTicoStateFollowPlayerNrvWait::sInstance);
    if (mHost->isTargeting()) {
        SupportTico* host = mHost;
        host->mPosition.set(host->mPosition + mJumpOffset);
        kill();
        return;
    }
    if (trySpin()) {
        return;
    }
}

// NONMATCHING: epilogue load order.
void SupportTicoStateFollowPlayer::exeSpin() {
    MR::updateActorStateAndNextNerve(this, &NrvSupportTicoStateFollowPlayer::SupportTicoStateFollowPlayerNrvWait::sInstance);
    SupportTicoStateFunction::calcFollowOffset(mHost);
    tryJumpStart();
}

void SupportTicoStateFollowPlayer::exeItemDrop() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "ItemDrop");
        mHost->releaseHoldSensor();
    }
    MR::calcVecToPlayerH(mHost->getFollowPos(), mHost, nullptr);
    SupportTicoStateFunction::calcFollowOffset(mHost);
    if (MR::isGreaterStep(this, 60)) {
        mHost->clearTarget();
        mHost->resetTarget();
        setNerve(&NrvSupportTicoStateFollowPlayer::SupportTicoStateFollowPlayerNrvWait::sInstance);
    }
}

bool SupportTicoStateFollowPlayer::tryItemDrop() {
    if (SupportTicoStateFunction::isHolding(mHost) && SupportTicoStateFunction::isReleaseA()) {
        setNerve(&NrvSupportTicoStateFollowPlayer::SupportTicoStateFollowPlayerNrvItemDrop::sInstance);
        return true;
    }
    return false;
}

bool SupportTicoStateFollowPlayer::tryJumpStart() {
    if (PSVECMag(mJumpOffset) < 50.0f && SupportTicoStateFunction::isTriggerA()) {
        setNerve(&NrvSupportTicoStateFollowPlayer::SupportTicoStateFollowPlayerNrvJump::sInstance);
        return true;
    }
    return false;
}

bool SupportTicoStateFollowPlayer::trySpin() {
    if (SupportTicoStateFunction::isSwing()) {
        setNerve(&NrvSupportTicoStateFollowPlayer::SupportTicoStateFollowPlayerNrvSpin::sInstance);
        return true;
    }
    return false;
}

namespace NrvSupportTicoStateFollowPlayer {
    void SupportTicoStateFollowPlayerNrvBackEnd::execute(Spine* pSpine) const {
        reinterpret_cast< SupportTicoStateFollowPlayer* >(pSpine->mExecutor)->exeBackEnd();
    }
    void SupportTicoStateFollowPlayerNrvWait::execute(Spine* pSpine) const {
        reinterpret_cast< SupportTicoStateFollowPlayer* >(pSpine->mExecutor)->exeWait();
    }
    void SupportTicoStateFollowPlayerNrvJump::execute(Spine* pSpine) const {
        reinterpret_cast< SupportTicoStateFollowPlayer* >(pSpine->mExecutor)->exeJump();
    }
    void SupportTicoStateFollowPlayerNrvSpin::execute(Spine* pSpine) const {
        reinterpret_cast< SupportTicoStateFollowPlayer* >(pSpine->mExecutor)->exeSpin();
    }
    void SupportTicoStateFollowPlayerNrvDummy::execute(Spine* pSpine) const {
    }
    void SupportTicoStateFollowPlayerNrvItemDrop::execute(Spine* pSpine) const {
        reinterpret_cast< SupportTicoStateFollowPlayer* >(pSpine->mExecutor)->exeItemDrop();
    }
};  // namespace NrvSupportTicoStateFollowPlayer

