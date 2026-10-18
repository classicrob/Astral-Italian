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

TalkState::TalkState() : _04(), mBalloon() {
}

void TalkState::init(TalkMessageCtrl* pArg1, TalkBalloon* pBalloon) {
    _04 = pArg1;
    mBalloon = pBalloon;
    mMessageID = _04->getMessageID();
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

    if (pArg1 != _04) {
        return true;
    }

    return _04->getMessageID() != mMessageID;
};

bool TalkState::isSelfInterrupt(const TalkMessageCtrl* pArg1) const {
    if (pArg1 == nullptr) {
        return false;
    }

    if (pArg1 != _04) {
        return false;
    }

    return _04->getMessageID() != mMessageID;
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
    mBalloon->open(_04);
    LiveActor* actor = _04->mHostActor;
    TalkMessageInfo* info = TalkFunction::getMessageInfo(_04);
    MR::startTalkSound(info->_6, actor);
}

void TalkStateShort::clos() {
    mBalloon->close();
}

TalkStateEvent::TalkStateEvent() {
    _18 = nullptr;
    _14 = nullptr;
    _10 = nullptr;
    mTalkSoundDelay = 0;
    _1D = true;
}

bool TalkStateEvent::prep(const TalkMessageCtrl* pArg1) {
    return !TalkState::isLostMessage(pArg1);
}

bool TalkStateEvent::test() {
    return _18->isEnableTalkPlayerStateEvent();
}

void TalkStateEvent::open() {
    TalkMessageInfo* info=TalkFunction::getMessageInfo(_04);
    _1C=1;
    mPageCount=0;
    _10=0;
    mTalkSoundDelay=0;
    if(!MR::isTimeKeepDemoActive() || !info->isCameraNormal()) _04->startCamera(-1);
    if(!TalkFunction::isSkipEventTalkBalloon()) mBalloon->open(_04);
}

void TalkStateEvent::clos() {
    if(!TalkFunction::isSkipEventTalkBalloon()) mBalloon->close();
}

bool TalkStateEvent::talk(const TalkMessageCtrl* pArg1) {
    if(TalkFunction::isSkipEventTalkBalloon()) return true;
    if(mTalkSoundDelay<12) ++mTalkSoundDelay;
    else if(mTalkSoundDelay==12) {
        TalkMessageInfo* info=TalkFunction::getMessageInfo(_04);
        MR::startTalkSound(info->_6,_04->mHostActor);
        ++mTalkSoundDelay;
    }
    if (MR::testCorePadButtonA(0)) {
        if (_10++ > 10) {
            for (int i = 0; i < 5; i++) {
                mBalloon->skipMessage();
            }
        }
    } else {
        _10 = 0;
    }

    if (!mBalloon->isTextAppearedAll()) {
        return false;
    }

    bool unknownBool = false;

    if (_1C != nullptr) {
        _1C = MR::testCorePadButtonA(0);
    } else if (MR::testCorePadButtonA(0) && !MR::testCorePadTriggerA(0)) {
        _1C = true;
        unknownBool = true;
    }

    if (TalkFunction::isSelectTalk(_04) && !mBalloon->hasNextPage()) {
        return true;
    }

    if (unknownBool) {
        mPageCount++;
        return !mBalloon->turnPage();
    }

    return false;
}

bool TalkStateEvent::term(const TalkMessageCtrl* pArg1) {
    if (_14++ > 30) {
        _14 = 0;
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
    if (!_18->isEnableTalkPlayerStateNormal() || !MR::testCorePadTriggerA(0)) {
        return false;
    }

    return TalkStateEvent::test();
}

void TalkStateNormal::clos() {
    mAButton->term();
    TalkStateEvent::clos();
}

bool TalkStateNormal::term(const TalkMessageCtrl* pArg1) {
    if (pArg1 != nullptr && pArg1 != _04) {
        return true;
    }

    if (pArg1 != nullptr && TalkFunction::isShortTalk(pArg1)) {
        if(_14++>5) { _14=0; return true; }
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

    if (!_18->isEnableTalkPlayerStateNormal()) {
        mAButton->term();
    } else if (!MR::testCorePadTriggerA(0)) {
        TalkMessageInfo* info = TalkFunction::getMessageInfo(_04);
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

    f64 absolute = __fabs(camZ.dot(up));  // TODO: look at fabs again
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
    _00 = new TalkSupportPlayerWatcher();

    mAButton = new IconAButton(true, false);
    mAButton->initWithoutIter();

    mTalkShort = new TalkStateShort();

    mTalkNormal = new TalkStateNormal();
    mTalkNormal->mAButton = mAButton;
    mTalkNormal->_18 = _00;

    mTalkEvent = new TalkStateEvent();
    mTalkEvent->_18 = _00;

    mTalk = new TalkState();
}

void TalkStateHolder::update() {
    _00->update();
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
