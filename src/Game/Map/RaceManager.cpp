#include "Game/Map/RaceManager.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/RaceRail.hpp"
#include "Game/Map/StartCountdownLayout.hpp"
#include "Game/Screen/RaceLayout.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/RailUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <algorithm>

// FIXME: String "Record" is out of order in .data, yet function order matches in retail and debug

namespace {
    // static const _ sTimeOutSecond = _;
    static const s32 sBgmStartStep = 166;
};  // namespace

namespace {
    struct RaceStructData {
        /* 0x00 */ const char* mName;
        /* 0x04 */ const char* mGalaxyName;
        /* 0x08 */ bool mIsDemoWithStarPointer;
    };

    static const RaceStructData sRaceStruct[] = {
        {"グライダー[ジャングル]", "JungleGliderGalaxy", false},
        {"グライダー[チャレンジ]", "ChallengeGliderGalaxy", false},
    };

    const RaceStructData& getRaceStruceData(s32 id) {
        return ::sRaceStruct[id];
    }

    RaceManager* getRaceManager() {
        return MR::getSceneObj< RaceManager >(SceneObj_RaceManager);
    }
};  // namespace

namespace NrvRaceManager {
    NEW_NERVE(RaceManagerNrvWaitFirst, RaceManager, Wait);
    NEW_NERVE(RaceManagerNrvWait, RaceManager, Wait);
    NEW_NERVE(RaceManagerNrvPrepWipe, RaceManager, Prep);
    NEW_NERVE(RaceManagerNrvPrepImme, RaceManager, Prep);
    NEW_NERVE(RaceManagerNrvPreWipeOut, RaceManager, WipeOut);
    NEW_NERVE(RaceManagerNrvPreWipeWait, RaceManager, WipeWait);
    NEW_NERVE(RaceManagerNrvPreWipeIn, RaceManager, WipeIn);
    NEW_NERVE(RaceManagerNrvIntro, RaceManager, Intro);
    NEW_NERVE(RaceManagerNrvCount, RaceManager, Count);
    NEW_NERVE(RaceManagerNrvRace, RaceManager, Race);
    NEW_NERVE(RaceManagerNrvDemo, RaceManager, Demo);
    NEW_NERVE(RaceManagerNrvGoal, RaceManager, Goal);
    NEW_NERVE(RaceManagerNrvRank, RaceManager, Rank);
    NEW_NERVE(RaceManagerNrvPstWipeOut, RaceManager, WipeOut);
    NEW_NERVE(RaceManagerNrvPstWipeWait, RaceManager, WipeWait);
    NEW_NERVE(RaceManagerNrvPstWipeIn, RaceManager, WipeIn);
};  // namespace NrvRaceManager

RaceManager::RaceManager()
    : LiveActor("レース管理"), mLayout(nullptr), mRacerNum(0), mAudienceNum(0), mRank(0), mBestTime(0), mTime(0), mPlayerRacer(nullptr) {
    mStartCountdown = nullptr;
    mPlayerRide = nullptr;
    mJudge = nullptr;

}

void RaceManager::init(const JMapInfoIter& rIter) {
    MR::connectToSceneMapObjMovement(this);

    mLayout = new RaceLayout("レース管理用レイアウト");
    mLayout->init(rIter);
    mStartCountdown = new StartCountdownLayout();
    mStartCountdown->init(rIter);

    MR::invalidateClipping(this);
    initNerve(GET_NERVE(RaceManager, RaceManagerNrvWaitFirst));
    makeActorAppeared();
}

void RaceManager::movement() {
    LiveActor::movement();
}

void RaceManager::exeWait() {
    if (!isNerve(GET_NERVE(RaceManager, RaceManagerNrvWait)) && GameSequenceFunction::isResultMiss()) {
        prepareStart();
        MR::requestStartDemoMarioPuppetableWithoutCinemaFrame(this, "レース", GET_NERVE(RaceManager, RaceManagerNrvPrepImme), nullptr);
        MR::stopStageBGM(90);
        MR::startSubBGM("BGM_MINIGAME_START", false);
    }
}

