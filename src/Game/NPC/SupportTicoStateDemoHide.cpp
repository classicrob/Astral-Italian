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

namespace NrvSupportTicoStateDemoHide {
    NEW_NERVE(SupportTicoStateDemoHideNrvHide, SupportTicoStateDemoHide, Hide);
    NEW_NERVE(SupportTicoStateDemoHideNrvStop, SupportTicoStateDemoHide, Stop);
};  // namespace NrvSupportTicoStateDemoHide

SupportTicoStateDemoHide::SupportTicoStateDemoHide(SupportTico* pHost)
    : SupportTicoStateBase("\203T\203|\201[\203g\203`\203R\203f\203\202\224\361\225\134\216\246\217\363\221\324", pHost) {
    mCounter = 0;
    mSilentHide = false;
    initNerve(&NrvSupportTicoStateDemoHide::SupportTicoStateDemoHideNrvHide::sInstance, 0);
}

void SupportTicoStateDemoHide::appear() {
    mIsDead = false;
    setNerve(&NrvSupportTicoStateDemoHide::SupportTicoStateDemoHideNrvHide::sInstance);
    mCounter = 0;
    MR::startAction(mHost, "DemoHide");
    mHost->clearTarget();
    SupportTicoStateFunction::hideHoldingObj(mHost);
    MR::hideModel(mHost);
    setNerve(&NrvSupportTicoStateDemoHide::SupportTicoStateDemoHideNrvStop::sInstance);
    MR::forceDeleteEffectAll(mHost);
}

void SupportTicoStateDemoHide::kill() {
    mIsDead = true;
    SupportTicoStateFunction::showHoldingObj(mHost);
}

void SupportTicoStateDemoHide::setSilentHide(bool silent) {
    mSilentHide = silent;
}

void SupportTicoStateDemoHide::exeHide() {
    if (MR::isFirstStep(this)) {
        mHost->clearTarget();
        SupportTicoStateFunction::hideHoldingObj(mHost);
        if (mSilentHide) {
            MR::hideModel(mHost);
            setNerve(&NrvSupportTicoStateDemoHide::SupportTicoStateDemoHideNrvStop::sInstance);
            return;
        }
        MR::startAction(mHost, "DemoHide");
        MR::startActionSound(mHost, "DemoHide", -1, -1, -1);
    }
    if (MR::isActionEnd(mHost)) {
        setNerve(&NrvSupportTicoStateDemoHide::SupportTicoStateDemoHideNrvStop::sInstance);
    }
}

void SupportTicoStateDemoHide::exeStop() {
    if (MR::isFirstStep(this)) {
        MR::hideModel(mHost);
    }
    if (!SupportTicoStateFunction::isHideDemo()) {
        mCounter++;
    } else {
        mCounter = 0;
    }
    if (mCounter > 5) {
        MR::showModel(mHost);
        SupportTicoStateFunction::calcFollowOffset(mHost);
        MR::resetPosition(mHost);
        kill();
    }
}

