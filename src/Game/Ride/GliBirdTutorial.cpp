#pragma auto_inline off
#include "Game/LiveActor/ActorParam.hpp"
#include "Game/Ride/GliBird.hpp"
#include "Game/Camera/CameraTargetMtx.hpp"
#include "Game/LiveActor/ActorJointCtrl.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/VolumeDrawer.hpp"
#include "Game/Map/RaceManager.hpp"
#include "Game/NPC/GliBirdNpc.hpp"
#include "Game/NPC/Pichan.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Screen/GliBirdGuidance.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/Color.hpp"
#include "Game/Util/SupportPlayUtil.hpp"
#include <revolution/wpad.h>

namespace NrvGliBirdTutorial {
    NEW_NERVE(GliBirdTutorialNrvTalkFirst, GliBirdTutorial, TalkFirst);
    NEW_NERVE(GliBirdTutorialNrvTalkStraight, GliBirdTutorial, TalkStraight);
    NEW_NERVE(GliBirdTutorialNrvStraightStart, GliBirdTutorial, StraightStart);
    NEW_NERVE(GliBirdTutorialNrvStraightHold, GliBirdTutorial, StraightHold);
    NEW_NERVE(GliBirdTutorialNrvStraightSuccess, GliBirdTutorial, StraightSuccess);
    NEW_NERVE(GliBirdTutorialNrvTalkTurnLeft, GliBirdTutorial, TalkTurnLeft);
    NEW_NERVE(GliBirdTutorialNrvTurnLeftStart, GliBirdTutorial, TurnLeftStart);
    NEW_NERVE(GliBirdTutorialNrvTurnLeftHold, GliBirdTutorial, TurnLeftHold);
    NEW_NERVE(GliBirdTutorialNrvTurnLeftSuccess, GliBirdTutorial, TurnLeftSuccess);
    NEW_NERVE(GliBirdTutorialNrvTalkTurnRight, GliBirdTutorial, TalkTurnRight);
    NEW_NERVE(GliBirdTutorialNrvTurnRightStart, GliBirdTutorial, TurnRightStart);
    NEW_NERVE(GliBirdTutorialNrvTurnRightHold, GliBirdTutorial, TurnRightHold);
    NEW_NERVE(GliBirdTutorialNrvTurnRightSuccess, GliBirdTutorial, TurnRightSuccess);
    NEW_NERVE(GliBirdTutorialNrvEnd, GliBirdTutorial, End);
};  // namespace NrvGliBirdTutorial

namespace {
    static TVec3f sPichanOffset(-400.0f, 0.0f, -300.0f);
    static TVec3f sPichanRot(0.0f, 60.0f, 0.0f);
    static const char* sTutorialPosName = "チュートリアル位置";
};  // namespace


GliBirdTutorial::GliBirdTutorial()
    : LiveActor("グライバードチュートリアル"), mGliBird(nullptr), mPichan(nullptr), mPichanPos(0.0f, 0.0f, 0.0f), mPichanRot(0.0f, 0.0f, 0.0f),
      mGuidance(nullptr), mTalkCtrl(nullptr), mPadAccel(0.0f, 0.0f, 0.0f), mFrame(0), mTutorialPos(0.0f, 0.0f, 0.0f), mTimerCount(0) {
}

void GliBirdTutorial::init(const JMapInfoIter& rIter) {
    MR::connectToSceneMapObjMovement(this);
    mGuidance = new GliBirdGuidance("ガイダンス用レイアウト");
    mGuidance->initWithoutIter();
    MR::invalidateClipping(this);
    initNerve(GET_NERVE(GliBirdTutorial, GliBirdTutorialNrvTalkFirst), 0);
    initSound(4, "GliBirdTutorial", nullptr, TVec3f(0.0f));
    MR::tryFindNamePos(::sTutorialPosName, &mTutorialPos, nullptr);
    makeActorDead();
}

