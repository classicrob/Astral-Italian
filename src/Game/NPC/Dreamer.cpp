#include "Game/Util/MessageUtil.hpp"
#include "Game/Util/PadRecorder.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Screen/DreamerButtonGuidance.hpp"
#include "Game/NPC/DreamerFunction.hpp"
#include "Game/NPC/NrvDreamer.hpp"
#include "Game/NPC/Dreamer.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/NPC/TalkMessageFunc.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/NPCUtil.hpp"

extern "C" const char* lbl_807CFEA0;
extern "C" const char* lbl_807CFEA4;
extern "C" const char* lbl_807CFEA8;
extern "C" s32 lbl_807E30F0;



Dreamer::Dreamer(const char* pName) : NPCActor(pName) {
    mButtonGuidance = nullptr;
    mStartId = -1;
    mMissNum = -1;
    _170 = false;
    _174 = 0;
    _178 = 0;
    mIsLuigi = false;
}

void Dreamer::init(const JMapInfoIter& rIter) {
    MR::getJMapInfoArg0NoInit(rIter, &mStartId);

    NPCActorCaps caps("Dreamer");
    caps.setDefault();
    caps.mSensorSize = 110.0f;
    caps.mUseShadow = true;
    caps.mMakeActor = false;
    caps.mLightCtrl = false;
    caps._95 = false;
    caps.mStarPointerOffs.x = 0.0f;
    caps.mStarPointerOffs.y = 110.0f;
    caps.mStarPointerOffs.z = 0.0f;
    caps.mSensorOffset.x = 0.0f;
    caps.mSensorOffset.y = 110.0f;
    caps.mSensorOffset.z = 0.0f;

    mParam._14 = "Wait";
    mParam._18 = "Wait";
    mParam._1C = "Wait";
    mParam._20 = "Wait";
    mParam._8 = 2.5f;
    _130 = "Spin";
    _134 = "Trampled";
    _138 = "Pointing";
    _13C = "Reaction";

    if (MR::isPlayerLuigi() ||
        (GameSequenceFunction::isPadMode() && !DreamerFunction::isPadModeDreamerAndStartId(mStartId))) {
        initForLuigi(rIter, caps);
    } else if (DreamerFunction::isPadModeDreamerAndStartId(mStartId)) {
        initForPlaying(rIter, caps);
    } else {
        initForNpc(rIter, caps);
    }

    MR::startBrk(this, "Dreamer");
    MR::startBtk(this, "Dreamer");
}

void Dreamer::appear() {
    if (!mIsLuigi) {
        if (MR::isDead(this)) {
            MR::startActionSound(this, "Appear");
            NPCActor::makeActorAppeared();
            setNerve(GET_NERVE(Dreamer, DreamerNrvAppear));
        }
    }
}

void Dreamer::makeActorAppeared() {
    if (mIsLuigi) {
        return;
    }
    NPCActor::makeActorAppeared();
}

void Dreamer::control() {
    NPCActor::control();
    if (!GameSequenceFunction::isPadMode() && !MR::isPlayerLuigi()) {
        MR::startSoundInPlayerDistance(this, "Waiting", -1, -1, 1000.0f);
    }
}

void Dreamer::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Appear");
    }
    if (MR::isActionEnd(this)) {
        setNerve(mWaitNerve);
    }
}

void Dreamer::exeStartHelp() {
    if (MR::isFirstStep(this)) {
        MR::onDestroySceneKeepAllSound();
        MR::startActionSound(this, "WipeIn");
        MR::stopStageBGM(0x1E);
    }
    if (MR::isGreaterEqualStep(this, lbl_807E30F0)) {
        if (mStartId >= 0) {
            GameSequenceFunction::setPadModeDreamer(this, mStartId);
        } else {
            setNerve(mWaitNerve);
        }
    }
}

void Dreamer::exeInHelp() {
    if (MR::isFirstStep(this)) {
        MR::startStageBGM("BGM_DREAM01", false);
        mButtonGuidance->forceAppear();
    }
    if (MR::isPadRecorderStopped()) {
        if (_170) {
            if (MR::tryTalkForceMarioPuppetable(mMsgCtrl)) {
                setNerve(mTalkNerve);
            }
        } else {
            MR::stopStageBGM(0xF);
            MR::onDestroySceneKeepAllSound();
            MR::startActionSound(this, "WipeOut");
            GameSequenceFunction::clearPadModeDreamer();
            setNerve(mWaitNerve);
        }
    }
}