void RaceManager::exePrep() {
    std::for_each(&mRacer[0], &mRacer[mRacerNum], std::mem_func(&AbstractRacer::initRacer));

    if (isNerve(GET_NERVE(RaceManager, RaceManagerNrvPrepWipe))) {
        setNerve(GET_NERVE(RaceManager, RaceManagerNrvPreWipeOut));
    } else {
        prepRace();
        setNerve(GET_NERVE(RaceManager, RaceManagerNrvIntro));
    }
}

void RaceManager::exeWipeOut() {
    if (MR::isFirstStep(this)) {
        if (isNerve(GET_NERVE(RaceManager, RaceManagerNrvPreWipeOut))) {
            MR::stopStageBGM(90);
        }

        if (!MR::isWipeBlank()) {
            MR::closeWipeFade();
        }
    }

    if (MR::isWipeActive()) {
        return;
    }

    if (isNerve(GET_NERVE(RaceManager, RaceManagerNrvPreWipeOut))) {
        prepRace();
        setNerve(GET_NERVE(RaceManager, RaceManagerNrvPreWipeWait));
    } else {
        resetRace();
        setNerve(GET_NERVE(RaceManager, RaceManagerNrvPstWipeWait));
    }
}

void RaceManager::exeWipeWait() {
    if (!MR::isStep(this, 30)) {
        return;
    }

    if (isNerve(GET_NERVE(RaceManager, RaceManagerNrvPreWipeWait))) {
        setNerve(GET_NERVE(RaceManager, RaceManagerNrvPreWipeIn));
    } else {
        setNerve(GET_NERVE(RaceManager, RaceManagerNrvPstWipeIn));
    }
}

void RaceManager::exeWipeIn() {
    if (MR::isFirstStep(this)) {
        if (isNerve(GET_NERVE(RaceManager, RaceManagerNrvPreWipeIn))) {
            if (mJudge == nullptr) {
                MR::startSubBGM("BGM_MINIGAME_START", false);
            }
        } else {
            MR::startCameraInterpolation(0);
            if (!MR::isPlayerDead()) {
                MR::startCurrentStageBGM(false);
            }
        }

        MR::openWipeFade();
    }

    if (MR::isWipeActive()) {
        return;
    }

    if (isNerve(GET_NERVE(RaceManager, RaceManagerNrvPreWipeIn))) {
        if (mJudge != nullptr) {
            if (!mJudge->isJudgeReady()) {
                return;
            }
            MR::startSubBGM("BGM_MINIGAME_START", false);
        }
        setNerve(GET_NERVE(RaceManager, RaceManagerNrvIntro));
    } else {
        MR::onPlayerControl(true);
        MR::endDemo(this, "レース");
        setNerve(GET_NERVE(RaceManager, RaceManagerNrvWait));
        std::for_each(&mRacer[0], &mRacer[mRacerNum], std::mem_func(&AbstractRacer::exitRacer));
    }
}

void RaceManager::exeIntro() {
    if (MR::isFirstStep(this)) {
        if (::getRaceStruceData(mCurrentRace).mIsDemoWithStarPointer) {
            MR::startStarPointerModeDemoWithStarPointer(this);
        } else {
            MR::startStarPointerModeDemo(this);
        }
    }

    if (MR::isStep(this, 110)) {
        setNerve(GET_NERVE(RaceManager, RaceManagerNrvCount));
    }
}

void RaceManager::exeCount() {
    if (MR::isFirstStep(this)) {
        mLayout->setTime(0);
        mLayout->setBestTime(mBestTime);
        mLayout->appear();
        mStartCountdown->appear();
    }
    mLayout->movement();
    if (mStartCountdown->isStart()) {
        startRace();
        setNerve(GET_NERVE(RaceManager, RaceManagerNrvRace));
    }
}

void RaceManager::exeRace() {
    mLayout->setTime(getNerveStep());

    if (mRacerNum > 1) {
        mLayout->playRank(getRank(mPlayerRacer));
    }

    for (s32 idx = mRank; idx < mRacerNum; idx++) {
        AbstractRacer* racer = mRacer[idx];
        if (racer->updateRacer(this)) {
            std::swap(mRacer[idx], mRacer[mRank++]);
            if (racer->goalRacer()) {
                renewTime();
                setNerve(GET_NERVE(RaceManager, RaceManagerNrvDemo));
                break;
            }
        }
    }

    std::sort(&mRacer[mRank], &mRacer[mRacerNum], AbstractRacer::compRacer);
}

