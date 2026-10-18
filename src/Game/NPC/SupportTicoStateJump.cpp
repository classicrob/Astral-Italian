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

namespace NrvSupportTicoStateJump {
    NEW_NERVE(SupportTicoStateJumpNrvJump, SupportTicoStateJump, Jump);
    NEW_NERVE(SupportTicoStateJumpNrvReturn, SupportTicoStateJump, Return);
};  // namespace NrvSupportTicoStateJump

SupportTicoStateJump::SupportTicoStateJump(SupportTico* pHost, TVec3f* pOffset)
    : SupportTicoStateBase("\202Q\202o\203`\203R\201A\203W\203\203\203\223\203v\201I", pHost) {
    mOffset = pOffset;
    initNerve(&NrvSupportTicoStateJump::SupportTicoStateJumpNrvJump::sInstance, 0);
}

void SupportTicoStateJump::appear() {
    mIsDead = false;
    setNerve(&NrvSupportTicoStateJump::SupportTicoStateJumpNrvJump::sInstance);
}

void SupportTicoStateJump::exeJump() {
    if (MR::isFirstStep(this)) {
        mOffset->zero();
        MR::startAction(mHost, "Jump");
    }
    SupportTicoStateFunction::calcFollowOffset(mHost);
    mHost->updateTarget();
    f32 height = 150.0f * MR::calcNerveEaseInOutRate(this, 20, 0, 20);
    TVec3f up;
    MR::calcUpVec(&up, mHost);
    TVec3f vec = up * height;
    mOffset->set(vec);
    if (MR::isGreaterEqualStep(this, 39)) {
        setNerve(&NrvSupportTicoStateJump::SupportTicoStateJumpNrvReturn::sInstance);
    }
}

void SupportTicoStateJump::exeReturn() {
    SupportTicoStateFunction::calcFollowOffset(mHost);
    mHost->updateTarget();
    f32 t = MR::calcNerveRate(this, 20) / 0.5f;
    f32 height = 0.5f * (150.0f * (2.0f * -(t * (1.0f + (t * (t - 2.0f))))));
    TVec3f up;
    MR::calcUpVec(&up, mHost);
    TVec3f vec = up * height;
    mOffset->set(vec);
    if (MR::isGreaterEqualStep(this, 10)) {
        kill();
    }
}