// NONMATCHING: frame-to-angle multiplication operands are commuted.
void GliBirdTutorial::movement() {
    MR::getCorePadAcceleration(&mPadAccel, WPAD_CHAN0);
    f32 sinValue = JMASinDegree(mFrame * 3.0f);
    TVec3f pos = mTutorialPos + (TVec3f(0.0f, 1.0f, 0.0f) * sinValue) * 30.0f;
    mFrame++;
    MR::setPosition(mGliBird, pos);
    LiveActor::movement();
}

void GliBirdTutorial::appear() {
    LiveActor::appear();
    MR::requestSupportTicoHide();
    MR::emitEffect(mGliBird, "GliBirdTutorialDust");
    MR::onGameEventFlagGliBirdTutorialAtFirst();
}

void GliBirdTutorial::kill() {
    LiveActor::kill();
    MR::requestSupportTicoShow();
    MR::deleteEffect(mGliBird, "GliBirdTutorialDust");
    MR::onGameEventFlagGliBirdTutorialAtFirst();
}

void GliBirdTutorial::registerPichan(LiveActor* pPichan) {
    mPichan = pPichan;
    mTalkCtrl = MR::createTalkCtrlDirect(mPichan, "GliBirdTutorial", TVec3f(0.0f, 0.0f, 0.0f), nullptr);
}

void GliBirdTutorial::exeEnd() {
    if (MR::isFirstStep(this)) {
        MR::closeWipeFade(-1);
    }

    if (!MR::isWipeActive()) {
        MR::resetPosition(mPichan, mPichanPos);
        MR::setRotation(mPichan, mPichanRot);
        mGuidance->kill();
        kill();
    }
}

void GliBirdTutorial::exeTalkFirst() {
    if (MR::isFirstStep(this)) {
        MR::startGliderBGM();
        MR::resetAndForwardNode(mTalkCtrl, 0);
        mGliBird->mIsTutorial = true;
        mGliBird->showBird();
        mPichanPos.set(mPichan->mPosition);
        mPichanRot.set(mPichan->mRotation);
        TVec3f pos(0.0f, 0.0f, 0.0f);
        MR::findNamePos(::sTutorialPosName, &pos, nullptr);
        pos.add(::sPichanOffset);
        MR::resetPosition(mPichan, pos);
        MR::setRotation(mPichan, ::sPichanRot);
        MR::setPlayerPos(::sTutorialPosName);
        MR::startAction(mPichan, "Talk");
    }

    if (MR::tryTalkForceWithoutDemoMarioPuppetableAtEnd(mTalkCtrl)) {
        setNerve(GET_NERVE(GliBirdTutorial, GliBirdTutorialNrvTalkStraight));
        mGliBird->mIsTutorial = false;
    }
}

void GliBirdTutorial::exeTalkStraight() {
    if (MR::isFirstStep(this)) {
        MR::resetAndForwardNode(mTalkCtrl, 1);
        mGuidance->appear();
        MR::startAction(mPichan, "Talk");
    }

    MR::tryTalkForceWithoutDemoMarioPuppetable(mTalkCtrl);

    if (MR::isStep(this, 120)) {
        setNerve(GET_NERVE(GliBirdTutorial, GliBirdTutorialNrvStraightStart));
    }
}

void GliBirdTutorial::exeStraightStart() {
    if (MR::isFirstStep(this)) {
        mGuidance->start(0);
    }

    MR::tryTalkForceWithoutDemoMarioPuppetable(mTalkCtrl);

    if (isOkStraight()) {
        setNerve(GET_NERVE(GliBirdTutorial, GliBirdTutorialNrvStraightHold));
    }
}

