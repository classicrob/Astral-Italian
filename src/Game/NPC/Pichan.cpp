#include "Game/NPC/Pichan.hpp"
#include "Game/NPC/TalkMessageFunc.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Ride/GliBird.hpp"
#include "Game/Util.hpp"

namespace {
    const char* cFinishPosition = "ピーチャン位置[スター渡し]";
}

namespace NrvPichan {
    NEW_NERVE(PichanNrvWait, Pichan, Wait);
    class PichanNrvTutorial : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static PichanNrvTutorial sInstance;
    };
    PichanNrvTutorial PichanNrvTutorial::sInstance ATTRIBUTE_WEAK;
    class PichanNrvRace : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static PichanNrvRace sInstance;
    };
    PichanNrvRace PichanNrvRace::sInstance ATTRIBUTE_WEAK;
    class PichanNrvTakeOutStar : public Nerve {
    public:
        virtual void execute(Spine* spine) const {
            if (MR::isFirstStep(static_cast<Pichan*>(spine->mExecutor))) {
                RaceManagerFunction::updateRank();
            }
        }
        static PichanNrvTakeOutStar sInstance;
    };
    PichanNrvTakeOutStar PichanNrvTakeOutStar::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(PichanNrvTalk, Pichan, Talk);
    class PichanNrvRaceEnd : public Nerve {
    public:
        virtual void execute(Spine* spine) const {
            Pichan* actor = static_cast<Pichan*>(spine->mExecutor);
            if (MR::isFirstStep(actor)) {
                RaceManagerFunction::updateRank();
            }
            MR::tryTalkForceAtEndAndStartTalkAction(actor);
        }
        static PichanNrvRaceEnd sInstance;
    };
    PichanNrvRaceEnd PichanNrvRaceEnd::sInstance ATTRIBUTE_WEAK;
}

#define PICHAN_NERVE(name) (&NrvPichan::PichanNrv##name::sInstance)

Pichan::Pichan(const char* name) : NPCActor(name), mTakeOutStar(nullptr), mIsTutorial(false) {}

// NONMATCHING: talk controller loads are scheduled after callback construction.
void Pichan::init(const JMapInfoIter& iter) {
    NPCActorCaps caps("Pichan");
    caps.setDefault();
    caps.mReactionNerve = PICHAN_NERVE(Wait);
    mParam.setMoveAction("Wait", "Wait");
    mParam.setTalkAction("Talk", "Talk");
    _130=cNPCSpinDefault; _134=cNPCTrampledDefault;
    _138=cNPCPointingDefault; _13C=cNPCReactionDefault;
    _12C = 450.0f;
    initialize(iter, caps, "Pichan", nullptr, nullptr);
    MR::registerEventFunc(mMsgCtrl, TalkMessageFunc(this, &Pichan::eventFunc));
    MR::registerBranchFunc(mMsgCtrl, TalkMessageFunc(this, &Pichan::branchFunc));
    MR::registerAnimeFunc(mMsgCtrl, TalkMessageFunc(this, &Pichan::animeFunc));
    bool hasStar = false;
    MR::getJMapInfoArg0NoInit(iter, &hasStar);
    if (hasStar) {
        mTakeOutStar = new TakeOutStar(this, "TakeOutStarPichan", "TakeOutStarPichan", PICHAN_NERVE(TakeOutStar), mTalkNerve);
        MR::declarePowerStar(this);
    }
    MR::initUseStageSwitchWriteA(this, iter);
    if (MR::isStageGlider()) {
        bool tutorial = false;
        MR::getJMapInfoArg1NoInit(iter, &tutorial);
        if (tutorial) {
            MR::registerPichanForTutorial(this);
        }
        RaceManagerFunction::entryAudience(this);
    }
    makeActorAppeared();
}

bool Pichan::branchFunc(u32 id) {
    if (id == 0) {
        return RaceManagerFunction::getRaceRank() != 0;
    }
    if (id == 1) return MR::isOnGameEventFlagGliBirdTutorialAtFirst();
    if (id == 2) return RaceManagerFunction::getRaceRank() == 1;
    return false;
}

bool Pichan::eventFunc(u32 id) {
    if (id == 0) {
        RaceManagerFunction::startRaceWithWipe();
    } else if (id == 1) {
        return mTakeOutStar->takeOut();
    } else if (id == 2) {
        RaceManagerFunction::entryJudge(this);
        MR::startGliBirdTutorialTalk();
        setNerve(PICHAN_NERVE(Tutorial));
        return true;
    } else if (id == 3) {
        RaceManagerFunction::updateRank();
    }
    return true;
}

bool Pichan::animeFunc(u32 id) {
    switch (id) {
    case 0: MR::startAction(this, "Talk"); break;
    case 1: MR::startAction(this, "TalkAngry"); break;
    case 2: MR::startAction(this, "TalkCry"); break;
    case 3: MR::startAction(this, "TalkSmile"); break;
    }
    return true;
}

void Pichan::prepAudience() { MR::requestMovementOn(this); }
void Pichan::startAudience() { setNerve(PICHAN_NERVE(Race)); }

void Pichan::resetAudience() {
    MR::requestMovementOn(this);
    if (mTakeOutStar != nullptr) {
        TVec3f pos(0.0f);
        if (MR::tryFindNamePos(cFinishPosition, &pos, nullptr)) {
            MR::setPosition(this, pos);
        }
        setNerve(PICHAN_NERVE(RaceEnd));
        return;
    }
    setNerve(mTalkNerve);
}

bool Pichan::isJudgeReady() const { return isNerve(mTalkNerve); }
void Pichan::exitAudience() { MR::requestMovementOn(this); }

void Pichan::exeWait() {
    if (!_E4 && MR::isGreaterEqualStep(this, 70) && MR::tryTalkNearPlayer(mMsgCtrl)) {
        popNerve();
        return;
    }
    if (MR::tryStartReactionAndPopNerve(this)) return;
}

void Pichan::exeTalk() {
    MR::tryStartTalkAction(this);
    if (MR::tryTalkForceWithoutDemoAtEnd(mMsgCtrl)) {
        setNerve(mTalkNerve);
    }
}

Pichan::~Pichan() {}

void NrvPichan::PichanNrvTutorial::execute(Spine* spine) const {
    Pichan* actor = static_cast<Pichan*>(spine->mExecutor);
    if (MR::isEndGliBirdTutorialTalk()) {
        RaceManagerFunction::startRaceWithWipe();
        actor->setNerve(PICHAN_NERVE(Talk));
    }
}
