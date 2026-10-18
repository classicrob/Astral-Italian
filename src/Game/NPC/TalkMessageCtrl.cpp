#pragma auto_inline off

#include "Game/NPC/TalkMessageCtrl.hpp"
#include "Game/AreaObj/MessageArea.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/NPC/TalkDirector.hpp"
#include "Game/NPC/TalkMessageFunc.hpp"
#include "Game/NPC/TalkMessageInfo.hpp"
#include "Game/NPC/TalkNodeCtrl.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/System/MessageHolder.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/AreaObjUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/NPCUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/TalkUtil.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JGeometry/TVec.hpp>
#include <revolution/mtx.h>

extern "C" const char lbl_806C0338[];
extern "C" const char lbl_806C0344[];

TalkMessageCtrl::TalkMessageCtrl(LiveActor* pHost, const TVec3f& rArg2, MtxPtr pArg3)
    : NameObj("会話制御"), mHostActor(pHost), mNodeCtrl(), mZoneID(-1), _1C(0.0f, 0.0f, 0.0f), mMsgBalloonFollowOffs(rArg2), mTalkDistance(240.0f),
      _3C(), mIsOnReadNodeAuto(true), mAlreadyDoneFlags(-1), mIsStartOnlyFront(), _SMG2_4F(false), _SMG2_50(true), _SMG2_51(false), _SMG2_52(false), _SMG2_53(false), _SMG2_54(false), mCameraInfo(), mBranchFunc(), mEventFunc(), mAnimeFunc(), mKillFunc(),
      mMtx(pArg3), mIsOnRootNodeAuto(), _18(), mTagArg(nullptr, CustomTagArg::Type_Uninitialized) {
    mTagArg.mArgType = CustomTagArg::Type_Uninitialized;
    mTagArg.mIntArg = 0;
    MR::createSceneObj(SceneObj_TalkDirector);
    TalkFunction::registerTalkSystem(this);
    mZoneID = mHostActor->mLinkInfo.mZoneID;
}

bool TalkMessageCtrl::inMessageArea() const {
    s8 messageInfoEnumByte = mNodeCtrl->mMessageInfo._A;
    s32 messageInfoEnum = mNodeCtrl->mMessageInfo._A;
    s32 zoneID = mZoneID;

    if (messageInfoEnumByte < 0) {
        return false;
    }

    TVec3f* playerPos = MR::getPlayerCenterPos();
    MessageArea* areaObj = (MessageArea*)MR::getAreaObj("MessageArea", *playerPos);
    if (areaObj != nullptr && messageInfoEnum == areaObj->mObjArg0 && zoneID == areaObj->mZoneID) {
        return true;
    }

    return false;
}

void TalkMessageCtrl::startCamera(s32 a1) {
    TalkMessageInfo* inf = &mNodeCtrl->mMessageInfo;
    if (_SMG2_54) {
        return;
    }
    if (_SMG2_53) {
        MR::startMultiActorCameraNoTarget(mHostActor, mCameraInfo, lbl_806C0338, a1);
    } else if (inf->isCameraNormal()) {
        MR::startNPCTalkCamera(this, mHostActor->getBaseMtx(), 1.0f, a1);
    } else if (inf->isCameraEvent()) {
        if (inf->mCameraSetID) {
            mCameraInfo->mCameraSetID = inf->mCameraSetID;
            MR::startMultiActorCameraNoTarget(mHostActor, mCameraInfo, lbl_806C0344, a1);
        }
    }
}

TalkMessageCtrl::~TalkMessageCtrl() {
}

bool TalkMessageCtrl::createMessage(const JMapInfoIter& rIter, const char* pName) {
    mNodeCtrl = new TalkNodeCtrl();

    if (!mNodeCtrl->createFlowNode(this, rIter, pName, &mCameraInfo)) {
        return false;
    }

    if (mNodeCtrl->mCurrentNode != nullptr) {
        _3C = MR::setupAlreadyDoneFlag(&mAlreadyDoneFlags, mNodeCtrl->mMessageName, rIter, 0);
    } else {
        _3C = 1;
    }

    _SMG2_4F = 1;
    return true;
}

bool TalkMessageCtrl::createMessageDirect(const JMapInfoIter& rIter, const char* pName) {
    mNodeCtrl = new TalkNodeCtrl();

    if (!mNodeCtrl->createFlowNodeDirect(this, rIter, pName, &mCameraInfo)) {
        return false;
    }

    if (MR::isValidInfo(rIter) && mNodeCtrl->mCurrentNode != nullptr) {
        _3C = MR::setupAlreadyDoneFlag(&mAlreadyDoneFlags, mNodeCtrl->mMessageName, rIter, 0);
    } else {
        _3C = 1;
    }

    _SMG2_4F = 1;
    return true;
}

