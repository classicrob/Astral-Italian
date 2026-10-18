#include "Game/NPC/TalkBalloon.hpp"
#include "Game/Scene/GameSceneFunction.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/NPC/TalkMessageCtrl.hpp"
#include "Game/NPC/TalkMessageInfo.hpp"
#include "Game/NPC/TalkState.hpp"
#include "Game/NPC/TalkTextFormer.hpp"
#include "Game/Screen/IconAButton.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MessageUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include <JSystem/JGeometry/TVec.hpp>
#include <JSystem/JMath/JMath.hpp>
#include <JSystem/JUtility/JUTVideo.hpp>

namespace NrvTalkBalloonShort {
    NEW_NERVE(TalkBalloonShortNrvOpen, TalkBalloonShort, Open);

    // SMG2 Talk nerve: execute() is just a virtual updateTalking() call (no exeTalk symbol in the target).
    class TalkBalloonShortNrvTalk : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            TalkBalloonShort* actor = reinterpret_cast< TalkBalloonShort* >(pSpine->mExecutor);
            actor->updateTalking();
        };
        static TalkBalloonShortNrvTalk sInstance;
    };
    TalkBalloonShortNrvTalk TalkBalloonShortNrvTalk::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(TalkBalloonShortNrvClose, TalkBalloonShort, Close);
};  // namespace NrvTalkBalloonShort

namespace NrvTalkBalloonEvent {
    NEW_NERVE(TalkBalloonEventNrvWait, TalkBalloonEvent, Wait);
    NEW_NERVE(TalkBalloonEventNrvOpen, TalkBalloonEvent, Open);
    NEW_NERVE(TalkBalloonEventNrvTalk, TalkBalloonEvent, Talk);
    NEW_NERVE(TalkBalloonEventNrvClose, TalkBalloonEvent, Close);
};  // namespace NrvTalkBalloonEvent

namespace NrvTalkBalloonSign {
    NEW_NERVE(TalkBalloonSignNrvNextPage, TalkBalloonSign, NextPage);
}

TalkBalloon::TalkBalloon(const char* pName) : LayoutActor(pName, true), mMessageCtrl(), mTextFormer(), _28(), _29(), _36(0xFF) {
}

void TalkBalloon::create(const char* pLayoutName, bool arg2, bool isTalkLayout) {
    _28 = arg2;

    if (isTalkLayout) {
        MR::connectToSceneTalkLayout(this);
    } else {
        MR::connectToSceneLayout(this);
    }

    if (pLayoutName != nullptr) {
        initLayoutManager(pLayoutName, 2);

        if (_28) {
            MR::createAndAddPaneCtrl(this, "Balloon", 1);
        }

        mTextFormer = new TalkTextFormer(this, "Text00");
        MR::hideScreen(this);
    }
}

void TalkBalloon::init(const JMapInfoIter& rIter) {
    kill();
}

void TalkBalloon::kill() {
    LayoutActor::kill();
    mMessageCtrl = nullptr;
}

void TalkBalloon::open(TalkMessageCtrl* pCtrl) {
    mMessageCtrl = pCtrl;
    MR::startAnim(this, "Appear", 0);
    // Host actor named "モック" (Shift-JIS bytes) clears the flag at 0x36, which enables the talk SE in updateTalking.
    if (MR::isEqualString(mMessageCtrl->mHostActor->getName(), "\x83\x82\x83\x62\x83\x4E")) {
        _36 = 0;
    } else {
        _36 = 0xFF;
    }

    if (_28) {
        MR::startPaneAnim(this, "Balloon", "Beak", 0);
    }
}

void TalkBalloon::close() {
    MR::startAnim(this, "End", 0);
}

inline f32 fmin(f32 a, f32 b) {
    return b >= a ? a : b;
}

inline f32 fmax(f32 a, f32 b) {
    return b >= a ? b : a;
}

