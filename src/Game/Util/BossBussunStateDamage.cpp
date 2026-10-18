#include "Game/Util/BossBussun.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace NrvBossBussunStateDamage {
    NEW_NERVE(BossBussunStateDamageNrvDamage, BossBussunStateDamage, Damage);
};  // namespace NrvBossBussunStateDamage

BossBussunStateDamage::BossBussunStateDamage(BossBussun* pHost) : ActorStateBase< BossBussun >("ボスブッスンダメージ", pHost) {
    mPosA.set(0.0f, 0.0f, 0.0f);
    mPosB.set(0.0f, 0.0f, 0.0f);
    mPosC.set(0.0f, 0.0f, 0.0f);
    initNerve(GET_NERVE(BossBussunStateDamage, BossBussunStateDamageNrvDamage), 0);
}

void BossBussunStateDamage::appear() {
    mIsDead = false;
    setNerve(GET_NERVE(BossBussunStateDamage, BossBussunStateDamageNrvDamage));
    fn_80098D20(mHost);
}

void BossBussunStateDamage::exeDamage() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "Damage");
        MR::zeroVelocity(mHost);
        fn_80099D10(mHost->mSequencer);
        MR::startMultiActorCameraTargetSelf(mHost, mHost->mCameraInfo, "Damage", 0);
        MR::stopScene(10);
        updateDamagePos();
    }
    if (MR::isStep(this, 1)) {
        MR::emitEffectHit(mHost, mHost->mPosition, nullptr);
    }
    MR::vecBlend(&mHost->mPosition, mPosA, mPosB, MR::calcNerveRate(this, 0x28));
    if (MR::isGreaterEqualStep(this, 1) && MR::isLessStep(this, 0x28)) {
        TVec3f axis(0.0f, 1.0f, 0.0f);
        MR::turnVecToVecDegree(&mHost->_D0, mHost->_D0, mPosC, 180.0f, axis);
    }
    if (MR::isStep(this, 0x32)) {
        MR::endMultiActorCamera(mHost, mHost->mCameraInfo, "Damage", false, -1);
        fn_80099CD0(mHost->mSequencer);
    }
    if (MR::isGreaterStep(this, 0x78)) {
        kill();
    }
}

// Translation of fn_8009BE10 (asm-faithful call order and constants).
void BossBussunStateDamage::updateDamagePos() {
    TVec3f sp50;
    TVec3f sp44;
    TVec3f sp38;
    TVec3f sp20;
    TVec3f sp14;
    TVec3f sp8;
    TVec3f sp2C;

    MR::calcVecToTargetPosH(&sp50, mHost, mHost->_DC, &mHost->_E8);
    if (MR::isNearZero(sp50, 0.001f)) {
        sp50.set(mHost->_D0);
    }
    sp50.x = -sp50.x;
    sp50.y = -sp50.y;
    sp50.z = -sp50.z;
    sp50.scale(800.0f);

    sp44.set(mHost->_DC);
    sp44.add(sp50);

    MR::calcUpVec(&sp38, mHost);
    sp20.set(sp38);
    sp20.scale(400.0f);
    sp44.add(sp20);

    mPosA.set(mHost->mPosition);

    sp14.set(sp38);
    sp14.scale(-800.0f);
    MR::getFirstPolyOnLineToMap(&mPosB, nullptr, sp44, sp14);

    sp8.set(mPosA);
    sp8.x -= mPosB.x;
    sp8.y -= mPosB.y;
    sp8.z -= mPosB.z;

    f32 d = sp38.dot(sp8);
    JMAVECScaleAdd((Vec*)&sp38, (Vec*)&sp8, (Vec*)&sp2C, -d);
    if (MR::normalizeOrZero(&sp2C)) {
        mPosC.set(mHost->_D0);
    } else {
        mPosC.set(sp2C);
    }
}

BossBussunStateDamage::~BossBussunStateDamage() {
}
