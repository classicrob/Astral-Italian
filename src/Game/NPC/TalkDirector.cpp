// NONMATCHING: literal aliases and independent prepTalk store ordering.
#include "Game/NPC/TalkDirector.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/NPC/TalkBalloon.hpp"
#include "Game/NPC/TalkMessageCtrl.hpp"
#include "Game/NPC/TalkState.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Screen/GameSceneLayoutHolder.hpp"
#include "Game/Screen/YesNoLayout.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/DirectDraw.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/Util/TalkUtil.hpp"
#include <JSystem/JUtility/JUTVideo.hpp>
#include <cstdio>
#include <revolution/gx/GXGet.h>

namespace {
    TalkDirector* getTalkDirector() {
        return MR::getSceneObj< TalkDirector >(SceneObj_TalkDirector);
    }

    static const f32 sTalkDistanceScale = 1.0f;
    static const f32 sNearScale = 1.2f;
};  // namespace

namespace NrvTalkDirector {
    NEW_NERVE(TalkDirectorNrvWait, TalkDirector, Wait);
    NEW_NERVE(TalkDirectorNrvPrep, TalkDirector, Prep);
    NEW_NERVE(TalkDirectorNrvTalk, TalkDirector, Talk);
    NEW_NERVE(TalkDirectorNrvClose, TalkDirector, Close);
    NEW_NERVE(TalkDirectorNrvSlct, TalkDirector, Slct);
    NEW_NERVE(TalkDirectorNrvNext, TalkDirector, Next);
    NEW_NERVE(TalkDirectorNrvTerm, TalkDirector, Term);
};  // namespace NrvTalkDirector

TalkPeekZ::TalkPeekZ() : mDrawSync(DrawSyncManager::sInstance->setCallback(4, 1, this)) {
}

void TalkPeekZ::setDrawSyncToken() {
    GXGetProjectionv(&_20);
    GXGetViewportv(&_3C);
    _8 = 0;
    DrawSyncManager::sInstance->pushBreakPoint();
    GXSetDrawSync(mDrawSync);
}

void TalkPeekZ::drawSyncCallback(u16 arg) {
    if (!MR::isInRange(mScreenPos.x, 0.0f, MR::getScreenWidth() - 1) ||
        !MR::isInRange(mScreenPos.y, 0.0f, JUTVideo::getManager()->getEfbHeight() - 1)) {
        return;
    }

    TVec2f pos;
    MR::convertScreenPosToFrameBufferPos(&pos, mScreenPos);

    GXPeekZ(pos.x, pos.y, &_8);

    TDDraw::invProject(&_14, TVec3f(mScreenPos.x, mScreenPos.y, static_cast< f32 >(_8)), const_cast< MtxPtr >(MR::getCameraViewMtx().mMtx), &_20,
                       &_3C, false);
}

TalkDirector::TalkDirector(const char* pName)
    : LayoutActor(pName, true), mMsgCtrl(), _3C(), _40(), _44(), mTalkState(), _4C(), _4D(), _4E(), mIsInvalidClipping(), _50(), mDemoType(), _58(), _59() {
}

TalkDirector::~TalkDirector() {
}

void TalkDirector::init(const JMapInfoIter& rIter) {
    MR::connectToScene(this, MR::MovementType_TalkDirector, MR::CalcAnimType_None, MR::DrawBufferType_None, MR::DrawType_None);
    mBalloonHolder = new TalkBalloonHolder();
    mStateHolder = new TalkStateHolder();
    mMsgControls.init(128);
    mPeekZ = new TalkPeekZ();
    initNerve(GET_NERVE(TalkDirector, TalkDirectorNrvWait));
    appear();
}
void TalkDirector::movement() {
    updateMessage();
    if(_4C && (!MR::isNewNerve(this) || (!isNerve(GET_NERVE(TalkDirector,TalkDirectorNrvTalk)) && !isNerve(GET_NERVE(TalkDirector,TalkDirectorNrvNext))))) _4C=false;
    if(_4D && (!isNerve(GET_NERVE(TalkDirector,TalkDirectorNrvTerm)) || !MR::isNewNerve(this))) _4D=false;
    LayoutActor::movement();
}

