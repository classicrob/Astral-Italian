#include "Game/MapObj/FlipPanel.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActorGroupArray.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/MapObjActorInitInfo.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include <JSystem/JMath/JMath.hpp>
#ifdef GALAXY_NATIVE
#include "Game/Util/NativeActorObserve.hpp"
#endif


namespace NrvFlipPanel {
    NEW_NERVE(FlipPanelNrvFrontLand, FlipPanel, FrontLand);
    NEW_NERVE(FlipPanelNrvBackLand, FlipPanel, BackLand);
    NEW_NERVE(FlipPanelNrvFront, FlipPanel, Wait);
    NEW_NERVE(FlipPanelNrvBack, FlipPanel, Wait);
    NEW_NERVE(FlipPanelNrvEndPrepare, FlipPanel, EndPrepare);
    NEW_NERVE(FlipPanelNrvEnd, FlipPanel, End);
};  // namespace NrvFlipPanel

namespace {
    static u32 sBloomSyncStep;
};  // namespace

namespace NrvFlipPanelObserver {
    NEW_NERVE(FlipPanelObserverNrvWait, FlipPanelObserver, Wait);
    NEW_NERVE(FlipPanelObserverNrvComplete, FlipPanelObserver, Complete);
    NEW_NERVE(FlipPanelObserverNrvDemoWait, FlipPanelObserver, DemoWait);
};  // namespace NrvFlipPanelObserver

FlipPanel::FlipPanel(const char* pName) : MapObjActor(pName) {
    mDelegator = 0;
    mFlipPanelGroup = 0;
    _CC = 0;
    _CD = 0;
    _D0 = 0;
    mIsReverse = false;
}

void FlipPanel::init(const JMapInfoIter& rIter) {
    MapObjActor::init(rIter);
    MapObjActorInitInfo info;
    info.setupHioNode("地形オブジェ");
    info.setupDefaultPos();
    info.setupConnectToScene();
    info.setupEffect("FlipPanel");
    info.setupSound(4);
    info.setupNerve(GET_NERVE(FlipPanel, FlipPanelNrvFront));
    initialize(rIter, info);
    mIsReverse = isObjectName("FlipPanelReverse");
    mDelegator = MR::createJointDelegatorWithNullChildFunc(this, &FlipPanel::calcJointMove, "Panel");
    mFlipPanelGroup = MR::joinToGroupArray(this, rIter, "フリップパネル", 0x80);
    MR::invalidateClipping(this);
    MR::startAction(this, "PanelB");
    f32 frameMax = MR::getBckFrameMax(this);
    MR::setBckFrameAndStop(this, frameMax);

    if (mIsReverse) {
        MapObjActorUtil::appearBloomModel(this);
    } else {
        MapObjActorUtil::killBloomModel(this);
    }
}

void FlipPanel::exeFrontLand() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "PanelB");

        if (mIsReverse) {
            MapObjActorUtil::appearBloomModel(this);
            ModelObj* bloomModel = mBloomModel;
            u32 frameMax = MR::getBrkFrameMax(bloomModel);
            u32 val = ::sBloomSyncStep / frameMax;
            frameMax = ::sBloomSyncStep - (val)*frameMax;
            MR::setBrkFrame(bloomModel, frameMax);
        } else {
            MapObjActorUtil::killBloomModel(this);
        }

        MR::tryRumblePadMiddle(this, WPAD_CHAN0);
        MR::startActionSound(this, "OjFlipPanelChange", -1, -1, -1);

        if (_CD) {
            MR::sendMsgToGroupMember(0x77 /* ACTMES_FAILURE */, this, getSensor(0), "Body");
            _CD = 0;
        }
    }

    if (MR::isOnGroundPlayer() && !MR::isOnPlayer(this)) {
        setNerve(GET_NERVE(FlipPanel, FlipPanelNrvFront));
    }
}

