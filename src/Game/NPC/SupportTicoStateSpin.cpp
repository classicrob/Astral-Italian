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

namespace NrvSupportTicoStateSpin {
    NEW_NERVE(SupportTicoStateSpinNrvSpin, SupportTicoStateSpin, Spin);
    NEW_NERVE(SupportTicoStateSpinNrvStop, SupportTicoStateSpin, Stop);
};  // namespace NrvSupportTicoStateSpin

SupportTicoStateSpin::SupportTicoStateSpin(SupportTico* pHost)
    : SupportTicoStateBase("\203T\203|\201[\203g\203`\203R\201A\203X\203s\203\223", pHost) {
    initNerve(&NrvSupportTicoStateSpin::SupportTicoStateSpinNrvSpin::sInstance, 0);
}

void SupportTicoStateSpin::appear() {
    mIsDead = false;
    MR::zeroVelocity(mHost);
    setNerve(&NrvSupportTicoStateSpin::SupportTicoStateSpinNrvSpin::sInstance);
}

void SupportTicoStateSpin::exeSpin() {
    if (MR::isFirstStep(this)) {
        MR::startCSSound2P("CS_SPIN_HIT", nullptr, 0, 1.0f);
        MR::startAction(mHost, "Spin");
        SupportTicoStateFunction::trySpinAttack(mHost);
    }
    if (MR::isGreaterStep(this, 10)) {
        setNerve(&NrvSupportTicoStateSpin::SupportTicoStateSpinNrvStop::sInstance);
    }
}

void SupportTicoStateSpin::exeStop() {
    if (MR::isGreaterStep(this, 40)) {
        mHost->resetTarget();
        kill();
    }
}

void SupportTicoStateSpin::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvSupportTicoStateSpin::SupportTicoStateSpinNrvSpin::sInstance) && MR::sendMsgSupportTicoSpin(pReceiver, pSender)) {
        MR::tryHitReactionAtPos(mHost, "\203X\203s\203\223\203q\203b\203g", &mHost->mPosition);
        MR::tryRumblePadMiddle(mHost, 1);
    }
}