void GliBirdTutorial::exeStraightHold() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "OK", -1, -1, -1);
        mGuidance->hold(0);
    }

    startTimerSound(getNerveStep(), mTimerCount);

    if (MR::isStep(this, 120)) {
        MR::startActionSound(this, "GoNext", mTimerCount, -1, -1);
        mTimerCount++;
        setNerve(GET_NERVE(GliBirdTutorial, GliBirdTutorialNrvStraightSuccess));
        return;
    }

    MR::tryTalkForceWithoutDemoMarioPuppetable(mTalkCtrl);

    if (!isOkStraight()) {
        MR::startActionSound(this, "NG", -1, -1, -1);
        setNerve(GET_NERVE(GliBirdTutorial, GliBirdTutorialNrvStraightStart));
    }
}

void GliBirdTutorial::exeStraightSuccess() {
    if (MR::isFirstStep(this)) {
        mGuidance->success(0);
        MR::startAction(mPichan, "TalkSmile");
    }

    if (!MR::isLessStep(this, 30)) {
        if (MR::isStep(this, 30)) {
            MR::resetAndForwardNode(mTalkCtrl, 2);
        }

        if (MR::tryTalkForceWithoutDemoMarioPuppetableAtEnd(mTalkCtrl)) {
            setNerve(GET_NERVE(GliBirdTutorial, GliBirdTutorialNrvTalkTurnLeft));
        }
    }
}

void GliBirdTutorial::exeTalkTurnLeft() {
    if (MR::isFirstStep(this)) {
        MR::resetAndForwardNode(mTalkCtrl, 3);
        MR::startAction(mPichan, "Talk");
    }

    MR::tryTalkForceWithoutDemoMarioPuppetable(mTalkCtrl);

    if (MR::isStep(this, 120)) {
        setNerve(GET_NERVE(GliBirdTutorial, GliBirdTutorialNrvTurnLeftStart));
    }
}

void GliBirdTutorial::exeTurnLeftStart() {
    if (MR::isFirstStep(this)) {
        mGuidance->start(1);
    }

    MR::tryTalkForceWithoutDemoMarioPuppetable(mTalkCtrl);

    if (isOkTurnLeft()) {
        setNerve(GET_NERVE(GliBirdTutorial, GliBirdTutorialNrvTurnLeftHold));
    }
}

void GliBirdTutorial::exeTurnLeftHold() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "OK", -1, -1, -1);
        mGuidance->hold(1);
    }

    startTimerSound(getNerveStep(), mTimerCount);

    if (MR::isStep(this, 120)) {
        MR::startActionSound(this, "GoNext", mTimerCount, -1, -1);
        mTimerCount++;
        setNerve(GET_NERVE(GliBirdTutorial, GliBirdTutorialNrvTurnLeftSuccess));
        return;
    }

    MR::tryTalkForceWithoutDemoMarioPuppetable(mTalkCtrl);

    if (!isOkTurnLeft()) {
        MR::startActionSound(this, "NG", -1, -1, -1);
        setNerve(GET_NERVE(GliBirdTutorial, GliBirdTutorialNrvTurnLeftStart));
    }
}

void GliBirdTutorial::exeTurnLeftSuccess() {
    if (MR::isFirstStep(this)) {
        mGuidance->success(1);
        MR::startAction(mPichan, "TalkSmile");
    }

    if (!MR::isLessStep(this, 30)) {
        if (MR::isStep(this, 30)) {
            MR::resetAndForwardNode(mTalkCtrl, 4);
        }

        if (MR::tryTalkForceWithoutDemoMarioPuppetableAtEnd(mTalkCtrl)) {
            setNerve(GET_NERVE(GliBirdTutorial, GliBirdTutorialNrvTalkTurnRight));
        }
    }
}

void GliBirdTutorial::exeTalkTurnRight() {
    if (MR::isFirstStep(this)) {
        MR::resetAndForwardNode(mTalkCtrl, 5);
        MR::startAction(mPichan, "Talk");
    }

    MR::tryTalkForceWithoutDemoMarioPuppetable(mTalkCtrl);

    if (MR::isStep(this, 120)) {
        setNerve(GET_NERVE(GliBirdTutorial, GliBirdTutorialNrvTurnRightStart));
    }
}

