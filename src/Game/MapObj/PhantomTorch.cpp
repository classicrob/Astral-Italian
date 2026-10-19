#include "Game/MapObj/PhantomTorch.hpp"
#include "Game/Effect/SpinPullParticleCallBack.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/Util/SupportPlayUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <revolution/types.h>

namespace NrvPhantomTorch {
    NEW_NERVE(HostTypeWait, PhantomTorch, Wait);
    NEW_NERVE(HostTypeAttack, PhantomTorch, Attack);
    NEW_NERVE(HostTypeExtinguish, PhantomTorch, Extinguish);
};  // namespace NrvPhantomTorch

namespace {
    const char* sRisePictureBookParticle = "PhantomTorchRotate";
};  // namespace

PhantomTorch::~PhantomTorch() {
}

PhantomTorch::PhantomTorch(const char* pName) : LiveActor(pName) {
    mParticle = nullptr;
    mItem = -1;
    mPullParticle = nullptr;
    mIsDecorative = false;
}

void PhantomTorch::initFromRisePictureBook() {
    mItem = 1;
    mParticle = sRisePictureBookParticle;
    initWithoutIter();
}

void PhantomTorch::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    f32 x = mScale.x;

    if (rIter.isValid()) {
        MR::getJMapInfoArg0NoInit(rIter, &mItem);
        s32 arg = -1;
        MR::getJMapInfoArg1NoInit(rIter, &arg);
        mIsDecorative = arg != -1;
        MR::getObjectName(&mParticle, rIter);
    }

    if (mItem == -1) {
        MR::declareCoin(this, 1);
    }

    if (!mItem) {
        MR::declareStarPiece(this, 1);
    }

    initEffectKeeper(0, mParticle, false);
    mPullParticle = new SpinPullParticleCallBack();
    initSound(4, "PhantomTorch", nullptr, TVec3f(0.0f));
    initHitSensor(1);
    TVec3f vec;
    vec.x = 0.0f;
    vec.y = 70.0f * x;
    vec.z = 0.0f;
    MR::addHitSensorMapObj(this, "body", 8, 50.0f * x, vec);
    MR::setClippingTypeSphere(this, 500.0f * x);
    MR::initStarPointerTarget(this, 50.0f * x, TVec3f(0.0f, 70.0f * x, 0.0f));
    MR::joinToGroupArray(this, rIter, mParticle, 32);
    initNerve(GET_NERVE(PhantomTorch, HostTypeWait));
    MR::connectToSceneMapObjMovement(this);
    makeActorAppeared();
    MR::initUseStageSwitchWriteDead(this, rIter);
    MR::useStageSwitchAwake(this, rIter);

    if (MR::initUseStageSwitchReadAppear(this, rIter)) {
        MR::listenStageSwitchOnOffAppear(this, MR::Functor(this, &PhantomTorch::makeActorAppeared), MR::Functor(this, &PhantomTorch::makeActorDead));
        makeActorDead();
    }
}

void PhantomTorch::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    if (!MR::isClipped(this)) {
        MR::emitEffect(this, mParticle);
    }
}

void PhantomTorch::makeActorDead() {
    if (MR::isValidSwitchDead(this)) {
        MR::onSwitchDead(this);
    }
    MR::deleteEffect(this, mParticle);
    LiveActor::makeActorDead();
}

void PhantomTorch::kill() {
    switch (mItem) {
    case -1:
        MR::appearCoinPop(this, mPosition, 1);
        break;
    case 1:
        break;
    case 2:
        break;
    case 0:
        if (MR::appearStarPiece(this, mPosition, 1, 10.0f, 40.0f, false)) {
            MR::startActionSound(this, "OjStarPieceBurst");
        }
        break;
    }
    soundOrder();
    LiveActor::kill();
}

void PhantomTorch::startClipped() {
    LiveActor::startClipped();
    MR::deleteEffect(this, mParticle);
}

void PhantomTorch::endClipped() {
    MR::emitEffect(this, mParticle);
    LiveActor::endClipped();
}

void PhantomTorch::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!mIsDecorative && isNerve(GET_NERVE(PhantomTorch, HostTypeWait)) && MR::sendMsgEnemyAttackFire(pReceiver, pSender)) {
        setNerve(GET_NERVE(PhantomTorch, HostTypeAttack));
    }
}

bool PhantomTorch::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (mIsDecorative) {
        return false;
    }

    if (MR::isMsgPlayerSpinAttackOrSupportTico(msg) && isNerve(GET_NERVE(PhantomTorch, HostTypeWait))) {
        setNerve(GET_NERVE(PhantomTorch, HostTypeExtinguish));
        return true;
    }

    return false;
}

bool PhantomTorch::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (mIsDecorative) {
        return false;
    }

    if (MR::isInSpinStormRange(msg, pSender, pReceiver, 350.0f) && isNerve(GET_NERVE(PhantomTorch, HostTypeWait))) {
        setNerve(GET_NERVE(PhantomTorch, HostTypeExtinguish));
        return true;
    }
    return false;
}

void PhantomTorch::soundOrder() {
    if (MR::getGroupFromArray(this) && MR::getGroupFromArray(this)->getObjNum() > 0) {
        s32 i;
        LiveActorGroup* group;
        s32 num = 0;
        group = MR::getGroupFromArray(this);

        for (i = 0; i < group->getObjNum(); i++) {
            if (!MR::isDead(group->getActor(i))) {
                num++;
            }
        }

        switch (num) {
        case 1:
            MR::startActionSound(this, "SyCandleCombo1", -1, -1, -1);
            break;
        case 2:
            MR::startActionSound(this, "SyCandleCombo2", -1, -1, -1);
            break;
        case 3:
            MR::startActionSound(this, "SyCandleCombo3", -1, -1, -1);
            break;
        case 4:
            MR::startActionSound(this, "SyCandleCombo4", -1, -1, -1);
            break;
        case 5:
        default:
            MR::startActionSound(this, "SyCandleCombo5", -1, -1, -1);
            break;
        }
    }
}

void PhantomTorch::exeWait() {
    if (MR::isEqualString(mParticle, "PhantomBonfire")) {
        MR::startActionSound(this, "OjLvBonfireBurn");
    } else {
        MR::startActionSound(this, "OjLvPhantomToachBurn");
    }

    MR::attachSupportTicoToTarget(this);
}

void PhantomTorch::exeAttack() {
    if (MR::isStep(this, 30)) {
        setNerve(GET_NERVE(PhantomTorch, HostTypeWait));
        return;
    } else if (MR::isEqualString(mParticle, "PhantomBonfire")) {
        MR::startActionSound(this, "OjLvBonfireBurn");
    } else {
        MR::startActionSound(this, "OjLvPhantomToachBurn");
    }

    MR::attachSupportTicoToTarget(this);
}

void PhantomTorch::exeExtinguish() {
    if (MR::isFirstStep(this)) {
        MR::deleteEffect(this, mParticle);
        MR::emitEffectWithParticleCallBack(this, "Extinguish", mPullParticle);
        MR::startActionSound(this, "OjPhantomToachOff");
    }

    if (MR::isStep(this, 15)) {
        kill();
    }
}