bool TalkMessageCtrl::rootNodeEve() {
    TalkNodeCtrl* nodeCtrl = mNodeCtrl;
    TalkNode* nodeEvent = nodeCtrl->getCurrentNodeEvent();
    if (nodeEvent == nullptr) {
        return true;
    }

    switch (nodeEvent->mEventType) {
    case 0:
    case 1:
        if (!mEventFunc) return true;
        if (!mEventFunc->operator()(nodeEvent->mUnknown)) return false;
        break;
    case 4:
        if (!mAnimeFunc) return true;
        if (!mAnimeFunc->operator()(nodeEvent->mUnknown)) return false;
        break;
    case 3:
        rootNodePst();
        return true;
    case 5: MR::onSwitchA(mHostActor); break;
    case 6: MR::onSwitchB(mHostActor); break;
    case 7:
        if (!mKillFunc->operator()(nodeEvent->mUnknown)) return false;
        break;
    case 8: MR::offSwitchA(mHostActor); break;
    case 9: MR::offSwitchB(mHostActor); break;
    case 10: MR::hideBubbleBeak(this); break;
    case 11: MR::showBubbleBeak(this); break;
    }

    bool cond = nodeCtrl->isExistNextNode();
    rootNodePst();

    if (cond && nodeCtrl->getCurrentNodeEvent()) {
        return false;
    }

    return true;
}

void TalkMessageCtrl::rootNodePre(bool arg) {
    TalkNodeCtrl* control = mNodeCtrl;
    control->resetTempFlowNode();
    while (true) {
        TalkNode* branch = control->getCurrentNodeBranch();

        if (branch == nullptr) {
            if (arg) {
                return;
            }

            if (!isCurrentNodeContinue()) {
                return;
            }

            control->forwardFlowNode();
            continue;
        }

        bool cond = false;

        switch (branch->mIndex) {
        case TalkMessageCtrl_Type0:
            return;
        case TalkMessageCtrl_Type1:
            if (mBranchFunc == nullptr) {
                cond = true;
            } else {
                cond = mBranchFunc->operator()(branch->mNextIdx);
            }

            break;
        case TalkMessageCtrl_Type2:
            cond = MR::isNearPlayerAnyTime(mHostActor, mTalkDistance);
            break;
        case TalkMessageCtrl_Type3:
            cond = MR::isOnSwitchA(mHostActor);
            break;
        case TalkMessageCtrl_Type4:
            cond = MR::isOnSwitchB(mHostActor);
            break;
        case TalkMessageCtrl_Type5:
            cond = MR::isPlayerElementModeNormal();
            break;
        case TalkMessageCtrl_Type6:
            cond = MR::isPlayerElementModeBee();
            break;
        case TalkMessageCtrl_Type7:
            cond = MR::isPlayerElementModeTeresa();
            break;
        case TalkMessageCtrl_Type8:
            cond = MR::isStageStatePowerStarAppeared();
            break;
        case 9: cond = !GameDataFunction::isOnGameEventFlagEnding120Stars(); break;
        case 10: cond = _3C; break;
        case 11: cond = MR::isPlayerLuigi(); break;
        case 12: cond = MR::isTimeKeepDemoActive(); break;
        case 13: cond = GameDataFunction::isOnMessageAlreadyRead(branch->mNextIdx); break;
        case 14: cond = MR::isPlayerElementModeYoshi(); break;
        case 15: cond = MR::isPlayerElementModeCloud(); break;
        case 16: cond = MR::isPlayerElementModeRock(); break;
        }

        if (cond) {
            control->forwardCurrentBranchNode(true);
            continue;
        }

        control->forwardCurrentBranchNode(false);
    }
}

void TalkMessageCtrl::rootNodePst() {
    TalkNodeCtrl* ctrl = mNodeCtrl;
    if (ctrl->isExistNextNode()) {
        ctrl->forwardFlowNode();
    } else {
        ctrl->resetFlowNode();
    }

    ctrl->recordTempFlowNode();
}

bool TalkMessageCtrl::isCurrentNodeContinue() const {
    TalkNode* nodeEvent = mNodeCtrl->getCurrentNodeEvent();

    if (nodeEvent == nullptr) {
        return false;
    }

    return nodeEvent->mEventType <= 4 && ((1 << nodeEvent->mEventType) & 0x15);
}

void TalkMessageCtrl::rootNodeSel(bool a1) {
    TalkNodeCtrl* ctrl = mNodeCtrl;
    ctrl->forwardFlowNode();

    if (a1) {
        ctrl->forwardCurrentBranchNode(true);
    } else {
        ctrl->forwardCurrentBranchNode(false);
    }

    ctrl->recordTempFlowNode();
}

void TalkMessageCtrl::registerBranchFunc(const TalkMessageFuncBase& rFunc) {
    mBranchFunc = rFunc.clone();
}