bool TalkDirector::request(TalkMessageCtrl* pCtrl, bool force) {
    bool var31 = false;
    if (isInvalidTalk()) {
        return false;
    }

    if(!pCtrl->mHasMessage) return false;

    if (pCtrl->mIsOnRootNodeAuto) {
        if (TalkFunction::isShortTalk(pCtrl)) {
            if (!isNerve(GET_NERVE(TalkDirector, TalkDirectorNrvNext))) {
                pCtrl->rootNodePre(false);
            }
        } else if (!isNerve(GET_NERVE(TalkDirector, TalkDirectorNrvTalk)) && !isNerve(GET_NERVE(TalkDirector, TalkDirectorNrvClose)) && !isNerve(GET_NERVE(TalkDirector, TalkDirectorNrvNext))) {
            pCtrl->rootNodePre(false);
        }
    }

    if (!force && !TalkFunction::isShortTalk(pCtrl) && MR::isPlayerElementModeHopper()) {
        return false;
    }

    if ((isNerve(GET_NERVE(TalkDirector, TalkDirectorNrvTalk)) || isNerve(GET_NERVE(TalkDirector, TalkDirectorNrvClose))) && !TalkFunction::isShortTalk(mTalkState->mTalkCtrl)) {
        if (mTalkState->mTalkCtrl == pCtrl) {
            var31 = true;
        } else {
            return false;
        }
    }

    if (MR::isTalkNone(pCtrl)) {
        TalkFunction::onTalkStateEntry(pCtrl);
    }

    if(TalkFunction::isGlobalEventTalk(pCtrl)) force=true;

    if (force) {
        if (TalkFunction::isShortTalk(pCtrl)) {
            if (pCtrl->isNearPlayer(_40)) {
                _40 = pCtrl;
            }
        } else {
            _40 = pCtrl;
            _3C = pCtrl;
            mMsgCtrl = pCtrl;
        }
    } else {
        f32 scale = var31 ? ::sNearScale : ::sTalkDistanceScale;
        f32 talkDistance = pCtrl->mTalkDistance;

        if (!pCtrl->isNearPlayer(scale * talkDistance)) {
            return false;
        }

        if (pCtrl->isNearPlayer(_40)) {
            _40 = pCtrl;
        }
    }

    if (mMsgCtrl != pCtrl) {
        return false;
    }

    if (!isNerve(GET_NERVE(TalkDirector, TalkDirectorNrvWait))) {
        return mTalkState->mTalkCtrl == pCtrl;
    }

    mTalkState = initState(pCtrl);
    setNerve(GET_NERVE(TalkDirector, TalkDirectorNrvPrep));
    return true;
}

bool TalkDirector::test(TalkMessageCtrl* pCtrl, bool arg2, bool arg3) {
    if (isInvalidTalk()) {
        return false;
    }

    if (mMsgCtrl != pCtrl) {
        return false;
    }

    if (isNerve(GET_NERVE(TalkDirector, TalkDirectorNrvTalk)) || isNerve(GET_NERVE(TalkDirector, TalkDirectorNrvClose))) {
        return true;
    }

    if (!isNerve(GET_NERVE(TalkDirector, TalkDirectorNrvPrep))) {
        return false;
    }

    if (!arg2 && !mTalkState->test()) {
        return false;
    }

    if (getDemoType(pCtrl, arg3) == 1 && !MR::canStartDemo()) {
        return false;
    }

    return true;
}

bool TalkDirector::start(TalkMessageCtrl* pCtrl, bool arg2, bool arg3, bool arg4) {
    if(!test(pCtrl,arg2,arg3)) return false;
    if(isNerve(GET_NERVE(TalkDirector,TalkDirectorNrvTalk))) return true;
    if(isNerve(GET_NERVE(TalkDirector,TalkDirectorNrvClose))) return true;
    pCtrl->rootNodePre(true);
    prepTalk(pCtrl,arg2,arg3,arg4);
    if(TalkFunction::isEventNode(pCtrl)) {
        setNerve(GET_NERVE(TalkDirector,TalkDirectorNrvNext));
        return true;
    }
    mTalkState->open();
    setNerve(GET_NERVE(TalkDirector,TalkDirectorNrvTalk));
    return true;
}

