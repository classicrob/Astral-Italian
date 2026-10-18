#include "Game/Screen/InformationObserver.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MessageUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

extern "C" bool fn_800210B0();

struct InformationObserverAttribute {
    /* 0x00 */ const char* mMessageId;
    /* 0x04 */ bool _4;
    /* 0x08 */ const char* mFirstMorphBgmName;
    /* 0x0C */ const char* mBgmName;
};

static InformationObserverAttribute sAttr[] = {
    {"InformationObserverBee", true, "BGM_FIRST_MORPH_A", nullptr},
    {"InformationObserverTeresa", true, "BGM_FIRST_MORPH_A", nullptr},
    {"InformationObserverHopper", true, "BGM_FIRST_MORPH_A", nullptr},
    {"InformationObserverFire", true, "BGM_FIRST_MORPH_A", "BGM_FIRE_B"},
    {"InformationObserverIce", true, "BGM_FIRST_MORPH_A", "BGM_ICE_B"},
    {"InformationObserverFlying", true, "BGM_FIRST_MORPH_A", "BGM_FLYING_B"},
    {"InformationObserverInvincible", true, "BGM_FIRST_MORPH_A", "BGM_MUTEKI_B"},
    {"InformationObserverRock", true, "BGM_FIRST_MORPH_A", nullptr},
    {"InformationObserverCloud", true, "BGM_FIRST_MORPH_A", nullptr},
    {"InformationObserverDrill", true, "BGM_FIRST_MORPH_A", nullptr},
    {"InformationObserverLifeUpMario", true, nullptr, nullptr},
    {"InformationObserverOneUp", true, nullptr, nullptr},
    {"InformationObserverYoshi", false, nullptr, nullptr},
    {"InformationObserverCometMedal", false, nullptr, nullptr},
    {"InformationObserverMarioFacePlanet", false, nullptr, nullptr},
};

namespace {
    static s32 sDisplayFramesMin = 30;
};  // namespace

namespace {
    InformationObserver* getInformationObserver() {
        return MR::getSceneObj< InformationObserver* >(SceneObj_InformationObserver);
    }
};  // namespace

namespace NrvInformationObserver {
    NEW_NERVE(InformationObserverNrvWait, InformationObserver, Wait);
    NEW_NERVE(InformationObserverNrvDisp, InformationObserver, Disp);
    NEW_NERVE(InformationObserverNrvEnd, InformationObserver, End);
};  // namespace NrvInformationObserver

InformationObserver::InformationObserver() : LiveActor("初出監視"), mType(Type_Bee), _90(false), mDisplayFrame(-1) {
}

void InformationObserver::init(const JMapInfoIter& rIter) {
    MR::connectToSceneLayoutMovement(this);
    MR::declareEventCameraProgrammable("初出監視カメラ");
    initNerve(GET_NERVE(InformationObserver, InformationObserverNrvWait));
    MR::invalidateClipping(this);
    makeActorDead();
}

void InformationObserver::entry(TYPE type, LiveActor* pParam2, bool isPauseable) {
    _94 = pParam2;
    mType = type;
    _90 = false;

    if (::fn_800210B0()) {
        if (isPauseable) {
            _90 = true;
            MR::pauseTimeKeepDemo(_94);
        }
        setNerve(GET_NERVE(InformationObserver, InformationObserverNrvDisp));
    } else {
        MR::requestStartDemoWithoutCinemaFrame(this, "初出表示", GET_NERVE(InformationObserver, InformationObserverNrvDisp),
                                               GET_NERVE(InformationObserver, InformationObserverNrvWait));
    }

    makeActorAppeared();
}

void InformationObserver::exeWait() {
}