void TalkMessageCtrl::registerEventFunc(const TalkMessageFuncBase& rFunc) {
    mEventFunc = rFunc.clone();
}

void TalkMessageCtrl::registerAnimeFunc(const TalkMessageFuncBase& rFunc) {
    mAnimeFunc = rFunc.clone();
}

void TalkMessageCtrl::registerKillFunc(const TalkMessageFuncBase& rFunc) {
    mKillFunc = rFunc.clone();
}

void TalkMessageCtrl::readMessage() {
    if (mIsOnReadNodeAuto) {
        mNodeCtrl->readMessage();
    }

    if (!_3C) {
        MR::updateAlreadyDoneFlag(mAlreadyDoneFlags);
    }
}

bool TalkMessageCtrl::isSelectYesNo() const {
    TalkNode* node = mNodeCtrl->getNextNodeBranch();

    if (node == nullptr || node->mIndex != 0) {
        return false;
    }

    switch (node->mNextIdx) {
    case 2:
        return false;
    case 11:
        return false;
    case 15:
        return false;
    case 17:
        return false;
    }

    return true;
}

u32 TalkMessageCtrl::getMessageID() const {
    TalkNodeCtrl* ctrl = mNodeCtrl;
    if (ctrl->getCurrentNodeMessage() != nullptr) {
        return ctrl->getCurrentNodeMessage()->mIndex;
    } else {
        return ctrl->mCurrentNodeIdx;
    }
}

bool TalkMessageCtrl::requestTalk() {
    return TalkFunction::requestTalkSystem(this, false);
}

bool TalkMessageCtrl::requestTalkForce() {
    return TalkFunction::requestTalkSystem(this, true);
}

bool TalkMessageCtrl::startTalk() {
    return TalkFunction::startTalkSystem(this, false, true, true);
}

bool TalkMessageCtrl::startTalkForce() {
    return TalkFunction::startTalkSystem(this, true, true, true);
}

bool TalkMessageCtrl::startTalkForcePuppetable() {
    return TalkFunction::startTalkSystem(this, true, true, false);
}

bool TalkMessageCtrl::startTalkForceWithoutDemo() {
    return TalkFunction::startTalkSystem(this, true, false, true);
}

bool TalkMessageCtrl::startTalkForceWithoutDemoPuppetable() {
    return TalkFunction::startTalkSystem(this, true, false, false);
}

bool TalkMessageCtrl::endTalk() {
    return TalkFunction::endTalkSystem(this);
}

bool TalkMessageCtrl::isNearPlayer(const TalkMessageCtrl* pCtrl) {
    TVec3f* hostPos;
    TVec3f* ctrlHostPos;

    if (pCtrl == nullptr) {
        return true;
    }

    TVec3f* playerPos = MR::getPlayerPos();
    hostPos = &mHostActor->mPosition;
    ctrlHostPos = &pCtrl->mHostActor->mPosition;

    return hostPos->squared(*playerPos) < ctrlHostPos->squared(*playerPos);
}

const char* TalkMessageCtrl::getBranchID() const {
    TalkNode* node = mNodeCtrl->getNextNodeBranch();

    if (node == nullptr || node->mIndex != 0) {
        return nullptr;
    }

    const char* result = nullptr;
    switch (node->mNextIdx) {
    case 0: result = "PichanRace"; break;
    case 1: result = "TicoShopExchange"; break;
    case 2: result = "TicoShopWhich"; break;
    case 3: result = "Dreamer"; break;
    case 4: result = "SuperDreamer"; break;
    case 5: result = "MuimuiPlay"; break;
    case 6: result = "ScoreAttackPlay"; break;
    case 7: result = "ScoreAttackContinue"; break;
    case 8: result = "GliBirdRide"; break;
    case 9: result = "TicoFatCoinGive"; break;
    case 10: result = "FireWoodBoxPlay"; break;
    case 11: result = "GliBirdTutorial"; break;
    case 12: result = "LuigiTalkNpc"; break;
    case 13: result = "KinopioBank"; break;
    case 14: result = "LuckeyRoom"; break;
    case 15: result = "MeisterTrade"; break;
    case 16: result = "TamakoroTutorial"; break;
    case 17: result = "MeisterConfirm"; break;
    }
    return result;
}

bool TalkFunction::isShortTalk(const TalkMessageCtrl* pCtrl) {
    return pCtrl->mNodeCtrl->mMessageInfo.isShortTalk();
}

bool TalkFunction::isComposeTalk(const TalkMessageCtrl* pCtrl) {
    return pCtrl->mNodeCtrl->mMessageInfo.isComposeTalk();
}

bool TalkFunction::isSelectTalk(const TalkMessageCtrl* pCtrl) {
    return pCtrl->mNodeCtrl->mNodeData == 0;
}

bool TalkFunction::isEventNode(const TalkMessageCtrl* pCtrl) {
    return pCtrl->mNodeCtrl->isCurrentNodeEvent();
}

