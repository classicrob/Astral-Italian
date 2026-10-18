#include "Game/LiveActor/ActorParam.hpp"
#include "Game/Demo/DemoTitleAnim.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

// Constructed in an unsplit area (fn_ names); called directly, not through `new`.
extern "C" DemoTitleAnimParam* fn_8013CF30(DemoTitleAnimParam*);
extern "C" DemoTitleAnimSub* fn_8013CA10(DemoTitleAnimSub*, DemoTitleAnim*, DemoTitleAnimParam*);
extern "C" DemoTitleAnimSub* fn_8013CC80(DemoTitleAnimSub*, DemoTitleAnim*, DemoTitleAnimParam*);

namespace {
    const char* sEntryActorList = "EntryActorList";
};  // namespace

namespace {
    class DemoTitleAnimNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static DemoTitleAnimNrvWait sInstance;
    };
    DemoTitleAnimNrvWait DemoTitleAnimNrvWait::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(DemoTitleAnimNrvStart, DemoTitleAnim, Start);
    NEW_NERVE(DemoTitleAnimNrvLoop, DemoTitleAnim, Loop);
    NEW_NERVE(DemoTitleAnimNrvSelecting, DemoTitleAnim, Selecting);
};  // namespace

DemoTitleAnim::DemoTitleAnim(const char* pName) : LiveActor(pName), mCameraInfo(nullptr), mSubs(nullptr), mSubCount(0), mBgmStartStep(nullptr), mTitleLogoStep(nullptr), mStep(0) {
}

DemoTitleAnim::~DemoTitleAnim() {
}

void DemoTitleAnim::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "DemoTitleAnim", false);
    createSubs();
    mBgmStartStep = MR::getActorParamS32(this, "ＢＧＭ開始フレーム");
    mTitleLogoStep = MR::getActorParamS32(this, "タイトルロゴ");
    mCameraInfo = new ActorCameraInfo(-1, 0);
    MR::initAnimCamera(this, mCameraInfo, "Start");
    MR::initAnimCamera(this, mCameraInfo, "Loop");
    initNerve(GET_NERVE_ANON(DemoTitleAnimNrvWait), 0);
    MR::invalidateClipping(this);
    MR::hideModelAndOnCalcAnim(this);
    makeActorAppeared();
}

void DemoTitleAnim::control() {
    if (!isNerve(GET_NERVE_ANON(DemoTitleAnimNrvWait))) {
        mStep++;
    }
}

void DemoTitleAnim::start() {
    setNerve(GET_NERVE_ANON(DemoTitleAnimNrvStart));
}

void DemoTitleAnim::setLoop() {
    for (s32 i = 0; i < mSubCount; i++) {
        mSubs[i]->loopSub();
    }
    MR::startAction(this, "Loop");
    setNerve(GET_NERVE_ANON(DemoTitleAnimNrvLoop));
}

void DemoTitleAnim::exeStart() {
    if (MR::isFirstStep(this)) {
        mStep = 0;
        for (s32 i = 0; i < mSubCount; i++) {
            mSubs[i]->startSub();
        }
        MR::startAction(this, "Start");
        MR::startAnimCameraTargetSelf(this, mCameraInfo, "Start", 0, false, MR::getBckRate(this));
    }
    if (MR::isActionEnd(this)) {
        setLoop();
    }
}

void DemoTitleAnim::exeLoop() {
    if (MR::isFirstStep(this)) {
        MR::endAnimCamera(this, mCameraInfo, "Loop", -1, true);
        MR::startAnimCameraTargetSelf(this, mCameraInfo, "Loop", 0, false, MR::getBckRate(this));
    }
    if (MR::isActionEnd(this)) {
        setLoop();
    }
}

void DemoTitleAnim::exeSelecting() {
    if (MR::isFirstStep(this)) {
        MR::endAnimCamera(this, mCameraInfo, "Loop", -1, true);
        MR::startAction(this, "Selecting");
        for (s32 i = 0; i < mSubCount; i++) {
            mSubs[i]->selectSub();
        }
    }
}

void DemoTitleAnim::endAll() {
    MR::endAnimCamera(this, mCameraInfo, "Loop", -1, true);
    for (s32 i = 0; i < mSubCount; i++) {
        mSubs[i]->endSub();
    }
}

bool DemoTitleAnim::isBgmStartStep() const {
    return mStep == *mBgmStartStep;
}

bool DemoTitleAnim::isTitleLogoStep() const {
    return mStep >= *mTitleLogoStep;
}

void DemoTitleAnim::createSubs() {
    ResourceHolder* holder = MR::getResourceHolder(this);
    JMapInfo* csv = MR::tryCreateCsvParser(holder, "%s.bcsv", sEntryActorList);
    if (csv == nullptr) {
        return;
    }
    mSubCount = MR::getCsvDataElementNum(csv);
    if (mSubCount == 0) {
        return;
    }
    mSubs = new DemoTitleAnimSub*[mSubCount];
    for (s32 i = 0; i < mSubCount; i++) {
        DemoTitleAnimParam* param = static_cast<DemoTitleAnimParam*>(::operator new(0x14));
        if (param != nullptr) {
            param = fn_8013CF30(param);
        }
        MR::getCsvDataStr(&param->mArcName, csv, "ArciveName", i);
        MR::getCsvDataStrOrNULL(&param->mJointName, csv, "JointName", i);
        MR::getCsvDataStrOrNULL(&param->mActionName, csv, "ActionName", i);
        MR::getCsvDataStrOrNULL(&param->mLoopActionName, csv, "LoopActionName", i);
        MR::getCsvDataStrOrNULL(&param->mSelectingActionName, csv, "SelectingActionName", i);

        DemoTitleAnimSub* sub;
        if (MR::isEqualString(param->mArcName, "Player")) {
            sub = static_cast<DemoTitleAnimSub*>(::operator new(0x9C));
            if (sub != nullptr) {
                sub = fn_8013CC80(sub, this, param);
            }
        } else {
            sub = static_cast<DemoTitleAnimSub*>(::operator new(0x9C));
            if (sub != nullptr) {
                sub = fn_8013CA10(sub, this, param);
            }
        }
        mSubs[i] = sub;
        mSubs[i]->initWithoutIter();
    }
}
