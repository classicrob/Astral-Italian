#pragma auto_inline off
#include "Game/Boss/TwoLegs.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"

namespace NrvTwoLegsStateDamage {
    NEW_NERVE(TwoLegsStateDamageNrvDamage, TwoLegsStateDamage, Damage);
    NEW_NERVE(TwoLegsStateDamageNrvRunAwayJump, TwoLegsStateDamage, RunAwayJump);
    NEW_NERVE(TwoLegsStateDamageNrvHide, TwoLegsStateDamage, Hide);
    NEW_NERVE(TwoLegsStateDamageNrvRunAwayLand, TwoLegsStateDamage, RunAwayLand);
};  // namespace NrvTwoLegsStateDamage

TwoLegsStateDamage::TwoLegsStateDamage(TwoLegs* pHost) : ActorStateBase< TwoLegs >("State[\x83_\x83\x81\x81[\x83W]", pHost) {}

TwoLegsStateDamage::~TwoLegsStateDamage() {}

void TwoLegsStateDamage::init() {
    TwoLegs* host = mHost;
    MR::declareStarPiece(host, (host->getPhaseNum() - 1) * 12);
    initNerve(&NrvTwoLegsStateDamage::TwoLegsStateDamageNrvDamage::sInstance, 0);
}

void TwoLegsStateDamage::appear() {
    mIsDead = false;
    mHost->jointTrackOff();
    mHost->mIsUp = true;
    setNerve(&NrvTwoLegsStateDamage::TwoLegsStateDamageNrvDamage::sInstance);
}

bool TwoLegsStateDamage::isHide() {
    return isNerve(&NrvTwoLegsStateDamage::TwoLegsStateDamageNrvRunAwayLand::sInstance);
}

void TwoLegsStateDamage::appearStarPiece() {
    TVec3f posA;
    mHost->calcAllRootPosOnPlane(&posA);
    TVec3f posB;
    mHost->getAllRootPos(&posB);
    TVec3f pos;
    JMAVECLerp(&posA, &posB, &pos, 0.5f);
    MR::appearStarPiece(mHost, pos, 12, 25.0f, 55.0f, false);
}

void TwoLegsStateDamage::exeDamage() {
    if (MR::isFirstStep(this)) {
        mHost->placeAtAllRoot();
        MR::startAction(mHost, "Damage");
        MR::startActionSound(mHost, "DamageAS", -1, -1, -1);
    }
    if (MR::isActionEnd(mHost)) {
        setNerve(&NrvTwoLegsStateDamage::TwoLegsStateDamageNrvRunAwayJump::sInstance);
    }
}

void TwoLegsStateDamage::exeRunAwayJump() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "RunAwayJump");
    }
    if (MR::isStep(this, 20)) {
        appearStarPiece();
    }
    if (MR::isStep(this, 43)) {
        setNerve(&NrvTwoLegsStateDamage::TwoLegsStateDamageNrvHide::sInstance);
    }
}

void TwoLegsStateDamage::exeHide() {
    if (MR::isFirstStep(this)) {
        MR::hideModel(mHost);
    }
    if (MR::isGreaterStep(this, 60) && !MR::isPlayerInRush()) {
        setNerve(&NrvTwoLegsStateDamage::TwoLegsStateDamageNrvRunAwayLand::sInstance);
    }
}

void TwoLegsStateDamage::exeRunAwayLand() {
    if (MR::isFirstStep(this)) {
        const TVec3f* pPlayer = MR::getPlayerPos();
        TVec3f dir;
        dir.sub(mHost->getBodyDir()->mPos, *pPlayer);
        mHost->placeOnBodyPlane(dir);
        MR::showModel(mHost);
        MR::startAction(mHost, "RunAwayLand");
    }
    if (MR::isActionEnd(mHost)) {
        kill();
    }
}