void TalkFunction::onTalkStateNone(TalkMessageCtrl* pCtrl) {
    pCtrl->_18 = 0;
}

void TalkFunction::onTalkStateEntry(TalkMessageCtrl* pCtrl) {
    pCtrl->_18 = 1;
}

void TalkFunction::onTalkStateEnableStart(TalkMessageCtrl* pCtrl) {
    pCtrl->_18 = 2;
}

void TalkFunction::onTalkStateTalking(TalkMessageCtrl* pCtrl) {
    pCtrl->_18 = 3;
}

void TalkFunction::onTalkStateEnableEnd(TalkMessageCtrl* pCtrl) {
    pCtrl->_18 = 4;
}

TalkMessageInfo* TalkFunction::getMessageInfo(const TalkMessageCtrl* pCtrl) {
    return &pCtrl->mNodeCtrl->mMessageInfo;
}

const wchar_t* TalkFunction::getMessage(const TalkMessageCtrl* pCtrl) {
    return (const wchar_t*)pCtrl->mNodeCtrl->mMessageInfo._0;
}

const wchar_t* TalkFunction::getSubMessage(const TalkMessageCtrl* pCtrl) {
    return (const wchar_t*)pCtrl->mNodeCtrl->getSubMessage();
}

// NONMATCHING: register allocation and equivalent floating addition operand order differ.
bool TalkMessageCtrl::isNearPlayer(f32 distance) const {
    if (mTalkDistance < 0.0f) {
        return true;
    }

    if (distance <= 0.0f) {
        distance = mTalkDistance;
        if (_18 == 2) {
            distance = mTalkDistance + 50.0f;
        }
    }

    TVec3f v2(mHostActor->mPosition);
    if (mMtx != nullptr) {
        v2.set< f32 >(mMtx[0][3], mMtx[1][3], mMtx[2][3]);
    }

    TVec3f v3;
    f32 f2 = MR::vecKillElement(v2 - *MR::getPlayerCenterPos(), mHostActor->mGravity, &v3);

    if (!(MR::abs(f2) > distance) && v3.squared() < distance * distance) {
        TalkMessageInfo* info = &mNodeCtrl->mMessageInfo;
        bool cond = info->mTalkType;
        if (mIsStartOnlyFront || info->isBalloonSign()) {
            TPos3f pos;
            pos.set(mHostActor->getBaseMtx());

            TVec3f v3;  // 0x2C
            pos.getZDir(v3);

            TVec3f v4;  // 0x20
            pos.getTrans(v4);
            v4 = *MR::getPlayerPos() - v4;

            f32 f3 = MR::vecKillElement(v4, v3, &v4);

            if (f3 > 0.0f) {
                return true;
            }
        } else {
            return true;
        }
    }

    return inMessageArea();
}

void TalkMessageCtrl::updateBalloonPos() {
    TMtx34f pos;
    pos.set(mHostActor->getBaseMtx());

    if (mMtx != nullptr) {
        pos.set(mMtx);
    }

    TPos3f tPos = MR::getCameraInvViewMtx();
    TVec3f v3;
    TVec3f nullVec(0.0f, 0.0f, 0.0f);
    MR::setMtxTrans(tPos, nullVec.x, nullVec.y, nullVec.z);
    MR::addTransMtxLocal(tPos, mMsgBalloonFollowOffs);

    v3.set< f32 >(pos[0][3], pos[1][3], pos[2][3]);
    v3.add(TVec3f(tPos[0][3], tPos[1][3], tPos[2][3]));
    MR::calcScreenPosition(&_1C, v3);
}

void TalkMessageCtrl::initCamera(const JMapInfoIter& iter) {
    _SMG2_53 = true;
    MR::initMultiActorCamera(mHostActor, iter, &mCameraInfo, lbl_806C0338);
}

void TalkMessageCtrl::disableCamera() {
    _SMG2_54 = true;
}

void TalkMessageCtrl::endCamera(const TalkMessageInfo& info, s32 step) {
    if (_SMG2_54) {
        return;
    }
    if (_SMG2_53) {
        MR::endMultiActorCamera(mHostActor, mCameraInfo, lbl_806C0338, false, step);
    } else if (info.isCameraNormal()) {
        MR::endNPCTalkCamera(false, step);
    } else if (info.isCameraEvent()) {
        MR::endMultiActorCamera(mHostActor, mCameraInfo, lbl_806C0344, false, step);
    }
}

bool TalkFunction::isGlobalEventTalk(const TalkMessageCtrl* ctrl) {
    return ctrl->mNodeCtrl->mMessageInfo.isGlobalEventTalk();
}

bool TalkFunction::isForwardNode(const TalkMessageCtrl* ctrl) {
    return ctrl->mNodeCtrl->isForwardNode();
}