void TalkDirector::updateMessage() {
    mBalloonHolder->update();
    mStateHolder->update();

    for (TalkMessageCtrl** pIter = mMsgControls.begin(); pIter != mMsgControls.end(); pIter++) {
        if (MR::isTalkEntry(*pIter)) {
            TalkFunction::onTalkStateNone(*pIter);
        }
    }

    if (isNerve(GET_NERVE(TalkDirector, TalkDirectorNrvWait)) || isNerve(GET_NERVE(TalkDirector, TalkDirectorNrvPrep))) {
        mMsgCtrl = _3C;
        _3C = _40;
        _40 = nullptr;
    } else {
        mMsgCtrl = _40;
        _3C = mMsgCtrl;
        _40 = nullptr;
    }

    if (mMsgCtrl != nullptr) {
        _44 = mMsgCtrl;
    }

    if (_3C != nullptr) {
        _3C->updateBalloonPos();

        TalkPeekZ* peek = mPeekZ;
        TVec2f v1(_3C->mBalloonScreenPosition.x, _3C->mBalloonScreenPosition.y);
        peek->mScreenPos.set(v1.x, v1.y);
    }

    if (MR::isPowerStarGetDemoActive()) {
        mMsgCtrl = nullptr;
        _4E = true;
    }
}

void TalkDirector::prepTalk(TalkMessageCtrl* pCtrl, bool arg2, bool arg3, bool arg4) {
    _58 = arg2;
    _4C = true;
    _59 = arg4;

    mDemoType = getDemoType(pCtrl, arg3);
    if(!arg2 && TalkFunction::isShortTalk(pCtrl)) mIsInvalidClipping=true;
    else mIsInvalidClipping = MR::isInvalidClipping(pCtrl->mHostActor);

    if (mDemoType == 0) {
        _59 = false;
    }

    MR::invalidateClipping(pCtrl->mHostActor);

    switch (mDemoType) {
    case 2:
        MR::pauseTimeKeepDemo(pCtrl->mHostActor);
        break;
    case 1:
        // "Discussion"
        if(pCtrl->mUseCinemaFrame) MR::tryStartDemoMarioPuppetable(pCtrl->mHostActor, "会話");
        else MR::tryStartDemoMarioPuppetableWithoutCinemaFrame(pCtrl->mHostActor, "会話");
        MR::requestMovementOn(this);
        mBalloonHolder->pauseOff();
        mStateHolder->pauseOff();
        break;
    }

    if (_59) {
        MR::startPlayerTalk(pCtrl->mHostActor);
    }
}

void TalkDirector::termTalk() {
    TalkMessageCtrl* control = mTalkState->mTalkCtrl;
    _4D = true;

    control->endCamera(mMessageInfo,-1);

    if (!mIsInvalidClipping) {
        MR::validateClipping(control->mHostActor);
    }

    if (_59) {
        MR::endPlayerTalk();
    }

    switch (mDemoType) {
    case 2:
        MR::resumeTimeKeepDemo(control->mHostActor);
        break;
    case 1:
        MR::endDemo(control->mHostActor, "会話");
        break;
    }

    mDemoType = 0;
    _59 = false;
    mIsInvalidClipping = false;
}

TalkState* TalkDirector::initState(TalkMessageCtrl* pCtrl) {
    TalkState* state = mStateHolder->getState(pCtrl);
    TalkBalloon* balloon = mBalloonHolder->getBalloon(pCtrl);
    state->init(pCtrl, balloon);

    TalkMessageInfo* info = TalkFunction::getMessageInfo(pCtrl);

    if (info->isCameraNormal() || info->isCameraEvent()) {
        info = TalkFunction::getMessageInfo(pCtrl);
        mMessageInfo = *info;
        mCameraInfo = pCtrl->mCameraInfo;
        mHostActor = pCtrl->mHostActor;
    }

    return state;
}