void FlipPanel::exeBackLand() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "PanelA");

        if (mIsReverse) {
            MapObjActorUtil::killBloomModel(this);
        } else {
            MapObjActorUtil::appearBloomModel(this);
            ModelObj* bloomModel = mBloomModel;
            u32 frameMax = MR::getBrkFrameMax(bloomModel);
            u32 val = ::sBloomSyncStep / frameMax;
            frameMax = ::sBloomSyncStep - (val)*frameMax;
            MR::setBrkFrame(bloomModel, frameMax);
        }

        MR::tryRumblePadMiddle(this, WPAD_CHAN0);
        MR::startActionSound(this, "OjFlipPanelChange", -1, -1, -1);

        if (!_CD) {
            MR::sendMsgToGroupMember(0x76 /* ACTMES_SUCCESS */, this, getSensor(0), "Body");
            _CD = 1;
        }
    }

    if (MR::isOnGroundPlayer() && !MR::isOnPlayer(this)) {
        setNerve(GET_NERVE(FlipPanel, FlipPanelNrvBack));
    }
}

void FlipPanel::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);
    }

    if (_CC && !MR::isOnPlayer(this) && MR::isOnGroundPlayer()) {
        _CC = 0;
    }

    if (!_CC) {
        if (checkPlayerOnTop()) {
            if (isNerve(GET_NERVE(FlipPanel, FlipPanelNrvFront))) {
                setNerve(GET_NERVE(FlipPanel, FlipPanelNrvBackLand));
            } else {
                setNerve(GET_NERVE(FlipPanel, FlipPanelNrvFrontLand));
            }
        }
    }
}

void FlipPanel::exeEndPrepare() {
    if (MR::isStep(this, 20)) {
        setNerve(GET_NERVE(FlipPanel, FlipPanelNrvEnd));
    }
}

void FlipPanel::exeEnd() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "OjFlipPanelComplete", -1, -1, -1);
        MR::startAction(this, "PanelEnd");
        MapObjActorUtil::killBloomModel(this);
        _CC = 0;
    }

    if (MR::isActionEnd(this)) {
        if (!MR::isEffectValid(this, "FlipPanelGold")) {
            MR::validateClipping(this);
        }
    }
}

void FlipPanel::appear() {
    MapObjActor::appear();
    if (mIsReverse) {
        MapObjActorUtil::appearBloomModel(this);
    } else {
        MapObjActorUtil::killBloomModel(this);
    }
}

void FlipPanel::endClipped() {
    MapObjActor::endClipped();

    if (MR::getBrkCtrl(mBloomModel)) {
        ModelObj* bloomModel = mBloomModel;
        u32 frameMax = MR::getBrkFrameMax(bloomModel);
        u32 val = ::sBloomSyncStep / frameMax;
        frameMax = ::sBloomSyncStep - (val)*frameMax;
        MR::setBrkFrame(bloomModel, frameMax);
    }
}

void FlipPanel::calcAndSetBaseMtx() {
    MapObjActor::calcAndSetBaseMtx();

    if (_CC) {
        mDelegator->registerCallBack();
    }
#ifdef GALAXY_NATIVE
    // state: 1 flipped to the back (the side the puzzle wants), 0 front.
    MR::Native::publishActor("FlipPanel", mPosition, -mGravity, _CD ? 1 : 0, 0);
#endif
}

bool FlipPanel::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(FlipPanel, FlipPanelNrvEnd))) {
        return false;
    }

    if (msg == 0x76 /* ACTMES_SUCCESS */ || msg == 0x77 /* ACTMES_FAILURE */) {
        _CC = this == MR::getSensorHost(pSender);

        return true;
    }

    if (msg == 0x7B /* ACTMES_GROUP_MOVE_STOP */) {
        MR::invalidateClipping(this);
        setNerve(GET_NERVE(FlipPanel, FlipPanelNrvEndPrepare));

        return true;
    }

    return false;
}

bool FlipPanel::calcJointMove(TPos3f* pPos, const JointControllerInfo& rInfo) {
    if (!_CC) {
        return false;
    }

    TVec3f jointPos;
    MR::copyJointPos(this, "Panel", &jointPos);
    TVec3f upVec;
    MR::calcUpVec(&upVec, this);
    jointPos.scaleAdd(-25.0f, upVec, jointPos);
    pPos->setTrans(jointPos);
    return true;
}

bool FlipPanel::checkPlayerOnTop() {
    if (MR::isPlayerJumpRising() && _D0) {
        _D0 = 0;
        return true;
    }

    if (!MR::isOnPlayer(this)) {
        _D0 = 0;
        return false;
    }

    TVec3f upVec;
    MR::calcUpVec(&upVec, this);

    if (!MR::getPlayerGroundingPolygon()) {
        return false;
    }

    if (upVec != *MR::getPlayerGroundNormal()) {
        _D0 = 0;
        return false;
    }

    if (_D0 < 2) {
        _D0 = _D0 + 1;
        return false;
    }

    _D0 = 0;
    return true;
}