void RaceManager::exeDemo() {
    if (!MR::canStartDemo()) {
        return;
    }

    goalRace();
    MR::offPlayerControl();
    setNerve(GET_NERVE(RaceManager, RaceManagerNrvGoal));
}

void RaceManager::exeGoal() {
    if (MR::isFirstStep(this)) {
        MR::stopStageBGM(30);

        if (mRank == 0) {
            MR::startSubBGM("BGM_RACE_LOSE", false);
        } else if (mRank == 1) {
            MR::startSubBGM("BGM_RACE_WIN", false);
            if (mTime < mBestTime) {
                mLayout->playGoalWithTime(mTime);
            } else {
                mLayout->playGoal();
            }
        } else {
            MR::startSubBGM("BGM_RACE_LOSE", false);
            mLayout->playLose();
        }
    }

    if (mLayout->isAllAnimStopped()) {
        setNerve(GET_NERVE(RaceManager, RaceManagerNrvRank));
    }
}

void RaceManager::exeRank() {
    if (MR::isLessStep(this, 60)) {
        return;
    }

    if (!MR::canStartDemo()) {
        return;
    }

    MR::tryStartDemoWithoutCinemaFrame(this, "レース");
    setNerve(GET_NERVE(RaceManager, RaceManagerNrvPstWipeOut));
}

void RaceManager::prepRace() {
    mPlayerRacer->prepRacer(this);
    prepBind();

    for (s32 i = 0; i < mRacerNum; i++) {
        AbstractRacer* pRacer = mRacer[i];

        if (pRacer != mPlayerRacer) {
            pRacer->prepRacer(this);
        }
    }

    std::for_each(&mAudience[0], &mAudience[mAudienceNum], std::mem_func(&AbstractAudience::prepAudience));
}

void RaceManager::startRace() {
    if (mPlayerRide != nullptr) {
        MR::sendSimpleMsgToActor(0xEF, mPlayerRide);
    }

    std::for_each(&mRacer[0], &mRacer[mRacerNum], std::mem_func(&AbstractRacer::startRacer));
    std::for_each(&mAudience[0], &mAudience[mAudienceNum], std::mem_func(&AbstractAudience::startAudience));
    MR::endStarPointerMode(this);
    MR::endDemo(this, "レース");
    MR::onPlayerControl(true);
}

void RaceManager::goalRace() {
    if (mPlayerRide != nullptr) {
        MR::sendSimpleMsgToActor(0xF0, mPlayerRide);
    }

    std::for_each(&mRacer[mRank], &mRacer[mRacerNum], std::mem_func(&AbstractRacer::loseRacer));
}

void RaceManager::resetRace() {
    if (mPlayerRide != nullptr && MR::isPlayerInRush()) {
        MR::sendSimpleMsgToActor(0xF1, mPlayerRide);
    }

    mPlayerRacer->resetRacer(this);

    for (s32 i = 0; i < mRacerNum; i++) {
        AbstractRacer* pRacer = mRacer[i];

        if (pRacer != mPlayerRacer) {
            pRacer->resetRacer(this);
        }
    }

    std::for_each(&mAudience[0], &mAudience[mAudienceNum], std::mem_func(&AbstractAudience::resetAudience));
    mLayout->kill();
}

void RaceManager::prepBind() {
    if (mPlayerRide != nullptr) {
        MR::requestMovementOn(mPlayerRide);
        MR::sendSimpleMsgToActor(0xEE, mPlayerRide);
    }
}

void RaceManager::updateRank() {
    std::for_each(&mAudience[0], &mAudience[mAudienceNum], std::mem_func(&AbstractAudience::exitAudience));
    std::for_each(&mRacer[mRank], &mRacer[mRacerNum], std::mem_func(&AbstractRacer::requestMovementOnRace));
}

