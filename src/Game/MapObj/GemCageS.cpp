#include "Game/MapObj/GemCageS.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
namespace NrvGemCageS {
    NEW_NERVE(GemCageSNrvWait, GemCageS, Wait);
    NEW_NERVE(GemCageSNrvBreak, GemCageS, Break);
}
GemCageS::GemCageS(const char* name) : LiveActor(name), mBreakModel(nullptr), mItemType(-1), mIsRockCage(false) {}
GemCageS::~GemCageS() {}
void GemCageS::init(const JMapInfoIter& iter) {
    MR::initActor(this, iter, false);
    mIsRockCage = MR::isObjectName(iter, "RockCageS");
    initNerve(&NrvGemCageS::GemCageSNrvWait::sInstance);
    mBreakModel = MR::createBreakModel(this, nullptr);
    MR::getJMapInfoArg0NoInit(iter, &mItemType);
    switch (mItemType) {
    case 0: MR::declareStarPiece(this, 5); break;
    case 1: MR::declareCoin(this, 1); break;
    }
    makeActorAppeared();
}
bool GemCageS::receiveMsgPlayerAttack(u32 msg, HitSensor*, HitSensor* receiver) {
    if (!isNerve(&NrvGemCageS::GemCageSNrvWait::sInstance)) return false;
    if (MR::isMsgRockAttack(msg)) {
        if (MR::isEqualSensor(receiver, this, "Collision")) {
            setNerve(&NrvGemCageS::GemCageSNrvBreak::sInstance);
            return true;
        }
        return false;
    }
    if (!mIsRockCage && MR::isMsgPlayerHitAll(msg)) {
        setNerve(&NrvGemCageS::GemCageSNrvBreak::sInstance);
        return true;
    }
    return false;
}
void GemCageS::attackSensor(HitSensor* sender, HitSensor* receiver) {
    if (!MR::isSensorRide(receiver)) MR::sendMsgPush(receiver, sender);
}
bool GemCageS::appearItem() {
    if (mItemType == 0) { MR::appearStarPiece(this, mPosition, 5, 10.0f, 40.0f, false); return true; }
    else if (mItemType == 1) { MR::appearCoinPop(this, mPosition, 1); return true; }
    return false;
}
void GemCageS::exeWait() {
    if (MR::isFirstStep(this)) MR::startBvaAndSetFrameAndStop(this, mIsRockCage ? "RockCageS" : "GemCageS", 0.0f);
    TVec3f up;
    MR::calcUpVec(&up, this);
}
void GemCageS::exeBreak() {
    if (MR::isFirstStep(this)) {
        MR::startBvaAndSetFrameAndStop(this, mIsRockCage ? "RockCageS" : "GemCageS", 1.0f);
        MR::startActionSound(this, "Break", -1, -1, -1);
        MR::invalidateCollisionParts(this);
        MR::invalidateHitSensors(this);
        mBreakModel->appear();
        if (MR::isValidSwitchDead(this)) MR::onSwitchDead(this);
        appearItem();
    }
}