FlipPanelObserver::FlipPanelObserver(const char* pName) : LiveActor(pName) {
    _8C = 0;
    _90 = 0;
    mDemoDelay = 0;
    mPowerStarId = -1;
    _9C = 0;
    ::sBloomSyncStep = 0;
}

void FlipPanelObserver::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::connectToSceneMapObjMovement(this);
    initHitSensor(1);
    MR::addBodyMessageSensorMapObj(this);
    MR::invalidateClipping(this);
    _8C = MR::joinToGroupArray(this, rIter, "フリップパネル", 0x80);
    MR::initUseStageSwitchWriteA(this, rIter);
    initSound(2, "FlipPanel", nullptr, TVec3f(0.0f));
    MR::getJMapInfoArg7NoInit(rIter, &mPowerStarId);

    if (mPowerStarId != -1) {
        if (!mPowerStarId) {
            MR::declarePowerStar(this);
        } else {
            MR::declarePowerStar(this, mPowerStarId);
        }
    }

    MR::getJMapInfoArg0NoInit(rIter, &mDemoDelay);
    mDemoDelay += 0x28;

    _9C = 0;

    initNerve(GET_NERVE(FlipPanelObserver, FlipPanelObserverNrvWait));
    MR::useStageSwitchSyncAppear(this, rIter);
}

void FlipPanelObserver::exeWait() {
    if (MR::isFirstStep(this) && MR::isValidSwitchAppear(this)) {
        MR::callAppearAllGroupMember(this);
        MR::startActionSound(this, "OjFlipPanelAppear", -1, -1, -1);
    }

    if (_90 == _8C->getObjNum() - 1 && MR::tryStartDemo(this, "FlipPanelComplete")) {
        setNerve(GET_NERVE(FlipPanelObserver, FlipPanelObserverNrvComplete));
    } else {
        ::sBloomSyncStep++;
    }
}

void FlipPanelObserver::exeComplete() {
    if (MR::isFirstStep(this)) {
        MR::callRequestMovementOnAllGroupMember(this);
        MR::requestMovementOn(_8C);
        MR::sendMsgToGroupMember(0x7B /* ACTMES_GROUP_MOVE_STOP */, this, getSensor(0), "Body");
    }

    if (MR::isGreaterStep(this, mDemoDelay)) {
        MR::endDemo(this, "FlipPanelComplete");

        if (mPowerStarId != -1) {
            if (!mPowerStarId) {
                MR::requestAppearPowerStar(this, mPosition);
            } else {
                MR::requestAppearPowerStar(this, mPowerStarId, mPosition);
            }
        }

        setNerve(GET_NERVE(FlipPanelObserver, FlipPanelObserverNrvDemoWait));
    }
}

void FlipPanelObserver::exeDemoWait() {
    if (!MR::isDemoActive()) {
        if (MR::isValidSwitchA(this)) {
            MR::onSwitchA(this);
        }

        makeActorDead();
    }
}

void FlipPanelObserver::initAfterPlacement() {
    if (MR::isValidSwitchAppear(this)) {
        MR::callMakeActorDeadAllGroupMember(this);
    }
}

bool FlipPanelObserver::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (msg == 0x76 /* ACTMES_SUCCESS */) {
        if (!_90) {
            if (_9C) {
                MR::startSystemME("ME_FLIP_PANEL_INV_OFF_FIRST");
            } else {
                MR::startSystemME("ME_FLIP_PANEL_ON_FIRST");
            }
        } else {
            if (_9C) {
                MR::startSystemME("ME_FLIP_PANEL_INV_OFF");
            } else {
                MR::startSystemME("ME_FLIP_PANEL_ON");
            }
        }

        _90++;

        return true;
    } else if (msg == 0x77 /* ACTMES_FAILURE */) {
        if (_9C) {
            MR::startSystemME("ME_FLIP_PANEL_INV_ON");
        } else {
            MR::startSystemME("ME_FLIP_PANEL_OFF");
        }

        _90--;

        return true;
    }

    return false;
}