bool TalkDirector::isInvalidTalk() const {
    if (_4E) {
        return true;
    }

    if (MR::isStarPointerModeBlueStarReady()) {
        return true;
    }

    if (MR::isFirstPersonCamera()) {
        return true;
    }

    if(MR::isPlayerDead()) return true;
    return MR::getAreaObj("NoMessageArea",*MR::getPlayerCenterPos())!=nullptr;
}

void TalkDirector::appearYesNoSelector(const TalkMessageCtrl* pCtrl) const {
    const char* branchID = pCtrl->getBranchID();

    char buffYes[256];
    snprintf(buffYes, sizeof(buffYes), "Select_%s_Yes", branchID);

    char buffNo[256];
    snprintf(buffNo, sizeof(buffNo), "Select_%s_No", branchID);

    if (pCtrl->isSelectYesNo()) {
        MR::resetYesNoSelectorSE();
    } else {
        MR::setYesNoSelectorSE("SE_SY_TALK_FOCUS_ITEM", "SE_SY_TALK_SELECT", "SE_SY_TALK_SELECT");
    }

    MR::requestMovementOn(MR::getGameSceneLayoutHolder()->mYesNoLayout);
    MR::appearYesNoSelector(buffYes, buffNo);
}

s32 TalkDirector::getDemoType(const TalkMessageCtrl* pCtrl, bool arg2) const {
    s32 demoType;

    if (TalkFunction::isShortTalk(pCtrl)) {
        demoType = 0;
    } else if (MR::isTimeKeepDemoActive()) {
        if (arg2) {
            demoType = 2;
        } else {
            demoType = 3;
        }
    } else {
        demoType = 1;
    }

    if (!arg2 && demoType != 3) {
        return 0;
    }

    return demoType;
}

void TalkDirector::pauseOff() {
    MR::requestMovementOn(this);
    mBalloonHolder->pauseOff();
    mStateHolder->pauseOff();
}

bool TalkDirector::isSystemTalking() const {
    if (isNerve(GET_NERVE(TalkDirector, TalkDirectorNrvTalk)) || isNerve(GET_NERVE(TalkDirector, TalkDirectorNrvClose)) || isNerve(GET_NERVE(TalkDirector, TalkDirectorNrvSlct)) ||
        isNerve(GET_NERVE(TalkDirector, TalkDirectorNrvNext))) {
        return !TalkFunction::isShortTalk(mTalkState->mTalkCtrl);
    }

    return false;
}

bool TalkDirector::isNormalTalking() const {
    return isSystemTalking() && mDemoType == 1;
}

LiveActor* TalkDirector::getTalkingActor() const {
    if (isSystemTalking()) {
        return mTalkState->mTalkCtrl->mHostActor;
    }

    return nullptr;
}

void TalkDirector::exePrep() {
    if (MR::isLessStep(this, 4)) {
    }

    if (mTalkState->prep(mMsgCtrl)) {
        TalkFunction::onTalkStateEnableStart(mTalkState->mTalkCtrl);
        return;
    }

    TalkFunction::onTalkStateNone(mTalkState->mTalkCtrl);
    mTalkState = nullptr;
    setNerve(GET_NERVE(TalkDirector, TalkDirectorNrvWait));
}

void TalkDirector::exeWait() {
}

void TalkDirector::exeTalk() {
    TalkMessageCtrl* control = mTalkState->mTalkCtrl;
    TalkFunction::onTalkStateTalking(control);

    bool cond = false;

    if (mDemoType == 0) {
        cond = mTalkState->isLostMessage(mMsgCtrl);
    }

    if (!cond) {
        if (!mTalkState->talk(mMsgCtrl)) {
            return;
        }

        MR::readMessage(control);
    }

    if (!cond && TalkFunction::isSelectTalk(control)) {
        setNerve(GET_NERVE(TalkDirector, TalkDirectorNrvSlct));
        return;
    }
    setNerve(GET_NERVE(TalkDirector, TalkDirectorNrvClose));
}

void TalkDirector::exeClose() {
    if(MR::isFirstStep(this)) mTalkState->clos();
    if(mTalkState->isBalloonDead()) {
        _50=false;
        TalkMessageCtrl* control=mTalkState->mTalkCtrl;
        if(control->mIsOnRootNodeAuto && !mTalkState->isSelfInterrupt(_44)) control->rootNodePst();
        setNerve(GET_NERVE(TalkDirector,TalkDirectorNrvNext));
        exeNext();
    }
}