// NONMATCHING: equivalent multiplication operands are allocated in reverse order.
void TalkBalloon::updateBalloon() {
    mMessageCtrl->updateBalloonPos();
    if (_28) {
        TVec2f paneTrans;
        MR::copyPaneTrans(&paneTrans, this, "PicBeak");

        TVec2f position(mMessageCtrl->_1C.x, mMessageCtrl->_1C.y);
        if (mMessageCtrl->_SMG2_52) {
            f32 halfHeight = 0.5f * MR::getScreenHeight();
            position.set(0.5f * MR::getScreenWidth(), halfHeight);
        }
        TVec2f v2(position.x - paneTrans.x, position.y - paneTrans.y);
        TVec2f v3(0.0f, 1.0f);

        MR::normalizeOrZero(&v2);

        f32 cosine = MR::acos(v2.x * v3.x + v2.y * v3.y) * _180_PI;

        f32 paneAnimFrameMax = MR::getPaneAnimFrameMax(this, "Balloon", 0);
        f32 paneAnimFrame = MR::getPaneAnimFrame(this, "Balloon", 0);

        if (v2.x * v3.y - v2.y * v3.x > 0.0f) {
            f32 halfMax = paneAnimFrameMax;
            halfMax *= 0.5f;
            paneAnimFrameMax = fmin(paneAnimFrameMax, halfMax + cosine);
        } else {
            f32 half = 0.5f;
            paneAnimFrameMax = fmax(0.0f, paneAnimFrameMax * half - cosine);
        }

        if (!_29) {
            if (MR::fabs(paneAnimFrame - paneAnimFrameMax) < 10.0f) {
                return;
            }

            _29 = true;
        }

        if (paneAnimFrame > paneAnimFrameMax) {
            paneAnimFrameMax += 1.0f;
        } else {
            paneAnimFrameMax -= 1.0f;
        }

        if (MR::abs(paneAnimFrame - paneAnimFrameMax) < 2.0f) {
            _29 = false;
        }

        MR::setPaneAnimFrameAndStop(this, "Balloon", paneAnimFrameMax, 0);
    }
}

void TalkBalloon::updateTalking() {
    updateBalloon();
    if (!isTextAppearedAll() && _36 == 0) {
        MR::startSystemLevelSE("SE_SY_LV_TALK_MESSAGE", -1, -1);
    }
    mTextFormer->updateTalking();
}

bool TalkBalloon::isTextAppearedAll() {
    return mTextFormer->isTextAppearedAll();
}

bool TalkBalloon::turnPage() {
    return false;
}

bool TalkBalloon::hasNextPage() {
    return mTextFormer->hasNextPage();
}

void TalkBalloon::skipMessage() {
}

void TalkBalloon::pauseOff() {
    MR::requestMovementOn(this);
}

TalkBalloonShort::TalkBalloonShort(const char* pName) : TalkBalloon(pName) {
    initNerve(GET_NERVE(TalkBalloonShort, TalkBalloonShortNrvOpen));
}

void TalkBalloonShort::init(const JMapInfoIter& rIter) {
    TalkBalloon::create("TalkBalloonStretch", true, false);
}

// NONMATCHING: message and formatter use swapped nonvolatile registers.
void TalkBalloonShort::open(TalkMessageCtrl* pCtrl) {
    TalkBalloon::open(pCtrl);
    MR::showScreen(this);
    LayoutActor::appear();
    TalkTextFormer* tempFormer = mTextFormer;

    const wchar_t* message = TalkFunction::getMessage(pCtrl);

    tempFormer->formMessage(message, 0);
    tempFormer->setArg(pCtrl->mTagArg, 0);

    u32 numLine = MR::countMessageLine(message);

    switch (numLine) {
    case 1:
        MR::startAnim(this, "OneLine", 1);
        break;

    case 2:
        MR::startAnim(this, "TwoLine", 1);
        break;

    default:
        MR::startAnim(this, "TwoLine", 1);
        break;
    }

    MR::setAnimFrameAndStopAdjustTextWidth(this, "TxtText", 1);
    LayoutActor::setNerve(GET_NERVE(TalkBalloonShort, TalkBalloonShortNrvOpen));
}

void TalkBalloonShort::close() {
    TalkBalloon::close();
    setNerve(GET_NERVE(TalkBalloonShort, TalkBalloonShortNrvClose));
}

void TalkBalloonShort::updateBalloon() {
    TalkBalloon::updateBalloon();
    if (mMessageCtrl->_1C.z > 0.0f) {
        MR::hideScreen(this);
    } else {
        MR::showScreen(this);
    }

    setTrans(TVec2f(mMessageCtrl->_1C.x, mMessageCtrl->_1C.y));
}

void TalkBalloonShort::exeClose() {
    updateBalloon();
    if (MR::isAnimStopped(this, 0)) {
        kill();
    }
}

void TalkBalloonShort::exeOpen() {
    updateTalking();
    if (MR::isAnimStopped(this, 0)) {
        setNerve(GET_NERVE(TalkBalloonShort, TalkBalloonShortNrvTalk));
    }
}

TalkBalloonEvent::TalkBalloonEvent(const char* pName) : TalkBalloon(pName), _2C(1), mAButton() {
    initNerve(GET_NERVE(TalkBalloonEvent, TalkBalloonEventNrvWait));
}

void TalkBalloonEvent::init(const JMapInfoIter& rIter) {
    create("TalkBalloonEvent", 1, 1);
    MR::createAndAddPaneCtrl(this, "AButtonPosition", 1);
}

