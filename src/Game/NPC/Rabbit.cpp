#include "Game/NPC/Rabbit.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/NPC/NPCActor.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JMapInfo.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/NPCUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/ParabolicPath.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/RailUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/Util/TalkUtil.hpp"
#include <JSystem/JGeometry/TVec.hpp>
#include <revolution/types.h>

namespace {
    static f32 cAppearHeight = 150.0f;
    static f32 cAppearHop = 10.0f;
}

namespace NrvRabbit {
    NEW_NERVE(RabbitNrvAppear, Rabbit, Appear);
    NEW_NERVE(RabbitNrvAppearLand, Rabbit, AppearLand);
    NEW_NERVE(RabbitNrvReaction, Rabbit, Reaction);
    NEW_NERVE(RabbitNrvTalk, Rabbit, Talk);
    NEW_NERVE(RabbitNrvJumpV, Rabbit, JumpV);
    NEW_NERVE(RabbitNrvJumpH, Rabbit, JumpH);
}

Rabbit::Rabbit(const char* pName) : NPCActor(pName) {
}

Rabbit::~Rabbit() {
}

void Rabbit::makeActorAppeared() {
    NPCActor::makeActorAppeared();
    MR::emitEffect(this, "Light");
}

bool Rabbit::isReactionNerve() const {
    return isNerve(GET_NERVE(Rabbit, RabbitNrvTalk));
}

// NONMATCHING: string pointer register allocation.
void Rabbit::init(const JMapInfoIter& rIter) {
    const char* name;
    MR::getObjectName(&name, rIter);
    NPCActorCaps caps("MoonRabbit");
    caps.setDefault();
    caps.mMessageId = name;
    caps.mMessageOffset = TVec3f(0.0f, 160.0f, 0.0f);
    initialize(rIter, caps, "MoonRabbit", nullptr, nullptr);
    s32 colorFrameArg = 0;
    MR::getJMapInfoArg0NoInit(rIter, &colorFrameArg);
    MR::startBtk(this, "MoonRabbit");
    MR::setBtkFrameAndStop(this, colorFrameArg);
    s32 behaviorArg = 0;
    MR::getJMapInfoArg1NoInit(rIter, &behaviorArg);
    switch (behaviorArg) {
    case 1: mBehavior = Behavior_Unknown; setNerve(GET_NERVE(Rabbit, RabbitNrvTalk)); break;
    case 2: mBehavior = Behavior_UpJump; setNerve(GET_NERVE(Rabbit, RabbitNrvTalk)); break;
    case 3: mBehavior = Behavior_LongJump; setNerve(GET_NERVE(Rabbit, RabbitNrvTalk)); break;
    }
    mJumpHeight = 0.0f;
    if (MR::isValidSwitchAppear(this)) {
        makeActorDead();
        pushNerve(GET_NERVE(Rabbit, RabbitNrvAppear));
    } else {
        makeActorAppeared();
        MR::startBckIfExist(this, "Wait2");
    }
    MR::setClippingFar100m(this);
    const char* turn = "TurnSmall";
    mParam._14 = "Wait";
    mParam._18 = turn;
    mParam._1C = "Talk";
    mParam._20 = turn;
    setDefaults("Reaction", "Pointing", "Press", "Spin");
    mSpinAttackRange = 450.0f;
    if (mBehavior == Behavior_LongJump) {
        mParam._0 = false;
        mParam._1 = false;
    }
}

void Rabbit::control() {
    if (mTrampledStart) {
        MR::startActionSound(this, "SmNpcTrampled");
        MR::startActionSound(this, "SvRabbitTrampled");
    }
    NPCActor::control();
}
// NONMATCHING: temporary vector stack slots exchanged.
void Rabbit::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::emitEffect(this, "Light");
        MR::startBckIfExist(this, "Appear");
    }

    ParabolicPath path = ParabolicPath();
    TVec3f v9, v8;
    const TVec3f up = -mGravity;
    v8.set(0.0f, ::cAppearHeight, 0.0f);
    path.initFromUpVector(v8, TVec3f(0.0f, 0.0f, 0.0f), up, ::cAppearHop);
    path.calcPosition(&v9, getNerveStep() / MR::getBckFrameMax(this));
    mJumpHeight = -v9.y;

    if (MR::isBckOneTimeAndStopped(this)) {
        mJumpHeight = 0.0f;
        setNerve(GET_NERVE(Rabbit, RabbitNrvAppearLand));
    }
}

void Rabbit::exeAppearLand() {
    if (MR::isFirstStep(this)) {
        MR::tryStartBck(this, "AppearLand");
    }

    if (MR::isBckOneTimeAndStopped(this)) {
        popNerve();
    }
}

void Rabbit::calcAndSetBaseMtx() {
    TVec3f pos(mPosition);
    mPosition += mGravity * mJumpHeight;
    NPCActor::calcAndSetBaseMtx();
    mPosition = pos;
}

void Rabbit::exeReaction() {
    if (mTrampledStart) {
        MR::startActionSound(this, "SmNpcTrampled");
        MR::startActionSound(this, "SvRabbitTrampled");
    }

    if (isPointingSe()) {
        MR::startDPDHitSound();
    }

    if (mPointed) {
        MR::startActionSound(this, "SmLvRabbitPoint");
    }

    if (mSpinAttackedStart) {
        MR::startActionSound(this, "SvRabbitSpin");
        MR::startActionSound(this, "SmRabbitSpin");
    }

    if (mReactionStart) {
        MR::limitedStarPieceHitSound();
        MR::startActionSound(this, "SmRabbitStarPieceHit");
        MR::startActionSound(this, "SvRabbitStarPieceHit");
    }

    if (MR::tryStartReactionAndPopNerve(this)) {
        return;
    }
}

void Rabbit::exeTalk() {
    if (MR::isFirstStep(this)) {
    }

    if (!MR::tryStartReactionAndPushNerve(this, GET_NERVE(Rabbit, RabbitNrvReaction)) &&
        (!MR::tryTalkNearPlayerAndStartTalkAction(this) || MR::isShortTalk(mMsgCtrl))) {
        if (mBehavior == Behavior_UpJump && MR::isGreaterStep(this, 180)) {
            pushNerve(GET_NERVE(Rabbit, RabbitNrvJumpV));
        } else if (mBehavior == Behavior_LongJump) {
            if (MR::isGreaterStep(this, 180)) {
                pushNerve(GET_NERVE(Rabbit, RabbitNrvJumpH));
            }
        }
    }
}

void Rabbit::exeJumpV() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "SpinLecture");
    }

    if (MR::isBckOneTimeAndStopped(this)) {
        popNerve();
    }
}

void Rabbit::exeJumpH() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "JumpSpinLectureMove");
    }

    if (MR::isBckLooped(this)) {
        popNerve();
    }
}