void RaceManager::entry(AbstractRacer* pRacer) {
    mRacer[mRacerNum++] = pRacer;
}

void RaceManager::entry(PlayerRacer* pRacer) {
    MR::getRailArg0WithInit(pRacer, &mCurrentRace);
    mBestTime = MR::getRaceBestTime(mCurrentRace);
    mPlayerRacer = pRacer;
    entry(static_cast< AbstractRacer* >(pRacer));
}

void RaceManager::entry(AbstractAudience* pAudience) {
    mAudience[mAudienceNum++] = pAudience;
}

bool RaceManager::isGoal(const AbstractRacer* pRacer) const {
    TVec3f endPos(0.0f, 0.0f, 0.0f);
    MR::calcRailEndPointPos(&endPos, mPlayerRacer);
    return pRacer->mCurrPosition.z < endPos.z && endPos.z <= pRacer->mPrevPosition.z;

}

u32 RaceManager::getRank(const AbstractRacer* pRacer) const {
    u32 rank = mRacerNum;

    for (int i = 0; i < mRacerNum; i++) {
        if (mRacer[i] == pRacer) {
            rank = i;
        }
    }

    return rank + 1;
}

void RaceManager::renewTime() {
    mTime = getNerveStep();

    if (mTime < mBestTime) {
        MR::setRaceBestTime(mCurrentRace, mTime);
    }
}

void RaceManager::prepareStart() {
    MR::validateHitSensors(mPlayerRide);
    mRank = 0;
    mLayout->hideAllPane();
}

namespace RaceManagerFunction {
    void entryRacerOthers(AbstractRacer* pRacer) {
        MR::createSceneObj(SceneObj_RaceManager);
        ::getRaceManager()->entry(pRacer);
    }

    void entryAudience(AbstractAudience* pAudience) {
        MR::createSceneObj(SceneObj_RaceManager);
        ::getRaceManager()->entry(pAudience);
    }

    void entryRacerPlayer(PlayerRacer* pRacer) {
        MR::createSceneObj(SceneObj_RaceManager);
        ::getRaceManager()->entry(pRacer);
    }

    void entryPlayerRide(LiveActor* pRide) {
        MR::createSceneObj(SceneObj_RaceManager);
        ::getRaceManager()->mPlayerRide = pRide;
    }

    void entryJudge(AbstractRaceJudge* pJudge) {
        MR::createSceneObj(SceneObj_RaceManager);
        ::getRaceManager()->mJudge = pJudge;
    }

    void startRaceWithWipe() {
        RaceManager* pManager = ::getRaceManager();
        pManager->prepareStart();
        MR::requestStartDemoMarioPuppetableWithoutCinemaFrame(pManager, "レース", GET_NERVE(RaceManager, RaceManagerNrvPrepWipe), nullptr);
    }

    void startRaceImmediately() {
        RaceManager* pManager = ::getRaceManager();
        pManager->prepareStart();
        MR::requestStartDemoMarioPuppetableWithoutCinemaFrame(pManager, "レース", GET_NERVE(RaceManager, RaceManagerNrvPrepImme), nullptr);
        MR::stopStageBGM(90);
        MR::startSubBGM("BGM_MINIGAME_START", false);
    }

    u32 getRaceRank() {
        return ::getRaceManager()->mRank;
    }

    u32 getRaceTime() {
        return ::getRaceManager()->mTime;
    }

    void updateRank() {
        ::getRaceManager()->updateRank();
    }

    void killRace() {
        if (MR::isExistSceneObj(SceneObj_RaceManager)) {
            RaceManager* pManager = ::getRaceManager();
            pManager->mLayout->kill();
            pManager->kill();
        }
    }

    const char* getRaceName(int index) {
        return ::getRaceStruceData(index).mName;
    }

    s32 getRaceId(const char* pGalaxyName) {
        for (u32 i = 0; i < ARRAY_SIZE(::sRaceStruct); i++) {
            if (MR::isEqualString(::sRaceStruct[i].mGalaxyName, pGalaxyName)) {
                return i;
            }
        }

        return -1;
    }
};  // namespace RaceManagerFunction

