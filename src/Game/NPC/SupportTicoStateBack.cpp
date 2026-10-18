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

namespace NrvSupportTicoStateBack {
    DECL_NRV(SupportTicoStateBackNrvBack);
    DECL_NRV(SupportTicoStateBackNrvSpin);
};  // namespace NrvSupportTicoStateBack

SupportTicoStateBack::SupportTicoStateBack(SupportTico* pHost)
    : SupportTicoStateBase("\202Q\202o\203`\203R\201A\203v\203\214\203C\203\204\201[\202\326\226\337\202\351", pHost), mStartPos(0.0f, 0.0f, 0.0f) {
    mTimer = 0.0f;
    initNerve(&NrvSupportTicoStateBack::SupportTicoStateBackNrvBack::sInstance, 1);
    mStateSpin = new SupportTicoStateSpin(pHost);
    MR::initActorState(this, mStateSpin, &NrvSupportTicoStateBack::SupportTicoStateBackNrvSpin::sInstance, "Spin");
}

void SupportTicoStateBack::appear() {
    mIsDead = false;
    setNerve(&NrvSupportTicoStateBack::SupportTicoStateBackNrvBack::sInstance);
    MR::zeroVelocity(mHost);
    mTimer = 0.0f;
}

void SupportTicoStateBack::control() {
    TVec3f vec;
    SupportTicoStateFunction::calcFollowOffset(&vec);
    MR::vecBlend(&mHost->mPosition, mStartPos, vec, MR::getEaseInOutValue(mTimer / 30.0f, 0.0f, 1.0f, 0.0f, 1.0f));
    mTimer += 1.0f;
}

void SupportTicoStateBack::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvSupportTicoStateBack::SupportTicoStateBackNrvSpin::sInstance)) {
        mStateSpin->attackSensor(pSender, pReceiver);
    }
}

void SupportTicoStateBack::exeBack() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "Back");
        mStartPos.x = mHost->mPosition.x;
        mStartPos.y = mHost->mPosition.y;
        mStartPos.z = mHost->mPosition.z;
    }
    if (!SupportTicoStateFunction::isHolding(mHost)) {
        mHost->updateTarget();
        if (mHost->isTargeting() && MR::isGreaterStep(this, 20)) {
            kill();
        }
    }
    MR::calcVecToPlayerH(mHost->getFollowPos(), mHost, nullptr);
    if (mTimer >= 30.0f) {
        if (SupportTicoStateFunction::isHolding(mHost)) {
            mHost->resetTarget();
        }
        kill();
        return;
    }
    if (mTimer >= 15.0f && SupportTicoStateFunction::isSwing()) {
        setNerve(&NrvSupportTicoStateBack::SupportTicoStateBackNrvSpin::sInstance);
    }
}

void SupportTicoStateBack::exeSpin() {
    if (MR::updateActorState(this)) {
        if (mTimer >= 30.0f) {
            kill();
            return;
        }
        setNerve(&NrvSupportTicoStateBack::SupportTicoStateBackNrvBack::sInstance);
    }
}

namespace NrvSupportTicoStateBack {
    void SupportTicoStateBackNrvSpin::execute(Spine* pSpine) const {
        reinterpret_cast< SupportTicoStateBack* >(pSpine->mExecutor)->exeSpin();
    }

    void SupportTicoStateBackNrvBack::execute(Spine* pSpine) const {
        reinterpret_cast< SupportTicoStateBack* >(pSpine->mExecutor)->exeBack();
    }
};  // namespace NrvSupportTicoStateBack

