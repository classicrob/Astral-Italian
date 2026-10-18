#pragma auto_inline off
#include "Game/MapObj/PlayerFollowPass.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util.hpp"
#include "Game/Util/GravityUtil.hpp"
PlayerFollowPass::PlayerFollowPass()
    : NameObj("プレイヤー追随パス"), mPositions(nullptr), mDistances(nullptr), mCapacity(0), mCount(0), mCurrent(0), mDelay(0), mPaused(false) {
    mPositions = new TVec3f[64];
    mDistances = new f32[64];
    mCapacity = 64;
    mCount = 0;
    mCurrent = 0;
    MR::connectToSceneMapObjMovement(this);
    MR::registerDemoSimpleCastAll(this);
}
PlayerFollowPass::~PlayerFollowPass() {}
void PlayerFollowPass::initAfterPlacement() { reset(); }
void PlayerFollowPass::movement() {
    if (mPaused) return;
    if (mDelay > 0) { --mDelay; return; }
    update();
}
void PlayerFollowPass::clear() {
    mCurrent = 0;
    mCount = 0;
    mDelay = 2;
}
// NONMATCHING: temporary vector pointer allocation changes saved registers and stack frame.
void PlayerFollowPass::reset() {
    TVec3f position(0.0f);
    calcPlayerPosition(&position);
    TVec3f front;
    MR::getPlayerFrontVec(&front);
    TVec3f gravity;
    if (!MR::calcGravityVectorOrZero(this, position, &gravity, nullptr, 0)) gravity.set<f32>(*MR::getPlayerGravity());
    JMAVECScaleAdd(&gravity, &front, &front, -gravity.dot(front));
    if (MR::isNearZero(front, 0.001f)) MR::getPlayerFrontVec(&front);
    TVec3f initial(position - front * 150.0f);
    mPositions[0].set<f32>(initial);
    mDistances[0] = 0.0f;
    mPositions[1].set<f32>(position);
    mDistances[1] = 150.0f;
    mCount = 2;
    mCurrent = 1;
}
void PlayerFollowPass::calcPlayerPosition(TVec3f* dst) {
    TVec3f position(*MR::getPlayerPos());
    TVec3f up;
    MR::extractMtxYDir(MR::getPlayerBaseMtx(), &up);
    position.add(position, up * 75.0f);
    dst->set<f32>(position);
}
void PlayerFollowPass::update() {
    TVec3f position(0.0f);
    calcPlayerPosition(&position);
    if (mCount <= 0) {
        mPositions[0].set<f32>(position);
        mDistances[0] = 0.0f;
        mPositions[1].set<f32>(position);
        mDistances[1] = 0.0f;
        mCount = 2;
        mCurrent = 1;
        return;
    }
    s32 previous = mCurrent - 1;
    if (previous < 0) previous += mCapacity;
    mPositions[mCurrent].set<f32>(position);
    mDistances[mCurrent] = PSVECDistance(&position, &mPositions[previous]);
    if (mDistances[mCurrent] < 20.0f) return;
    if (mCount < mCapacity) ++mCount;
    ++mCurrent;
    if (mCurrent >= mCapacity) mCurrent -= mCapacity;
    mPositions[mCurrent].set<f32>(position);
    mDistances[mCurrent] = 0.0f;
}
void PlayerFollowPass::calcPosition(TVec3f* dst, f32 distance) {
    if (mCount <= 1) { calcPlayerPosition(dst); return; }
    s32 current = mCurrent;
    f32 total = 0.0f;
    for (s32 i = 0; i < mCount - 1; ++i) {
        s32 previous = current - 1;
        if (previous < 0) previous += mCapacity;
        f32 segment = mDistances[current];
        total += segment;
        if (distance < total) {
            f32 rate = (distance - (total - segment)) / segment;
            f32 clamped = rate < 0.0f ? 0.0f : rate > 1.0f ? 1.0f : rate;
            MR::vecBlend(dst, mPositions[current], mPositions[previous], clamped);
            return;
        }
        current = previous;
    }
    dst->set<f32>(mPositions[current]);
}
void MR::createPlayerFollowPass() { MR::createSceneObj(SceneObj_PlayerFollowPass); }
void MR::calcPlayerFollowPassPosition(TVec3f* dst, f32 distance) {
    MR::getSceneObj<PlayerFollowPass*>(SceneObj_PlayerFollowPass)->calcPosition(dst, distance);
}
void MR::resetPlayerFollowPass() {
    if (MR::isExistSceneObj(SceneObj_PlayerFollowPass)) MR::getSceneObj<PlayerFollowPass*>(SceneObj_PlayerFollowPass)->reset();
}
void MR::clearPlayerFollowPass() {
    if (MR::isExistSceneObj(SceneObj_PlayerFollowPass)) MR::getSceneObj<PlayerFollowPass*>(SceneObj_PlayerFollowPass)->clear();
}
void MR::pausePlayerFollowPass() {
    if (MR::isExistSceneObj(SceneObj_PlayerFollowPass)) MR::getSceneObj<PlayerFollowPass*>(SceneObj_PlayerFollowPass)->mPaused = true;
}
void MR::resumePlayerFollowPass() {
    if (MR::isExistSceneObj(SceneObj_PlayerFollowPass)) MR::getSceneObj<PlayerFollowPass*>(SceneObj_PlayerFollowPass)->mPaused = false;
}
