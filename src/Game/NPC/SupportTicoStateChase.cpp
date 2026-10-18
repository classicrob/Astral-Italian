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

namespace NrvSupportTicoStateChase {
    DECL_NRV(SupportTicoStateChaseNrvChase);
    DECL_NRV(SupportTicoStateChaseNrvSpin);
    DECL_NRV(SupportTicoStateChaseNrvItemGet);
};  // namespace NrvSupportTicoStateChase

SupportTicoStateChase::SupportTicoStateChase(SupportTico* pHost)
    : SupportTicoStateBase("\202Q\202o\203`\203R\201A\222\307\202\242\202\251\202\257\201I", pHost), mStateSpin(nullptr), mStartPos(0.0f, 0.0f, 0.0f) {
    initNerve(&NrvSupportTicoStateChase::SupportTicoStateChaseNrvChase::sInstance, 1);
    mStateSpin = new SupportTicoStateSpin(pHost);
    MR::initActorState(this, mStateSpin, &NrvSupportTicoStateChase::SupportTicoStateChaseNrvSpin::sInstance, "Spin");
}

void SupportTicoStateChase::appear() {
    mIsDead = false;
    setNerve(&NrvSupportTicoStateChase::SupportTicoStateChaseNrvChase::sInstance);
}

void SupportTicoStateChase::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvSupportTicoStateChase::SupportTicoStateChaseNrvChase::sInstance)) {
        if (mHost->tryHold(pSender, pReceiver)) {
            MR::startActionSound(mHost, "ItemGet", -1, -1, -1);
            setNerve(&NrvSupportTicoStateChase::SupportTicoStateChaseNrvItemGet::sInstance);
        }
    } else if (isNerve(&NrvSupportTicoStateChase::SupportTicoStateChaseNrvSpin::sInstance)) {
        mStateSpin->attackSensor(pSender, pReceiver);
    }
}

void SupportTicoStateChase::exeChase() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "Chase");
        mStartPos.x = mHost->mPosition.x;
        mStartPos.y = mHost->mPosition.y;
        mStartPos.z = mHost->mPosition.z;
        MR::startDPDHitSound();
    }
    if (MR::isStep(this, 3)) {
        MR::startActionSound(mHost, "ChaseStart", -1, -1, -1);
    }
    mHost->updateTarget();
    if (!mHost->isTargeting()) {
        kill();
        return;
    }
    TVec3f targetPos;
    mHost->calcTargetPos(&targetPos);
    MR::calcVecToTargetPosH(mHost->getFollowPos(), mHost, targetPos, nullptr);
    TVec3f* position = &mHost->mPosition;
    MR::vecBlend(position, mStartPos, targetPos, MR::calcNerveEaseInRate(this, 12));
    if (MR::isStep(this, 12)) {
        if (MR::hasME()) {
            MR::startSystemME("ME_DPD_HIT");
        } else {
            MR::startSystemSE("SE_SY_DPD_HIT", -1, -1);
        }
        MR::startCSSound2P("CS_DPD_HIT", nullptr, 0, 1.0f);
    }
    if (MR::isGreaterEqualStep(this, 12) && SupportTicoStateFunction::isSwing()) {
        setNerve(&NrvSupportTicoStateChase::SupportTicoStateChaseNrvSpin::sInstance);
    }
}

namespace NrvSupportTicoStateChase {
    void SupportTicoStateChaseNrvChase::execute(Spine* pSpine) const {
        reinterpret_cast< SupportTicoStateChase* >(pSpine->mExecutor)->exeChase();
    }

    void SupportTicoStateChaseNrvSpin::execute(Spine* pSpine) const {
        SupportTicoStateChase* pState = reinterpret_cast< SupportTicoStateChase* >(pSpine->mExecutor);
        if (MR::updateActorState(pState)) {
            pState->kill();
        }
    }

    void SupportTicoStateChaseNrvItemGet::execute(Spine* pSpine) const {
        reinterpret_cast< SupportTicoStateChase* >(pSpine->mExecutor)->kill();
    }
};  // namespace NrvSupportTicoStateChase