void Dreamer::exeEndHelp() {
    if (MR::isFirstStep(this)) {
        MR::stopStageBGM(0xF);
        MR::onDestroySceneKeepAllSound();
        MR::startActionSound(this, "WipeOut");
        GameSequenceFunction::clearPadModeDreamer();
        setNerve(mWaitNerve);
    }
}

void Dreamer::initForLuigi(const JMapInfoIter& rIter, const NPCActorCaps& rCaps) {
    NPCActorCaps& caps = const_cast< NPCActorCaps& >(rCaps);
    caps._F = true;
    caps._10 = lbl_807CFEA4;
    initialize(rIter, caps, nullptr, nullptr, nullptr);
    makeActorDead();
    mIsLuigi = true;
}

void Dreamer::initForPlaying(const JMapInfoIter& rIter, const NPCActorCaps& rCaps) {
    NPCActorCaps& caps = const_cast< NPCActorCaps& >(rCaps);
    caps.mWaitNerve = GET_NERVE(Dreamer, DreamerNrvEndHelp);
    if (MR::isExistMessageId(rIter) && MR::isExistSceneMessage(lbl_807CFEA8, rIter, true)) {
        caps._10 = lbl_807CFEA8;
        _170 = true;
    } else {
        caps.mMessage = false;
    }
    initialize(rIter, caps, nullptr, nullptr, nullptr);
    if (mMsgCtrl) {
        MR::offTalkCinemaFrame(mMsgCtrl);
        MR::hideBubbleBeak(mMsgCtrl);
    }
    setNerve(GET_NERVE(Dreamer, DreamerNrvInHelp));
    MR::invalidateClipping(this);
    MR::registerDemoSimpleCastAll(this);
    makeActorAppeared();
    MR::hideModel(this);
    MR::invalidateHitSensors(this);
    MR::invalidateShadowAll(this);
    initButtonGuidance();
}

void Dreamer::initForNpc(const JMapInfoIter& rIter, const NPCActorCaps& rCaps) {
    NPCActorCaps& caps = const_cast< NPCActorCaps& >(rCaps);
    if (!MR::isExistMessageId(rIter) || !MR::isExistSceneMessage(lbl_807CFEA0, rIter, true)) {
        caps._F = true;
        caps._10 = lbl_807CFEA4;
    } else {
        caps._10 = lbl_807CFEA0;
    }
    initialize(rIter, caps, nullptr, nullptr, nullptr);
    MR::registerEventFunc(mMsgCtrl,
                          TalkMessageFuncM< Dreamer*, bool (Dreamer::*)(u32) >(this, &Dreamer::eventFunc));
    MR::getJMapInfoArg1NoInit(rIter, &mMissNum);
    if (isMakeAppear()) {
        makeActorAppeared();
    } else {
        makeActorDead();
    }
    DreamerFunction::initChildObj(rIter);
}

void Dreamer::initButtonGuidance() {
    DreamerButtonGuidance* pGuidance = new DreamerButtonGuidance("ドリーマー用レイアウト");
    mButtonGuidance = pGuidance;
    pGuidance->initWithoutIter();
    mButtonGuidance->showDreamer();
}

bool Dreamer::isMakeAppear() const {
    if (DreamerFunction::isPadModeDreamerAndStartId(mStartId)) {
        return true;
    }
    if (GameSequenceFunction::isPadMode2()) {
        return false;
    }
    bool result;
    if (mMissNum < 0) {
        result = false;
        if (MR::isValidSwitchAppear(this)) {
            if (MR::isOnSwitchAppear(this)) {
                result = true;
            }
        }
    } else {
        result = false;
        if (MR::isValidSwitchAppear(this) && MR::isOnSwitchAppear(this)) {
            result = true;
        } else if (GameSequenceFunction::isAllowDreamerMissNum(mMissNum)) {
            result = true;
        }
    }
    return result;
}

bool Dreamer::eventFunc(u32 event) {
    switch (event) {
    case 0:
        setNerve(mWaitNerve);
        break;
    case 1:
        setNerve(GET_NERVE(Dreamer, DreamerNrvStartHelp));
        break;
    }
    return true;
}