void TalkBalloonEvent::open(TalkMessageCtrl* pCtrl) {
    mMessageCtrl = pCtrl;
    LayoutActor::appear();
    TalkMessageInfo* info = TalkFunction::getMessageInfo(pCtrl);
    mTextFormer->formMessage(reinterpret_cast< const wchar_t* >(info->_0), _2C);
    mTextFormer->setArg(pCtrl->mTagArg, 0);
    mAButton->kill();
    mAButton->setFollowActorPane(this, "AButtonPosition");
    updateBalloon();
    LayoutActor::setNerve(GET_NERVE(TalkBalloonEvent, TalkBalloonEventNrvWait));
}

void TalkBalloonEvent::close() {
    MR::startAnim(this, "End", 0);

    if (!TalkFunction::isSelectTalk(mMessageCtrl)) {
        MR::startSystemSE("SE_SY_TALK_OK");
    }

    if (!MR::isDead(mAButton)) {
        mAButton->term();
    }

    MR::recoverSoundVolumeSetting(30);
    MR::moveVolumeStageBGM(1.0f, 60);
    MR::moveVolumeSubBGM(1.0f, 60);
    LayoutActor::setNerve(GET_NERVE(TalkBalloonEvent, TalkBalloonEventNrvClose));
}

void TalkBalloonEvent::updateBeak() {
    if (!_28) {
        return;
    }

    mMessageCtrl->updateBalloonPos();
    TalkMessageCtrl* messageCtrl = mMessageCtrl;
    if (messageCtrl->_SMG2_51) {
        MR::hidePane(this, "ShaBeak");
        MR::hidePane(this, "PicBeak");
        return;
    }
    TVec2f v1(messageCtrl->_1C.x, messageCtrl->_1C.y);

    if (0.0f <= v1.x && v1.x < MR::getScreenWidth() && 120.0f <= v1.y &&
        v1.y < MR::getScreenHeight()) {
        MR::showPane(this, "ShaBeak");
        MR::showPane(this, "PicBeak");
    } else {
        MR::hidePane(this, "ShaBeak");
        MR::hidePane(this, "PicBeak");
    }
}

bool TalkBalloonEvent::turnPage() {
    if (mTextFormer->nextPage()) {
        mTextFormer->setArg(mMessageCtrl->mTagArg, 0);
        MR::startSystemSE("SE_SY_TALK_FOCUS_ITEM");
        mAButton->term();

        return true;
    }

    return false;
}

void TalkBalloonEvent::skipMessage() {
    if (LayoutActor::isNerve(GET_NERVE(TalkBalloonEvent, TalkBalloonEventNrvTalk))) {
        mTextFormer->updateTalking();
    }
}

bool TalkBalloonEvent::isTextAppearedAll() {
    return mTextFormer->isTextAppearedAll() && LayoutActor::isNerve(GET_NERVE(TalkBalloonEvent, TalkBalloonEventNrvTalk));
}

void TalkBalloonEvent::exeWait() {
    if (MR::isTimeKeepDemoActive()) {
        LayoutActor::setNerve(GET_NERVE(TalkBalloonEvent, TalkBalloonEventNrvOpen));
    } else if (!MR::isFirstStep(this) && (MR::isCameraInterpolatingNearlyEnd() || GameSceneFunction::isExecSuperDreamerTalk())) {
        LayoutActor::setNerve(GET_NERVE(TalkBalloonEvent, TalkBalloonEventNrvOpen));
    }
}

void TalkBalloonEvent::exeOpen() {
    if (MR::isFirstStep(this)) {
        updateBeak();
        MR::showScreen(this);
        TalkBalloon::open(mMessageCtrl);
        MR::startSystemSE("SE_SM_TALKBLN_OPEN");
        MR::setSoundVolumeSetting(3, 30);
    }

    updateBalloon();

    if (MR::isAnimStopped(this, 0)) {
        MR::startAnim(this, "Wait", 0);
        LayoutActor::setNerve(GET_NERVE(TalkBalloonEvent, TalkBalloonEventNrvTalk));
    }
}

void TalkBalloonEvent::exeTalk() {
    updateTalking();
    if (mTextFormer->isTextAppearedAll() && MR::isDead(mAButton) && !TalkFunction::getMessageInfo(mMessageCtrl)->isBalloonCall()) {
        if (!TalkFunction::isSelectTalk(mMessageCtrl) || hasNextPage()) {
            mAButton->openWithoutMessage();
        }
    }
}

void TalkBalloonEvent::exeClose() {
    if (MR::isAnimStopped(this, 0)) {
        kill();
    }
}

TalkBalloonSign::TalkBalloonSign(const char* pName) : TalkBalloonEvent(pName) {
    _2C = 2;
}

void TalkBalloonSign::init(const JMapInfoIter& rIter) {
    TalkBalloon::create("SignBoardBalloon", false, true);
    MR::createAndAddPaneCtrl(this, "AButtonPosition", 1);
}

