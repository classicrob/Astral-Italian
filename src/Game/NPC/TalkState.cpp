// NONMATCHING: updateButton register allocation and stack layout; other residuals are literal aliases.
#include "Game/NPC/TalkState.hpp"
#include "Game/Scene/GameSceneFunction.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/NPC/TalkBalloon.hpp"
#include "Game/NPC/TalkMessageCtrl.hpp"
#include "Game/NPC/TalkMessageInfo.hpp"
#include "Game/NPC/TalkSupportPlayerWatcher.hpp"
#include "Game/Screen/IconAButton.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

TalkState::TalkState() : mTalkCtrl(), mBalloon() {
}

void TalkState::init(TalkMessageCtrl* pArg1, TalkBalloon* pBalloon) {
    mTalkCtrl = pArg1;
    mBalloon = pBalloon;
    mMessageID = mTalkCtrl->getMessageID();
}

u32 TalkState::getPageCount() const {
    return 0;
}

bool TalkState::isBalloonDead() const {
    return MR::isDead(mBalloon);
}

bool TalkState::isLostMessage(const TalkMessageCtrl* pArg1) const {
    if (pArg1 == nullptr) {
        return true;
    }

    if (pArg1 != mTalkCtrl) {
        return true;
    }

    return mTalkCtrl->getMessageID() != mMessageID;
};

bool TalkState::isSelfInterrupt(const TalkMessageCtrl* pArg1) const {
    if (pArg1 == nullptr) {
        return false;
    }

    if (pArg1 != mTalkCtrl) {
        return false;
    }

    return mTalkCtrl->getMessageID() != mMessageID;
}

TalkStateShort::TalkStateShort() : TalkState() {
}

bool TalkStateShort::prep(const TalkMessageCtrl* pArg1) {
    return !TalkState::isLostMessage(pArg1);
}

bool TalkStateShort::talk(const TalkMessageCtrl*) {
    return false;
}

void TalkStateShort::open() {
    mBalloon->open(mTalkCtrl);
    LiveActor* actor = mTalkCtrl->mHostActor;
    TalkMessageInfo* info = TalkFunction::getMessageInfo(mTalkCtrl);
    MR::startTalkSound(info->_6, actor);
}

void TalkStateShort::clos() {
    mBalloon->close();
}

TalkStateEvent::TalkStateEvent() {
    mPlayerWatcher = nullptr;
    mCloseDelayFrames = 0;
    mHeldButtonFrames = 0;
    mTalkSoundDelay = 0;
    mUnusedFlag = true;
}

bool TalkStateEvent::prep(const TalkMessageCtrl* pArg1) {
    return !TalkState::isLostMessage(pArg1);
}

bool TalkStateEvent::test() {
    return mPlayerWatcher->isEnableTalkPlayerStateEvent();
}

void TalkStateEvent::open() {
    TalkMessageInfo* info=TalkFunction::getMessageInfo(mTalkCtrl);
    mIsAButtonHeld=1;
    mPageCount=0;
    mHeldButtonFrames=0;
    mTalkSoundDelay=0;
    if(!MR::isTimeKeepDemoActive() || !info->isCameraNormal()) mTalkCtrl->startCamera(-1);
    if(!TalkFunction::isSkipEventTalkBalloon()) mBalloon->open(mTalkCtrl);
}

void TalkStateEvent::clos() {
    if(!TalkFunction::isSkipEventTalkBalloon()) mBalloon->close();
}

bool TalkStateEvent::talk(const TalkMessageCtrl* pArg1) {
    if(TalkFunction::isSkipEventTalkBalloon()) return true;
    if(mTalkSoundDelay<12) ++mTalkSoundDelay;
    else if(mTalkSoundDelay==12) {
        TalkMessageInfo* info=TalkFunction::getMessageInfo(mTalkCtrl);
        MR::startTalkSound(info->_6,mTalkCtrl->mHostActor);
        ++mTalkSoundDelay;
    }
    if (MR::testCorePadButtonA(0)) {
        if (mHeldButtonFrames++ > 10) {
            for (int i = 0; i < 5; i++) {
                mBalloon->skipMessage();
            }
        }
    } else {
        mHeldButtonFrames = 0;
    }

    if (!mBalloon->isTextAppearedAll()) {
        return false;
    }

    bool advancePage = false;

    if (mIsAButtonHeld != 0) {
        mIsAButtonHeld = MR::testCorePadButtonA(0);
    } else if (MR::testCorePadButtonA(0) && !MR::testCorePadTriggerA(0)) {
        mIsAButtonHeld = true;
        advancePage = true;
    }

    if (TalkFunction::isSelectTalk(mTalkCtrl) && !mBalloon->hasNextPage()) {
        return true;
    }

    if (advancePage) {
        mPageCount++;
        return !mBalloon->turnPage();
    }

    return false;
}