void GliBirdTutorial::exeTurnRightStart() {
    if (MR::isFirstStep(this)) {
        mGuidance->start(2);
    }

    MR::tryTalkForceWithoutDemoMarioPuppetable(mTalkCtrl);

    if (isOkTurnRight()) {
        setNerve(GET_NERVE(GliBirdTutorial, GliBirdTutorialNrvTurnRightHold));
    }
}

void GliBirdTutorial::exeTurnRightHold() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "OK", -1, -1, -1);
        mGuidance->hold(2);
    }

    startTimerSound(getNerveStep(), mTimerCount);

    if (MR::isStep(this, 120)) {
        MR::startActionSound(this, "GoNext2", mTimerCount, -1, -1);
        mTimerCount++;
        setNerve(GET_NERVE(GliBirdTutorial, GliBirdTutorialNrvTurnRightSuccess));
        return;
    }

    MR::tryTalkForceWithoutDemoMarioPuppetable(mTalkCtrl);

    if (!isOkTurnRight()) {
        MR::startActionSound(this, "NG", -1, -1, -1);
        setNerve(GET_NERVE(GliBirdTutorial, GliBirdTutorialNrvTurnRightStart));
    }
}

void GliBirdTutorial::exeTurnRightSuccess() {
    if (MR::isFirstStep(this)) {
        mGuidance->success(2);
        MR::startAction(mPichan, "TalkSmile");
    }

    if (!MR::isLessStep(this, 30)) {
        if (MR::isStep(this, 30)) {
            MR::resetAndForwardNode(mTalkCtrl, 6);
        }

        if (MR::tryTalkForceWithoutDemoMarioPuppetableAtEnd(mTalkCtrl)) {
            setNerve(GET_NERVE(GliBirdTutorial, GliBirdTutorialNrvEnd));
        }
    }
}

bool GliBirdTutorial::isOkStraight() const {
    bool result = false;
    if (MR::abs(mPadAccel.x) < 0.25f && MR::abs(mPadAccel.y) < 0.45f) {
        result = true;
    }

    return result;
}

bool GliBirdTutorial::isOkTurnLeft() const {
    bool result = false;
    if (mPadAccel.x < -0.65f && MR::abs(mPadAccel.y) < 0.5f) {
        result = true;
    }

    return result;
}

bool GliBirdTutorial::isOkTurnRight() const {
    bool result = false;
    if (MR::abs(mPadAccel.x) < 0.6f && 0.05f < mPadAccel.y) {
        result = true;
    }

    return result;
}

void GliBirdTutorial::startTimerSound(s32 step, s32 count) {
    if (step == 30) {
        MR::startActionSound(this, "TutorialTimer", count, -1, -1);
    }

    if (step == 60) {
        MR::startActionSound(this, "TutorialTimer2", count, -1, -1);
    }

    if (step == 90) {
        MR::startActionSound(this, "TutorialTimer3", count, -1, -1);
    }
}

namespace MR {
    void registerGliBirdForTutorial(GliBird* pGliBird) {
        MR::createSceneObj(SceneObj_GliBirdTutorial);
        MR::getSceneObj< GliBirdTutorial >(SceneObj_GliBirdTutorial)->mGliBird = pGliBird;
    }

    void registerPichanForTutorial(Pichan* pPichan) {
        MR::createSceneObj(SceneObj_GliBirdTutorial);
        MR::getSceneObj< GliBirdTutorial >(SceneObj_GliBirdTutorial)->registerPichan(pPichan);
    }

    void startGliBirdTutorialTalk() {
        MR::getSceneObj< GliBirdTutorial >(SceneObj_GliBirdTutorial)->appear();
    }

    bool isEndGliBirdTutorialTalk() {
        return MR::isDead(MR::getSceneObj< GliBirdTutorial >(SceneObj_GliBirdTutorial));
    }
};  // namespace MR