void TalkDirector::exeSlct() {
    TalkMessageCtrl* control = mTalkState->mTalkCtrl;

    if (MR::isFirstStep(this)) {
        appearYesNoSelector(control);
    }

    TalkFunction::onTalkStateTalking(control);

    if (!MR::isDeadYesNoLayout() && MR::isYesNoSelected()) {
        MR::resetYesNoSelectorSE();
        mTalkState->clos();
        if (control->mIsOnRootNodeAuto) {
            control->rootNodeSel(MR::isYesNoSelectedYes());
            if(!TalkFunction::isForwardNode(control)) _50=true;
            control->rootNodePre(true);
        } else {
            _50=true;
        }

        setNerve(GET_NERVE(TalkDirector, TalkDirectorNrvNext));
        exeNext();
    }
}

void TalkDirector::exeNext() {
    TalkMessageCtrl* control = mTalkState->mTalkCtrl;

    bool cond = control->isCurrentNodeContinue();
    TalkFunction::onTalkStateTalking(control);

    if (control->rootNodeEve()) {
        _50 |= cond;

        if (_50) {
            if (TalkFunction::isShortTalk(control)) {
                termTalk();
            }
            control->rootNodePre(true);
            if(control->rootNodeEve()) {
                mTalkState = initState(control);
                mTalkState->open();
                setNerve(GET_NERVE(TalkDirector, TalkDirectorNrvTalk));
            }

            return;
        }
        termTalk();
        TalkFunction::onTalkStateEnableEnd(control);
        setNerve(GET_NERVE(TalkDirector, TalkDirectorNrvTerm));
    }
}

void TalkDirector::exeTerm() {
    if (!_58) {
        if (!mTalkState->term(mMsgCtrl)) {
            return;
        }
    }

    TalkFunction::onTalkStateNone(mTalkState->mTalkCtrl);
    mTalkState = nullptr;

    setNerve(GET_NERVE(TalkDirector, TalkDirectorNrvWait));
}

bool TalkFunction::requestTalkSystem(TalkMessageCtrl* pCtrl, bool force) {
    return ::getTalkDirector()->request(pCtrl, force);
}

bool TalkFunction::startTalkSystem(TalkMessageCtrl* pCtrl, bool force, bool demo, bool notPuppetable) {
    return ::getTalkDirector()->start(pCtrl, force, demo, notPuppetable);
}

bool TalkFunction::endTalkSystem(TalkMessageCtrl* pCtrl) {
    ::getTalkDirector();
    return MR::isTalkEnableEnd(pCtrl);
}

bool TalkFunction::isTalkSystemStart(const TalkMessageCtrl* pCtrl) {
    TalkDirector* pDirector = ::getTalkDirector();

    return pDirector->mMsgCtrl == pCtrl && pDirector->_4C;
}

bool TalkFunction::isTalkSystemEnd(const TalkMessageCtrl* pCtrl) {
    TalkDirector* pDirector = ::getTalkDirector();

    return pDirector->mMsgCtrl == pCtrl && pDirector->_4D;
}

void TalkFunction::registerTalkSystem(TalkMessageCtrl* pCtrl) {
    ::getTalkDirector()->mMsgControls.push_back(pCtrl);
}

void MR::pauseOffTalkDirector() {
    ::getTalkDirector()->pauseOff();
}

void MR::validateTalkDirector() {
    ::getTalkDirector()->_4E = false;
}

void MR::invalidateTalkDirector() {
    ::getTalkDirector()->_4E = true;
}

void MR::setTalkDirectorDrawSyncToken() {
    if (::getTalkDirector() != nullptr) {
        ::getTalkDirector()->mPeekZ->setDrawSyncToken();
    }
}

bool MR::isActiveTalkBalloonShort() {
    if (::getTalkDirector() != nullptr) {
        return ::getTalkDirector()->mBalloonHolder->isActiveBalloonShort();
    }

    return false;
}
