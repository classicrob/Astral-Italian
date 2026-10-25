#include "Game/Util/Functor.hpp"
#include "Game/Player/GhostPlayerSound.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StringUtil.hpp"



namespace {
    struct AnimSoundInfo {
        const char* mAnimation;
        const char* mBodySound;
        const char* mVoiceSound;
    };

    const AnimSoundInfo sAnimSoundTable[] = {
        {"ジャンプ", "SE_BM_GHOST_MARIO_JUMP_S", "SE_BV_GHOST_MARIO_JUMP_S"},
        {"ジャンプB", "SE_BM_GHOST_MARIO_JUMP_M", "SE_BV_GHOST_MARIO_JUMP_M"},
        {"ジャンプC", "SE_BM_GHOST_MARIO_JUMP_L", "SE_BV_GHOST_MARIO_JUMP_L"},
        {"壁ジャンプ", "SE_BM_GHOST_MARIO_JUMP_S", "SE_BV_GHOST_MARIO_JUMP_S"},
        {"埋まり脱出ジャンプ", "SE_BM_GHOST_MARIO_JUMP_S", "SE_BV_GHOST_MARIO_JUMP_S"},
        {"幅とび", "SE_BM_GHOST_MARIO_JUMP_L", "SE_BV_GHOST_MARIO_JUMP_L"},
        {"しゃがみジャンプ", "SE_BM_GHOST_MARIO_JUMP_L", "SE_BV_GHOST_MARIO_JUMP_L"},
        {"崖つかまり開始", nullptr, "SE_BV_GHOST_MARIO_HANG"},
        {"崖つかまり終了", nullptr, "SE_BV_GHOST_MARIO_CLIMB"},
        {"壁押し", nullptr, "SE_BV_GHOST_MARIO_HANG"},
        {"ヒップドロップ開始", "SE_BM_GHOST_MARIO_HIP_DROP_TURN", "SE_BV_GHOST_MARIO_HIPDROP_S"},
        {"ヒップドロップ着地", "SE_BM_GHOST_MARIO_HIP_DROP_LAND", "SE_BV_GHOST_MARIO_HIPDROP_E"},
        {"埋まり脱出ジャンプ", "SE_BM_GHOST_MARIO_JUMP_S", "SE_BV_GHOST_MARIO_JUMP_S"},
        {"水泳スピン", "SE_BM_GHOST_MARIO_SPIN", "SE_BV_GHOST_MARIO_SPIN"},
        {"水泳ジェット開始", nullptr, "SE_BV_GHOST_MARIO_SWM_ACCEL"},
        {"カメ持ちリング準備", nullptr, "SE_BV_GHOST_MARIO_TAKE"},
        {"カメ持ちリング", nullptr, "SE_BV_GHOST_MARIO_SWM_ACCEL"},
        {"投げ", nullptr, "SE_BV_GHOST_MARIO_THROW"},
        {"空中ひねり", "SE_BM_GHOST_MARIO_SPIN", "SE_BV_GHOST_MARIO_SPIN"},
        {"地上ひねり", "SE_BM_GHOST_MARIO_SPIN", "SE_BV_GHOST_MARIO_SPIN"},
        {"空パンチ", "SE_BM_GHOST_MARIO_SPIN", "SE_BV_GHOST_MARIO_SPIN"},
        {"アイスひねり", "SE_BM_GHOST_MARIO_SPIN", "SE_BV_GHOST_MARIO_SPIN"},
        {"アイスひねり移動", "SE_BM_GHOST_MARIO_SPIN", "SE_BV_GHOST_MARIO_SPIN"},
        {"スケートアクセルジャンプ", "SE_BM_GHOST_MARIO_JUMP_L", "SE_BV_GHOST_MARIO_JUMP_L"},
        {"スケートジャンプ2", "SE_BM_GHOST_MARIO_JUMP_L", "SE_BV_GHOST_MARIO_JUMP_L"},
        {"氷上力行右", nullptr, "SE_BV_GHOST_MARIO_JUMP_S"},
        {"氷上力行左", nullptr, "SE_BV_GHOST_MARIO_JUMP_S"},
        {"ショートジャンプ", "SE_BM_GHOST_MARIO_JUMP_S", "SE_BV_GHOST_MARIO_JUMP_S"},
        {"雲スピン空中", "SE_BM_GHOST_MARIO_SPIN", "SE_BV_GHOST_MARIO_SPIN"},
        {"雲スピン", "SE_BM_GHOST_MARIO_SPIN", "SE_BV_GHOST_MARIO_SPIN"},
    };

    const AnimSoundInfo* getAnimSoundInfo(const char* animation) {
        for (u32 index = 0; index < 30; ++index) {
            if (MR::isEqualString(animation, sAnimSoundTable[index].mAnimation)) return &sAnimSoundTable[index];
        }
        return nullptr;
    }
}

