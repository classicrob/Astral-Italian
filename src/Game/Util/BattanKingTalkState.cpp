#define JGEOMETRY_VEC3_SUB_NEG_NOINLINE
#define JGEOMETRY_VEC3_ASSIGN_SUB_NOINLINE
#pragma auto_inline off
#include "Game/Util/BattanKingCollisionPress.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/TalkUtil.hpp"
#include "JSystem/J2DGraph/J2DPicture.hpp"

namespace {
    char sTalkStrings[] = "バッタンキングデモ状態\0\0"
                          "バッタンキング基準位置\0\0"
                          "バッタンキングダウン\0\0\0\0"
                          "ガクッ\0\0"
                          "死亡デモ\0\0\0\0"
                          "Watch\0\0\0"
                          "マリオ位置\0";
}

namespace BattanKingTalkVec {
    TVec3f sTalkOffset(0.0f, 1000.0f, 0.0f);
    TVec3f sBalloonOffset(-1100.0f, 1050.0f, -100.0f);
}

namespace NrvBattanKingTalkState {
    TryStart TryStart::sInstance;
    Talk Talk::sInstance;
    inline void Talk::execute(Spine* pSpine) const {
        static_cast<BattanKingTalkState*>(pSpine->mExecutor)->exeTalk();
    }
    inline void TryStart::execute(Spine* pSpine) const {
        static_cast<BattanKingTalkState*>(pSpine->mExecutor)->exeTryStart();
    }
}

BattanKingTalkState::BattanKingTalkState(BattanKing* host, const JMapInfoIter& iter, const char* demoName, const char* animName, const char* message)
    : ActorStateBase<BattanKing>(sTalkStrings, host), mTalkCtrl(0), mDemoName(demoName), mFlag18(0) {
    initNerve(&NrvBattanKingTalkState::TryStart::sInstance, 0);
    TalkMessageCtrl* talk = MR::createTalkCtrlDirect(host, iter, message, BattanKingTalkVec::sTalkOffset, host->getBaseMtx());
    MR::onRootNodeAutomatic(talk);
    MR::onTalkSmg2Flag52(talk);
    mTalkCtrl = talk;
    MR::initDemoSheetTalkAnim(host, iter, demoName, animName, talk, TVec3f(0.0f, 0.0f, 0.0f), 0);
}
void BattanKingTalkState::appear() {
    mIsDead = false;
    setNerve(&NrvBattanKingTalkState::TryStart::sInstance);
}
void BattanKingTalkState::exeTryStart() {
    if (MR::tryStartTimeKeepDemoMarioPuppetable(mHost, mDemoName, 0)) setNerve(&NrvBattanKingTalkState::Talk::sInstance);
}
void BattanKingTalkState::exeTalk() {
    const char* strings = sTalkStrings;
    if (MR::isFirstStep(this)) {
        MR::zeroVelocity(mHost);
        if (!mFlag18) {
            MR::showModel(mHost);
            MR::validateShadowAll(mHost);
            TPos3f mtx;
            mtx.identity();
            MR::findLinkNamePos(mHost, strings + 24, mtx.mMtx);
            MR::extractMtxTrans(mtx.mMtx, &mHost->mPosition);
            MR::extractMtxZDir(mtx.mMtx, &mHost->mFront);
            MR::setPlayerBaseMtx(mtx.mMtx);
        } else {
            mHost->restoreDemoPose();
            MR::setPlayerBaseMtx(mHost->getBaseMtx());
        }
    }
    if (MR::isDemoActive(strings + 48)) {
        MR::setMessageBalloonFollowOffset(mTalkCtrl, BattanKingTalkVec::sBalloonOffset);
        if (MR::isDemoPartActive(strings + 72) || MR::isDemoPartGreaterStep(strings + 80, 5)) MR::tryTalkTimeKeepDemoForceWithoutDemo(mTalkCtrl);
    }
    bool ended;
    if (mFlag18) ended = MR::isDemoLastStep();
    else ended = MR::tryEndDemoAndKillEffects(mHost, mDemoName);
    if (ended) {
        if (!mFlag18 && MR::isDemoEndWaitCameraInterpolating()) MR::readMessage(mTalkCtrl);
        if (mFlag18) {
            mHost->updateDemoPlayerBaseMtx();
            MR::startBckPlayer(strings + 92, 0);
        } else MR::setPlayerLinkPosAndWait(mHost, strings + 100, false);
        kill();
    }
}
BattanKingTalkState::~BattanKingTalkState() {}
