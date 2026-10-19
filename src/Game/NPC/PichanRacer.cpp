#include "Game/NPC/PichanRacer.hpp"
#include "Game/NPC/TalkMessageFunc.hpp"
#include "Game/Map/RaceRail.hpp"
#include "Game/Player/RaceDataReader.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include <cstdio>
#include <cmath>

namespace {
    const char* cFinishPosition = "ピーチャンレーサーレース終了後位置";
    const char* getRaceBlurEffectName(s32 color) NO_INLINE {
        if (color == 0) return "RaceBlurLeader";
        if (color == 4) return "RaceBlurBlue";
        if (color == 1) return "RaceBlurRed";
        if (color == 2) return "RaceBlurPurple";
        if (color == 3) return "RaceBlurGreen";
        return "RaceBlurLeader";
    }
}
namespace NrvPichanRacer {
    NEW_NERVE(PichanRacerNrvWait, PichanRacer, Wait);
    NEW_NERVE(PichanRacerNrvTalk, PichanRacer, Talk);
    class PichanRacerNrvStartWait : public Nerve {
    public:
        virtual void execute(Spine* spine) const {
            PichanRacer* actor = static_cast<PichanRacer*>(spine->mExecutor);
            if (MR::isFirstStep(actor)) MR::startAction(actor, "GliBirdStartWait");
        }
        static PichanRacerNrvStartWait sInstance;
    };
    PichanRacerNrvStartWait PichanRacerNrvStartWait::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(PichanRacerNrvStart, PichanRacer, Start);
    NEW_NERVE(PichanRacerNrvStartEnd, PichanRacer, StartEnd);
    class PichanRacerNrvGoal : public Nerve {
    public:
        virtual void execute(Spine* spine) const {
            static_cast<PichanRacer*>(spine->mExecutor)->mRaceDataReader->receivePacket();
        }
        static PichanRacerNrvGoal sInstance;
    };
    PichanRacerNrvGoal PichanRacerNrvGoal::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(PichanRacerNrvTakeOutStar, PichanRacer, Goal);
    class PichanRacerNrvRaceEnd : public Nerve {
    public:
        virtual void execute(Spine* spine) const {
            PichanRacer* actor = static_cast<PichanRacer*>(spine->mExecutor);
            if (!MR::tryStartReactionAndPushNerve(actor, actor->mReactionNerve)) {
                MR::tryTalkNearPlayerAtEndAndStartMoveTalkAction(actor);
            }
        }
        static PichanRacerNrvRaceEnd sInstance;
    };
    PichanRacerNrvRaceEnd PichanRacerNrvRaceEnd::sInstance ATTRIBUTE_WEAK;
    class PichanRacerNrvRankWait : public Nerve {
    public:
        virtual void execute(Spine* spine) const {
            if (MR::isFirstStep(static_cast<PichanRacer*>(spine->mExecutor))) RaceManagerFunction::updateRank();
        }
        static PichanRacerNrvRankWait sInstance;
    };
    PichanRacerNrvRankWait PichanRacerNrvRankWait::sInstance ATTRIBUTE_WEAK;
}
#define RACER_NERVE(name) (&NrvPichanRacer::PichanRacerNrv##name::sInstance)
PichanRacer::PichanRacer(const char* name) : NPCActor(name), AbstractRacer(), mColor(2), mRaceDataReader(nullptr), mTakeOutStar(nullptr) {}
// NONMATCHING: talk controller loads are scheduled after callback construction.
void PichanRacer::init(const JMapInfoIter& iter) {
    NPCActorCaps caps("PichanRacer");
    caps.setDefault();
    caps.mWaitNerve = RACER_NERVE(Wait);
    initialize(iter, caps, "PichanRacer", nullptr, nullptr);
    mParam.setMoveAction("Wait", "Wait");
    mParam.setTalkAction("TalkAngry", "TalkAngry");
    _130=cNPCSpinDefault; _134=cNPCTrampledDefault;
    _138=cNPCPointingDefault; _13C=cNPCReactionDefault;
    _12C = 450.0f;
    MR::registerBranchFunc(mMsgCtrl, TalkMessageFunc(this, &PichanRacer::branchFunc));
    MR::registerEventFunc(mMsgCtrl, TalkMessageFunc(this, &PichanRacer::eventFunc));
    MR::registerNamePosLinkObj(this, iter, cFinishPosition);
    s32 index = -1;
    MR::getJMapInfoArg7NoInit(iter, &index);
    char filename[256];
    if (index >= 0) snprintf(filename, sizeof(filename), "PichanRacerRaceData%03d.gst", index);
    else filename[0] = 0;
    mRaceDataReader = new RaceDataReader(this, filename, nullptr);
    RaceManagerFunction::entryRacerOthers(this);
    MR::getJMapInfoArg0NoInit(iter, &mColor);
    MR::startBrk(this, "ColorChange");
    MR::setBrkFrameAndStop(this, static_cast<f32>(mColor));
    if (mColor == 0) {
        MR::declarePowerStar(this);
        mTakeOutStar = new TakeOutStar(this, "TakeOutStarPichanRacer", "TakeOutStarPichanRacer", RACER_NERVE(RankWait), nullptr);
    }
    makeActorAppeared();
}
void PichanRacer::exeWait() {
    if (!MR::tryStartReactionAndPushNerve(this, mReactionNerve) && MR::tryTalkNearPlayerAndStartTalkAction(this) && mColor == 0) setNerve(RACER_NERVE(Talk));
}
void PichanRacer::exeTalk() {
    if (MR::tryTalkNearPlayerAtEndAndStartTalkAction(this)) {
        if (MR::tryTalkSelectLeft(mMsgCtrl)) {
            RaceManagerFunction::startRaceWithWipe();
            setNerve(RACER_NERVE(Wait));
            return;
        }
        setNerve(RACER_NERVE(Wait));
    }
}
void PichanRacer::exeStart() {
    if (MR::isFirstStep(this)) MR::startAction(this, "GliBirdStart");
    if (MR::isActionEnd(this)) setNerve(RACER_NERVE(StartEnd));
}
void PichanRacer::exeStartEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "GliBirdStartEnd");
        MR::emitEffect(this, getRaceBlurEffectName(mColor));
    }
    mRaceDataReader->receivePacket();
    f32 frame = MR::getBckFrame(this);
    f32 weights[3];
    MR::copyBckBlendWeights(weights, 3, this);
    if (static_cast<s32>(frame) % 12 == 0) MR::startActionSound(this, "RaceFlap", static_cast<s32>(100.0f * weights[0]), static_cast<s32>(100.0f * weights[1]), -1);
    if (MR::isGreaterStep(this, 60)) MR::startActionSound(this, "RaceFly", static_cast<s32>(100.0f * PSVECMag(&mVelocity)), -1, -1);
}
void PichanRacer::exeGoal() {
    if (!MR::tryTalkForceAtEndAndStartTalkAction(this)) {
        if (MR::isFirstStep(this)) RaceManagerFunction::updateRank();
    } else {
        if (RaceManagerFunction::getRaceRank() == 1) {
            setNerve(RACER_NERVE(RaceEnd));
            return;
        }
        MR::forceKillPlayerByGroundRace();
    }
}
void PichanRacer::prepRacer(const RaceManager*) {
    setNerve(RACER_NERVE(StartWait));
    mRaceDataReader->receivePacket();
    MR::invalidateClipping(this);
    MR::requestMovementOn(this);
}
void PichanRacer::startRacer() { setNerve(RACER_NERVE(StartEnd)); }
bool PichanRacer::updateRacer(const RaceManager* manager) {
    mRailCoord = MR::calcNearestRailCoord(manager->mPlayerRacer, mPosition);
    mPrevPosition.set(mCurrPosition);
    mCurrPosition.set(mPosition);
    if (static_cast<f32>(__fabs(mRailCoord - MR::getRailTotalLength(manager->mPlayerRacer))) < 1000.0f) return manager->isGoal(this);
    return false;
}
bool PichanRacer::goalRacer() { setNerve(RACER_NERVE(Goal)); return false; }
void PichanRacer::loseRacer() {}
void PichanRacer::resetRacer(const RaceManager*) {
    MR::tryFindLinkNamePos(this, cFinishPosition, &mPosition, &mRotation);
    mVelocity.zero();
    setNerve(RACER_NERVE(RaceEnd));
    MR::requestMovementOn(this);
}
void PichanRacer::exitRacer() {
    MR::validateClipping(this);
    MR::deleteEffect(this, getRaceBlurEffectName(mColor));
    if (mColor == 0) { setNerve(RACER_NERVE(TakeOutStar)); return; }
    setNerve(RACER_NERVE(RaceEnd));
}
void PichanRacer::requestMovementOnRace() { MR::requestMovementOn(this); }
bool PichanRacer::branchFunc(u32 id) {
    if (id == 0) return RaceManagerFunction::getRaceRank() != 0;
    if (id == 1) return MR::isOnGameEventFlagGliBirdTutorialAtFirst();
    if (id == 2) return RaceManagerFunction::getRaceRank() == 1;
    return false;
}
bool PichanRacer::eventFunc(u32 id) {
    if (id == 0) return mTakeOutStar->takeOut();
    if (id == 3) RaceManagerFunction::updateRank();
    return true;
}
PichanRacer::~PichanRacer() {}
