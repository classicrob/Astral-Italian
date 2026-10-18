#pragma auto_inline off
#include "Game/Boss/KoopaFinalBattlePos.hpp"
#include "Game/Boss/KoopaBattleAccessor.hpp"
#include "Game/Boss/KoopaBattleFunction.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace NrvKoopaFireBall {
    NEW_NERVE(KoopaFireBallNrvShot, KoopaFireBall, Shot);
    NEW_NERVE(KoopaFireBallNrvLand, KoopaFireBall, Land);
    NEW_NERVE(KoopaFireBallNrvBreak, KoopaFireBall, Break);
};  // namespace NrvKoopaFireBall

KoopaFinalBattlePos::KoopaFinalBattlePos(const char* pName) : LiveActor(pName) {
}

void KoopaFinalBattlePos::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::initUseStageSwitchWriteA(this, rIter);
    initHitSensor(1);
    MR::addBodyMessageSensorMapObj(this);
    MR::createKoopaBattleAccessor();
    MR::getKoopaBattleAccessor()->setPoseB(this);
    MR::addKoopaBattleReceiverSensor(getSensor(nullptr));
    makeActorDead();
}

bool KoopaFinalBattlePos::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgKoopaFinalBattleStart(msg)) {
        if (MR::isValidSwitchA(this)) {
            MR::onSwitchA(this);
        }
        return true;
    }
    return false;
}

KoopaFinalBattlePos::~KoopaFinalBattlePos() {}

KoopaFireBall::KoopaFireBall(const char* pName) : LiveActor(pName), mCount(1) {
    mQuat.set< f32 >(0.0f, 0.0f, 0.0f, 1.0f);
}

void KoopaFireBall::kill() {
    LiveActor::kill();
    MR::deleteEffectAll(this);
}

void KoopaFireBall::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mQuat);
}

void KoopaFireBall::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "KoopaFireBall", false);
    initNerve(&NrvKoopaFireBall::KoopaFireBallNrvShot::sInstance, 0);
    makeActorDead();
}

void KoopaFireBall::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isShotOrLand()) {
        if (MR::isSensorPlayer(pReceiver)) {
            if (!MR::sendMsgEnemyAttackFire(pReceiver, pSender)) {
                MR::sendMsgPush(pReceiver, pSender);
            }
        } else if (MR::sendMsgKoopaBD(pReceiver, pSender)) {
            TVec3f dir;
            MR::calcSensorDirectionNormalize(&dir, pReceiver, pSender);
            MR::setQuatUpNoSupport(this, dir);
            MR::zeroVelocity(this);
            setNerve(&NrvKoopaFireBall::KoopaFireBallNrvBreak::sInstance);
        }
    }
}

void KoopaFireBall::exeShot() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Shot");
        MR::zeroVelocity(this);
    }
    TVec3f facePos;
    MR::calcKoopaBattleFacePos(&facePos);
    TVec3f edgePos;
    MR::calcKoopaBattleActorEdgePos(&edgePos);
    TVec3f dir = edgePos - facePos;
    if (!MR::normalizeOrZero(&dir)) {
        MR::turnQuatZDirRate(&mQuat, mQuat, dir, 0.5f);
    }
    MR::vecBlend(&mPosition, facePos, edgePos, MR::calcNerveRate(this, 0x2D));
    MR::setScale(this, MR::calcNerveEaseInOutValue(this, 0x2D, 1.0f, 1.5f));
    MR::setSensorRadius(this, "Body", 190.0f * mScale.x);
    if (MR::isGreaterStep(this, 0x2D)) {
        if (mCount >= 2) {
            setNerve(&NrvKoopaFireBall::KoopaFireBallNrvLand::sInstance);
            return;
        }
        setNerve(&NrvKoopaFireBall::KoopaFireBallNrvBreak::sInstance);
    }
}

void KoopaFireBall::exeLand() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Land");
        MR::hideModelAndOnCalcAnim(this);
        MR::setSensorRadius(this, "Body", 190.0f);
    }
    if (MR::isGreaterStep(this, 0x78)) {
        setNerve(&NrvKoopaFireBall::KoopaFireBallNrvBreak::sInstance);
    }
}

void KoopaFireBall::exeBreak() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Break");
        MR::hideModelAndOnCalcAnim(this);
    }
    if (MR::isActionEnd(this)) {
        kill();
    }
}

void KoopaFireBall::shot(s32 count) {
    mCount = count;
    TVec3f facePos;
    MR::calcKoopaBattleFacePos(&facePos);
    TVec3f edgePos;
    MR::calcKoopaBattleActorEdgePos(&edgePos);
    TVec3f dir = edgePos - facePos;
    if (!MR::normalizeOrZero(&dir)) {
        MR::turnQuatZDirRate(&mQuat, mQuat, dir, 1.0f);
    }
    MR::resetPosition(this, facePos);
    mVelocity.zero();
    setNerve(&NrvKoopaFireBall::KoopaFireBallNrvShot::sInstance);
    MR::setScale(this, 1.0f);
    MR::setSensorRadius(this, "Body", 190.0f * mScale.x);
    MR::showModel(this);
    appear();
}

bool KoopaFireBall::isShotOrLand() const {
    if (isNerve(&NrvKoopaFireBall::KoopaFireBallNrvShot::sInstance) || isNerve(&NrvKoopaFireBall::KoopaFireBallNrvLand::sInstance)) {
        return true;
    }
    return false;
}

KoopaFireBall::~KoopaFireBall() {}
