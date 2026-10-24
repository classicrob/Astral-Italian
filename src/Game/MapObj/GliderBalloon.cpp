#pragma auto_inline off
#include "Game/MapObj/GliderBalloon.hpp"
#include "Game/MapObj/StarPieceTracking.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/SupportPlayUtil.hpp"
namespace { s32 sMaxColorFrame = 6; const char sHitEffect[] = "Hit"; }
namespace NrvGliderBalloon {
NEW_NERVE(GliderBalloonNrvWait, GliderBalloon, Wait);
NEW_NERVE(GliderBalloonNrvBurst, GliderBalloon, Burst);
NEW_NERVE(GliderBalloonNrvStarPieceBurst, GliderBalloon, StarPieceBurst);
}
GliderBalloon::GliderBalloon(const char* name) : LiveActor(name), mPieceCount(5), mPieces(0), mPieceSpeed(1.4f), mPieceDistance(1500.0f), mUsePlayerVelocity(true), mEffectOffset(0.0f, 0.0f, 0.0f) {}
// NONMATCHING: tracking-group construction loop registers and string-address scheduling.
void GliderBalloon::init(const JMapInfoIter& iter) {
    bool mini = false;
    MR::getJMapInfoArg0NoInit(iter, &mini);
    if (mini) MR::initActor(this, iter, "GliderBalloonMini", false);
    else MR::initActor(this, iter, "GliderBalloon", false);
    MR::addEffectHitNormal(this, sHitEffect);
    MR::setEffectBaseScale(this, sHitEffect, 4.0f);
    mEffectOffset.x = 0.0f; mEffectOffset.y = 200.0f; mEffectOffset.z = 0.0f;
    if (mini) {
        mScale = TVec3f(1.0f);
        mPieceSpeed = 0.7f;
        mUsePlayerVelocity = false;
        TVec3f up(0.0f);
        MR::calcUpVec(&up, this);
        up = -up;
        MR::setShadowDropDirection(this, "Body", up);
        MR::setEffectBaseScale(this, sHitEffect, 1.0f);
        mEffectOffset.x = 0.0f; mEffectOffset.y = 50.0f; mEffectOffset.z = 0.0f;
    }
    s32 color = 0;
    MR::getJMapInfoArg1NoInit(iter, &color);
    if (color > sMaxColorFrame) color = sMaxColorFrame;
    MR::startBrkAndSetFrameAndStop(this, "Color", color);
    f32 distance = -1.0f;
    MR::getJMapInfoArg2NoInit(iter, &distance);
    if (distance > 0.0f) mPieceDistance = distance;
    MR::startBtkAndSetFrameAndStop(this, "Burst", 0.0f);
    initNerve(&NrvGliderBalloon::GliderBalloonNrvWait::sInstance);
    MR::declareStarPiece(this, mPieceCount);
    StarPieceTracking* group = new StarPieceTracking("スターピース", mPieceCount);
    mPieces = group;
    const char* pieceName = "追尾スターピース";
    for (s32 i = 0; i < group->mObjNumMax; ++i) {
        StarPieceTrackingPiece* piece = new StarPieceTrackingPiece(pieceName);
        piece->initWithoutIter();
        group->registerActor(piece);
    }
    mPieces->makeAllDead();
    if (MR::isStageJungleGliderGalaxyExplainDemo()) MR::registerDemoSimpleCastAll(this);
    makeActorAppeared();
}
void GliderBalloon::attackSensor(HitSensor* sender, HitSensor* receiver) {
    if (isNerve(&NrvGliderBalloon::GliderBalloonNrvWait::sInstance)) {
        if (MR::isSensorPlayerOrRide(receiver)) {
            appearStarPieceTracking();
            setNerve(&NrvGliderBalloon::GliderBalloonNrvBurst::sInstance);
            return;
        }
        MR::sendMsgEnemyAttack(receiver, sender);
    }
}
bool GliderBalloon::receiveMsgPlayerAttack(u32 msg, HitSensor*, HitSensor*) {
    if (!isNerve(&NrvGliderBalloon::GliderBalloonNrvWait::sInstance)) return false;
    if (MR::isMsgLockOnStarPieceShoot(msg)) return true;
    if (MR::isMsgStarPieceAttack(msg)) { setNerve(&NrvGliderBalloon::GliderBalloonNrvStarPieceBurst::sInstance); return true; }
    if (MR::isMsgSupportTicoSpin(msg)) { appearStarPieceTracking(); setNerve(&NrvGliderBalloon::GliderBalloonNrvBurst::sInstance); return true; }
    return false;
}
void GliderBalloon::exeWait() {
    if (MR::isFirstStep(this)) MR::startAction(this, "Wait");
    MR::attachSupportTicoToTarget(this);
}
void GliderBalloon::exeBurst() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Burst");
        TVec3f pos(mPosition);
        pos += mEffectOffset;
        MR::emitEffectHit(this, pos, sHitEffect);
        MR::tryRumblePadStrong(this, 0);
    }
    if (MR::isBtkOneTimeAndStopped(this)) kill();
}
void GliderBalloon::exeStarPieceBurst() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Burst");
        MR::emitEffectHit(this, mPosition, sHitEffect);
        MR::tryRumblePadStrong(this, 0);
        MR::appearStarPiece(this, mPosition, mPieceCount, 15.0f, 60.0f, false);
    }
    if (MR::isBtkOneTimeAndStopped(this)) kill();
}
void GliderBalloon::appearStarPieceTracking() {
    TVec3f up(0.0f), side(0.0f), front(0.0f);
    MR::calcUpVec(&up, this);
    MR::calcSideVec(&side, this);
    if (mUsePlayerVelocity) {
        front = *MR::getPlayerVelocity();
        if (PSVECMag(&front) < 1.0f) { MR::calcFrontVec(&front, this); const TVec3f& reverseFront = -front; front = reverseFront; }
        else { MR::normalizeOrZero(&front, front); PSVECCrossProduct(&front, &up, &side); PSVECCrossProduct(&front, &side, &up); }
    } else {
        TPos3f mtx;
        mtx.identity();
        MR::calcPlayerJointMtx(&mtx, "Center");
        mtx.getZDir(front);
        mtx.getYDir(up);
        TVec3f bias(up);
        f32 biasScale = 0.05f;
        bias.x *= biasScale; bias.y *= biasScale; bias.z *= biasScale;
        front += bias;
        if (MR::normalizeOrZero(&front)) mtx.getZDir(front);
        PSVECCrossProduct(&up, &front, &side);
        if (MR::normalizeOrZero(&side)) MR::getPlayerSideVec(&side);
        PSVECCrossProduct(&side, &front, &up);
        if (MR::normalizeOrZero(&up)) mtx.getYDir(up);
    }
    if (mUsePlayerVelocity) mPieces->appear(*MR::getSensorPos(this, "Body"), front, up, side, 360.0f, mPieceSpeed, 50.0f, mPieceDistance);
    else mPieces->appear(*MR::getSensorPos(this, "Body"), front, up, side, 360.0f, mPieceSpeed, 0.0f, 1000.0f);
}
GliderBalloon::~GliderBalloon() {}