bool TalkStateEvent::term(const TalkMessageCtrl* pArg1) {
    if (mCloseDelayFrames++ > 30) {
        mCloseDelayFrames = 0;
        return true;
    } else {
        return false;
    }
}

u32 TalkStateEvent::getPageCount() const {
    return mPageCount;
}

TalkStateNormal::TalkStateNormal() : TalkStateEvent() {
}

bool TalkStateNormal::test() {
    if (!mPlayerWatcher->isEnableTalkPlayerStateNormal() || !MR::testCorePadTriggerA(0)) {
        return false;
    }

    return TalkStateEvent::test();
}

void TalkStateNormal::clos() {
    mAButton->term();
    TalkStateEvent::clos();
}

bool TalkStateNormal::term(const TalkMessageCtrl* pArg1) {
    if (pArg1 != nullptr && pArg1 != mTalkCtrl) {
        return true;
    }

    if (pArg1 != nullptr && TalkFunction::isShortTalk(pArg1)) {
        if(mCloseDelayFrames++>5) { mCloseDelayFrames=0; return true; }
        return false;
    }

    return TalkStateEvent::term(pArg1);
}

bool TalkStateNormal::prep(const TalkMessageCtrl* pArg1) {
    if (TalkStateNormal::isLostMessage(pArg1)) {
        mAButton->term();
        return false;
    }

    if(MR::isDemoActive()) { mAButton->term(); return false; }
    TalkStateNormal::updateButton();

    if (!mPlayerWatcher->isEnableTalkPlayerStateNormal()) {
        mAButton->term();
    } else if (!MR::testCorePadTriggerA(0)) {
        TalkMessageInfo* info = TalkFunction::getMessageInfo(mTalkCtrl);
        if (!mAButton->isOpen()) {
            if (info->isBalloonSign() || info->_0==nullptr) {
                mAButton->openWithRead();
            } else {
                mAButton->openWithTalk();
            }

            MR::startSystemSE("SE_SM_TALK_BUTTON_APPEAR");
        }
    } else {
        mAButton->term();
        MR::startSystemSE("SE_SY_TALK_START");
        MR::startCSSound("CS_CLICK_OPEN", nullptr, 0, 0, 1.0f);
    }

    return true;
}

void TalkStateNormal::updateButton() {
    TVec3f camZ(MR::getCamZdir());
    TVec3f camY(MR::getCamYdir());
    TVec3f up;
    TVec3f centerPlayer(*MR::getPlayerCenterPos());
    MR::getPlayerUpVec(&up);

    f64 absolute = __fabs(camZ.dot(up));
    f32 f1 = absolute;
    f32 f2 = camY.dot(up);
    f1 = f1 * f1;

    MR::calcNormalizedScreenPosition(&up, up * 1000.0f + centerPlayer);
    MR::calcNormalizedScreenPosition(&centerPlayer, centerPlayer);
    MR::normalize(up - centerPlayer, &up);
    TVec2f playerScreenPos;
    MR::calcScreenPosition(&playerScreenPos, *MR::getPlayerCenterPos());

    playerScreenPos.add(TVec2f(0.0f, -1.0f) * (40.0f * f1));
    playerScreenPos.add(TVec2f(up.x, up.y) * (60.0f * f2));

    mAButton->setTrans(playerScreenPos);
}

TalkStateHolder::TalkStateHolder() {
    mPlayerWatcher = new TalkSupportPlayerWatcher();

    mAButton = new IconAButton(true, false);
    mAButton->initWithoutIter();

    mTalkShort = new TalkStateShort();

    mTalkNormal = new TalkStateNormal();
    mTalkNormal->mAButton = mAButton;
    mTalkNormal->mPlayerWatcher = mPlayerWatcher;

    mTalkEvent = new TalkStateEvent();
    mTalkEvent->mPlayerWatcher = mPlayerWatcher;

    mTalk = new TalkState();
}

void TalkStateHolder::update() {
    mPlayerWatcher->update();
}

void TalkStateHolder::pauseOff() {
    MR::requestMovementOn(mAButton);
}

TalkState* TalkStateHolder::getState(const TalkMessageCtrl* pArg1) {
    TalkMessageInfo* info = TalkFunction::getMessageInfo(pArg1);

    if (info->isNormalTalk()) {
        return mTalkNormal;
    }

    if (info->isShortTalk()) {
        return mTalkShort;
    }

    if (info->isEventTalk()) {
        return mTalkEvent;
    }

    return mTalk;
}

bool TalkFunction::isSkipEventTalkBalloon() {
    if(GameSceneFunction::isExecSuperDreamerTalk()) return false;
    if(MR::isDemoActive("ベリードラゴンデモ（クッパＪｒ登場）")) return GameSequenceFunction::isPadMode2();
    return false;
}