bool TalkBalloonSign::turnPage() {
    setNerve(&NrvTalkBalloonSign::TalkBalloonSignNrvNextPage::sInstance);
    return hasNextPage();
}

void TalkBalloonSign::exeNextPage() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(this, "NextPage", 0);
        if (hasNextPage()) {
            MR::startSystemSE("SE_SY_TALK_FOCUS_ITEM");
        }
        mAButton->term();
    }
    if (MR::isStep(this, 20)) {
        mTextFormer->nextPage();
        mTextFormer->setArg(mMessageCtrl->mTagArg, 0);
        setNerve(&NrvTalkBalloonEvent::TalkBalloonEventNrvTalk::sInstance);
    }
}

TalkBalloonInfo::TalkBalloonInfo(const char* pName) : TalkBalloon(pName) {
}

void TalkBalloonInfo::open(TalkMessageCtrl* pCtrl) {
    TalkMessageInfo* info = TalkFunction::getMessageInfo(pCtrl);
    MR::appearInformationMessage(reinterpret_cast< const wchar_t* >(info->_0), true);
}

void TalkBalloonInfo::close() {
    MR::startSystemSE("SE_SY_TALK_OK");
    MR::disappearInformationMessage();
}

TalkBalloonIcon::TalkBalloonIcon(const char* pName) : TalkBalloonShort(pName) {
}

void TalkBalloonIcon::init(const JMapInfoIter& rIter) {
    TalkBalloon::create("TalkBalloonLetter", false, false);
}

void TalkBalloonIcon::open(TalkMessageCtrl* pCtrl) {
    TalkBalloon::open(pCtrl);
    MR::showScreen(this);
    LayoutActor::appear();

    const wchar_t* message = TalkFunction::getMessage(pCtrl);
    TalkTextFormer* former = mTextFormer;

    former->formMessage(message, 2);
    former->setArg(pCtrl->mTagArg, 0);
    LayoutActor::setNerve(GET_NERVE(TalkBalloonShort, TalkBalloonShortNrvOpen));
}

TalkBalloonHolder::TalkBalloonHolder() : mShortBalloonIndex() {
    mBalloonShortArray = new TalkBalloonShort*[4];

    for (u32 i = 0; i < 4; i++) {
        TalkBalloonShort* temp = new TalkBalloonShort("会話吹き出し[簡易会話]");
        mBalloonShortArray[i] = temp;
        mBalloonShortArray[i]->initWithoutIter();
        mBalloonShortArray[i]->kill();
    }

    mBalloonEvent = new TalkBalloonEvent("会話吹き出し[イベント]");
    mBalloonEvent->initWithoutIter();
    mBalloonEvent->kill();
    mBalloonSign = new TalkBalloonSign("会話吹き出し[看板]");
    mBalloonSign->initWithoutIter();
    mBalloonSign->kill();
    mBalloonIcon = new TalkBalloonIcon("会話吹き出し[アイコン]");
    mBalloonIcon->initWithoutIter();
    mBalloonIcon->kill();

    mAButton = new IconAButton(true, false);
    mAButton->initWithoutIter();

    mBalloonEvent->mAButton = mAButton;
    mBalloonSign->mAButton = mAButton;
}

TalkBalloon* TalkBalloonHolder::getBalloon(const TalkMessageCtrl* ctrl) {
    TalkBalloon* balloon;
    const TalkMessageInfo* info = TalkFunction::getMessageInfo(ctrl);
    if (info->isNormalTalk()) {
        balloon = mBalloonEvent;
    } else if (info->isShortTalk()) {
        balloon = mBalloonShortArray[mShortBalloonIndex];
    } else if (info->isEventTalk()) {
        balloon = mBalloonEvent;
    } else {
        balloon = nullptr;
    }
    if (info->isBalloonSign()) {
        balloon = mBalloonSign;
    } else if (info->isBalloonCall()) {
        balloon = mBalloonEvent;
    } else if (info->isBalloonIcon()) {
        balloon = mBalloonIcon;
    }
    if (balloon == mBalloonShortArray[mShortBalloonIndex]) {
        mShortBalloonIndex = (mShortBalloonIndex + 1) % 4;
    }
    return balloon;
}

void TalkBalloonHolder::pauseOff() {
    mBalloonEvent->pauseOff();
    mBalloonSign->pauseOff();
    mBalloonIcon->pauseOff();

    for (int i = 0; i < 4; i++) {
        mBalloonShortArray[i]->pauseOff();
    }

    MR::requestMovementOn(mAButton);
}

void TalkBalloonHolder::balloonOff() {
    mAButton->kill();
}

void TalkBalloonHolder::update() {
}

bool TalkBalloonHolder::isActiveBalloonShort() const {
    for (int i = 0; i < 4; i++) {
        if (!MR::isDead(mBalloonShortArray[i])) {
            return true;
        }
    }

    return false;
}