void InformationObserver::exeDisp() {
    if (MR::isFirstStep(this)) {
        if (mType == Type_LifeUp && MR::isPlayerLuigi()) {
            MR::appearInformationMessageUp("InformationObserverLifeUpLuigi", true);
        } else {
            MR::appearInformationMessageUp(sAttr[mType].mMessageId, true);
        }

        MR::disableCloseInformationMessage();

        if (sAttr[mType].mFirstMorphBgmName != nullptr) {
            MR::stopSubBGM(0);
            MR::startSubBGM(sAttr[mType].mFirstMorphBgmName, false);
        }

        mDisplayFrame = ::sDisplayFramesMin;
    }

    if (mDisplayFrame >= 0) {
        mDisplayFrame--;
    }

    if (!MR::testCorePadTriggerA(WPAD_CHAN0) || mDisplayFrame >= 0) {
        return;
    }

    if (sAttr[mType].mBgmName != nullptr) {
        if (mType == Type_Flying) {
            MR::stopSubBGM(5);
            MR::startStageBGM(sAttr[mType].mBgmName, false);
        } else {
            MR::startSubBGM(sAttr[mType].mBgmName, false);
        }
    }

    switch (mType) {
    case Type_Bee:
        MR::onGameEventFlagBeeMarioAtFirst();
        break;
    case Type_Teresa:
        MR::onGameEventFlagTeresaMarioAtFirst();
        break;
    case Type_Hopper:
        MR::onGameEventFlagHopperMarioAtFirst();
        break;
    case Type_Fire:
        MR::onGameEventFlagFireMarioAtFirst();
        break;
    case Type_Ice:
        MR::onGameEventFlagIceMarioAtFirst();
        break;
    case Type_Flying:
        MR::onGameEventFlagFlyingMarioAtFirst();
        break;
    case Type_Invincible:
        MR::onGameEventFlagInvincibleMarioAtFirst();
        break;
    case Type_Rock:
        MR::onGameEventFlagRockMarioAtFirst();
        break;
    case Type_Cloud:
        MR::onGameEventFlagCloudMarioAtFirst();
        break;
    case Type_Drill:
        MR::onGameEventFlagDrillAtFirst();
        break;
    case Type_LifeUp:
        MR::onGameEventFlagLifeUpAtFirst();
        break;
    case Type_OneUp:
        MR::onGameEventFlagOneUpAtFirst();
        break;
    case Type_Yoshi:
        MR::onGameEventFlagYoshiAtFirst();
        break;
    case Type_CometMedal:
        MR::onGameEventFlagCometMedalAtFirst();
        break;
    default:
        break;
    }

    setNerve(GET_NERVE(InformationObserver, InformationObserverNrvEnd));
}

void InformationObserver::exeEnd() {
    if (MR::isFirstStep(this)) {
        MR::disappearInformationMessage();
    }

    if (MR::isDeadInformationMessage()) {
        if (_90) {
            MR::resumeTimeKeepDemo(_94);
        } else if (MR::isDemoActive("初出表示")) {
            MR::endDemo(this, "初出表示");
        }

        if (sAttr[mType].mFirstMorphBgmName != nullptr && sAttr[mType].mBgmName == nullptr) {
            MR::stopSubBGM(10);
        }

        makeActorDead();
    }
}

namespace InformationObserverFunction {
    bool isActive() {
        bool result = false;

        if (!MR::isDead(::getInformationObserver())) {
            InformationObserver* pObserver = ::getInformationObserver();
            bool isDispOrEnd = false;

            if (pObserver->isNerve(GET_NERVE(InformationObserver, InformationObserverNrvDisp)) ||
                pObserver->isNerve(GET_NERVE(InformationObserver, InformationObserverNrvEnd))) {
                isDispOrEnd = true;
            }

            if (isDispOrEnd) {
                result = true;
            }
        }

        return result;
    }

    void explainBee() {
        ::getInformationObserver()->entry(InformationObserver::Type_Bee, nullptr, true);
    }

    void explainTeresa() {
        ::getInformationObserver()->entry(InformationObserver::Type_Teresa, nullptr, true);
    }

    void explainHopper() {
        ::getInformationObserver()->entry(InformationObserver::Type_Hopper, nullptr, true);
    }

    void explainFire() {
        ::getInformationObserver()->entry(InformationObserver::Type_Fire, nullptr, true);
    }

    void explainIce() {
        ::getInformationObserver()->entry(InformationObserver::Type_Ice, nullptr, true);
    }

    void explainFlying() {
        ::getInformationObserver()->entry(InformationObserver::Type_Flying, nullptr, true);
    }

    void explainInvincible() {
        ::getInformationObserver()->entry(InformationObserver::Type_Invincible, nullptr, true);
    }

    void explainRock() {
        ::getInformationObserver()->entry(InformationObserver::Type_Rock, nullptr, true);
    }

    void explainCloud() {
        ::getInformationObserver()->entry(InformationObserver::Type_Cloud, nullptr, true);
    }

    void explainDrill() {
        ::getInformationObserver()->entry(InformationObserver::Type_Drill, nullptr, true);
    }

    void explainLifeUp() {
        ::getInformationObserver()->entry(InformationObserver::Type_LifeUp, nullptr, true);
    }

    void explainOneUp() {
        ::getInformationObserver()->entry(InformationObserver::Type_OneUp, nullptr, true);
    }

    void explainYoshi(LiveActor* pParam1) {
        ::getInformationObserver()->entry(InformationObserver::Type_Yoshi, pParam1, false);
    }

    void explainCometMedal(LiveActor* pParam1) {
        ::getInformationObserver()->entry(InformationObserver::Type_CometMedal, pParam1, true);
    }

    void explainMarioFacePlanet(LiveActor* pParam1) {
        ::getInformationObserver()->entry(InformationObserver::Type_MarioFacePlanet, pParam1, true);
    }
};  // namespace InformationObserverFunction