namespace MR {
    void fn_803811B0(const LiveActor* actor, const char* animation) {
        const AnimSoundInfo* info = getAnimSoundInfo(animation);
        if (info == nullptr) return;
        if (info->mBodySound != nullptr) startSound(actor, info->mBodySound, -1, -1);
    }

    void fn_80381210(const LiveActor* actor, const char* animation) {
        const AnimSoundInfo* info = getAnimSoundInfo(animation);
        if (info == nullptr) return;
        if (info->mVoiceSound != nullptr) startSound(actor, info->mVoiceSound, -1, -1);
    }
}

#include "Game/Player/InfernoMario.hpp"
#include "Game/Player/InfernoMarioManager.hpp"
#include "Game/Player/InfernoMarioStateBase.hpp"
#include "Game/Player/InfernoMarioAppearance.hpp"
#include "Game/Player/InfernoMarioManekuri.hpp"
#include "Game/Player/GhostPacketData.hpp"
#include "Game/Player/GhostPacketHistory.hpp"
#include "Game/Player/GhostPlayer.hpp"
#include "Game/Player/GhostPlayerSound.hpp"
#include "Game/Util/DrawUtil.hpp"
#include "Game/Animation/XanimePlayer.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/Functor.hpp"
#include "Game/Util/StringUtil.hpp"
namespace NrvInfernoMario {
    NEW_NERVE(InfernoMarioNrvAppearance, InfernoMario, Appearance);
    NEW_NERVE(InfernoMarioNrvFollow, InfernoMario, Follow);
}
InfernoMario::InfernoMario(const char* name) : LiveActor(name), mXanimePlayer(nullptr), mUpperPlayer(nullptr), mHistory(nullptr), mState(nullptr), mInitialPos(0.0f,0.0f,0.0f), mFrameDelay(0), mFrameSpacing(50), mArg7(-1), mPlayExtraSound(false) {
    MR::createInfernoMarioManager();
    mActionName[0] = 0;
    mTargetQuat.set(0.0f,0.0f,0.0f,1.0f);
    mCurrentQuat.set(0.0f,0.0f,0.0f,1.0f);
    mInitialQuat.set(0.0f,0.0f,0.0f,1.0f);
}
InfernoMario::~InfernoMario() {}
void InfernoMario::init(const JMapInfoIter& iter) {
    MR::initActor(this, iter, "InfernoMario", false);
    MR::onCalcGravity(this);
    MR::makeQuatFromRotate(&mCurrentQuat, this);
    mTargetQuat.set(mCurrentQuat);
    mInitialQuat.set(mCurrentQuat);
    mInitialPos.set(mPosition);
    mXanimePlayer = MR::fn_8037FF80(this, nullptr);
    mUpperPlayer = MR::fn_80380030(this, mXanimePlayer, nullptr);
    initNerve(&NrvInfernoMario::InfernoMarioNrvFollow::sInstance, 2);
    if (MR::initUseStageSwitchReadB(this, iter))
        MR::listenStageSwitchOnOffB(this, MR::Functor(this, &InfernoMario::fn_80381730), MR::Functor(this, &InfernoMario::fn_80381760));
    if (MR::isValidInfo(iter))
        MR::getJMapInfoArg7NoInit(iter, &mArg7);
    MR::initActorState(this, new InfernoMarioAppearance(this), &NrvInfernoMario::InfernoMarioNrvAppearance::sInstance, "AppearDemo");
    if (MR::isValidInfo(iter)) {
        mState = new InfernoMarioManekuri(this);
        MR::initActorState(this, mState, &NrvInfernoMario::InfernoMarioNrvFollow::sInstance, "Run");
        MR::useStageSwitchSyncAppear(this, iter);
    } else {
        mState = new InfernoMarioComet(this);
        MR::initActorState(this, mState, &NrvInfernoMario::InfernoMarioNrvFollow::sInstance, "Run");
        MR::invalidateClipping(this);
        makeActorDead();
    }
}
void InfernoMario::draw() const { if (!MR::isDead(this)) MR::drawModelAlphaShadow(this); }
void InfernoMario::makeActorAppeared() { LiveActor::makeActorAppeared(); MR::emitEffect(this,"Shadow"); }
void InfernoMario::kill() { if(!MR::isDead(this)) MR::tryHitReactionDead(this); LiveActor::kill(); }
void InfernoMario::fn_80381670() { setNerve(&NrvInfernoMario::InfernoMarioNrvAppearance::sInstance); appear(); }
void InfernoMario::fn_803816C0(s32 frame) { if (mState && mState->startFrame(frame)) { setNerve(&NrvInfernoMario::InfernoMarioNrvFollow::sInstance); appear(); } }
void InfernoMario::fn_80381730() { if (mState) mState->fn_80381750(); }
void InfernoMario::fn_80381760() { if (mState) mState->fn_80381780(); }
void InfernoMario::fn_80381790() { mTargetQuat.set(mInitialQuat); mCurrentQuat.set(mInitialQuat); MR::resetPosition(this,mInitialPos); }
void InfernoMario::setFrameDelay(s32 delay) { mFrameDelay=delay; }
void InfernoMario::startActionName(const char* name) {
    if (!MR::isEqualString(name,mActionName)) {
        MR::fn_803811B0(this,name);
        if (mPlayExtraSound) MR::fn_80381210(this,name);
        MR::fn_803800C0(mXanimePlayer,name);
    }
    MR::copyString(mActionName,name,0x40);
}
void InfernoMario::startActionHash(u32 hash) {
    const char* name=mXanimePlayer->getCurrentAnimationName();
    if (!MR::isEqualString(name,mActionName)) {
        MR::fn_803811B0(this,name);
        if (mPlayExtraSound) MR::fn_80381210(this,name);
    }
    MR::fn_80380130(mXanimePlayer,mUpperPlayer,this,hash,nullptr);
    MR::copyString(mActionName,name,0x40);
}
void InfernoMario::setHistory(GhostPacketHistory* history) { mHistory=history; }
void InfernoMario::control() {}
void InfernoMario::calcAndSetBaseMtx() { MR::setBaseTRMtx(this,mCurrentQuat); }
void InfernoMario::attackSensor(HitSensor* sender,HitSensor* receiver) { if (isNerve(&NrvInfernoMario::InfernoMarioNrvFollow::sInstance) && mState) mState->attackSensor(sender,receiver); }
bool InfernoMario::receiveMsgPlayerAttack(u32 msg,HitSensor* sender,HitSensor* receiver) { if (isNerve(&NrvInfernoMario::InfernoMarioNrvFollow::sInstance) && mState) return mState->receiveMsgPlayerAttack(msg,sender,receiver); return false; }
bool InfernoMario::receiveMsgEnemyAttack(u32 msg,HitSensor* sender,HitSensor* receiver) { if (isNerve(&NrvInfernoMario::InfernoMarioNrvFollow::sInstance) && mState) return mState->receiveMsgEnemyAttack(msg,sender,receiver); return false; }
bool InfernoMario::receiveOtherMsg(u32 msg,HitSensor* sender,HitSensor* receiver) { if (isNerve(&NrvInfernoMario::InfernoMarioNrvFollow::sInstance) && mState) return mState->receiveOtherMsg(msg,sender,receiver); return false; }
bool InfernoMario::readPacket(s32 frame) { if (mHistory) { GhostPacketData packet; if (mHistory->read(frame,&packet)) { applyPacket(&packet); return true; } } return false; }
bool InfernoMario::readTransform(s32 frame) { if (mHistory) { GhostPacketData packet; if (mHistory->read(frame,&packet)) { applyTransform(&packet); return true; } } return false; }
void InfernoMario::applyTransform(const GhostPacketData* packet) {
    if(packet->testPosition()) mPosition.set(packet->mPosition);
    if(packet->testRotationX()) mRotation.x=packet->mRotation.x;
    if(packet->testRotationY()) mRotation.y=packet->mRotation.y;
    if(packet->testRotationZ()) mRotation.z=packet->mRotation.z;
    if(packet->testTRMtx()) MR::makeQuatFromRotate(&mTargetQuat,this);
    if(packet->testScale()) mScale.set(packet->mScale);
}
void InfernoMario::applyPacket(const GhostPacketData* packet) {
    applyTransform(packet);
    if(packet->testVelocity()) mVelocity.set(packet->mVelocity);
    if(packet->testActionHash()) startActionHash(packet->mActionHash);
    else if(packet->testActionName()) startActionName(packet->mActionName);
    if(packet->testBckFrame()) MR::getBckCtrl(this)->mFrame=packet->mBckFrame;
    if(packet->testBckRate()) MR::setBckRate(this,packet->mBckRate);
    if(packet->testTrackWeight0()) {
        f32 weight=packet->mTrackWeights[0];
        mXanimePlayer->changeTrackWeight(0,weight);
    }
    if(packet->testTrackWeight1()) {
        f32 weight=packet->mTrackWeights[1];
        mXanimePlayer->changeTrackWeight(1,weight);
    }
    if(packet->testTrackWeight2()) {
        f32 weight=packet->mTrackWeights[2];
        mXanimePlayer->changeTrackWeight(2,weight);
    }
    if(packet->testTrackWeight3()) {
        f32 weight=packet->mTrackWeights[3];
        mXanimePlayer->changeTrackWeight(3,weight);
    }
}
void InfernoMario::exeAppearance() { if(MR::updateActorState(this)) kill(); }
void InfernoMario::exeFollow() { if(MR::updateActorState(this)) kill(); }
