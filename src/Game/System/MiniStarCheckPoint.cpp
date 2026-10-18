#include "Game/System/GameDataFunction.hpp"
#include "Game/System/MiniStarCheckPoint.hpp"
#include "Game/System/MiniWorldMapMark.hpp"
#include "Game/System/WorldMapFunction.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

class TalkMessageCtrl;


// PLACEHOLDER(core): unnamed SMG2 callees and helpers (no symbol name yet); resolve when their owners name them
extern "C" bool fn_804E5A10(const char*);
extern "C" void fn_804E5CE0(MiniStarCheckPoint*);
extern "C" void fn_804E5EF0(MiniStarCheckPoint*);

namespace NrvMiniStarCheckPoint {
    class MiniStarCheckPointNrvClose : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            MiniStarCheckPoint* pActor = reinterpret_cast<MiniStarCheckPoint*>(pSpine->mExecutor);
            if (MR::isFirstStep(pActor)) {
                MR::startAction(pActor, "Close");
            }
        };
        static MiniStarCheckPointNrvClose sInstance;
    };
    MiniStarCheckPointNrvClose MiniStarCheckPointNrvClose::sInstance ATTRIBUTE_WEAK;

    class MiniStarCheckPointNrvStandby : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            MiniStarCheckPoint* pActor = reinterpret_cast<MiniStarCheckPoint*>(pSpine->mExecutor);
            if (MR::isFirstStep(pActor)) {
                MR::startAction(pActor, "Standby");
            }
        };
        static MiniStarCheckPointNrvStandby sInstance;
    };
    MiniStarCheckPointNrvStandby MiniStarCheckPointNrvStandby::sInstance ATTRIBUTE_WEAK;

    class MiniStarCheckPointNrvOpen : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            fn_804E5EF0(reinterpret_cast<MiniStarCheckPoint*>(pSpine->mExecutor));
        };
        static MiniStarCheckPointNrvOpen sInstance;
    };
    MiniStarCheckPointNrvOpen MiniStarCheckPointNrvOpen::sInstance ATTRIBUTE_WEAK;

    class MiniStarCheckPointNrvDone : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {};
        static MiniStarCheckPointNrvDone sInstance;
    };
    MiniStarCheckPointNrvDone MiniStarCheckPointNrvDone::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvMiniStarCheckPoint

namespace {
    TVec3f sTalkBalloonOffset(0.0f, 2500.0f, 0.0f);
    TVec3f sZeroOffset(0.0f, 0.0f, 0.0f);
};  // namespace

MiniStarCheckPoint::~MiniStarCheckPoint() {}

void MiniStarCheckPoint::init(const JMapInfoIter& rIter) {
    if (fn_804E5A10(mKind)) {
        MR::initActor(this, rIter, "MiniStarCheckPointGrand", false);
    } else {
        MR::initActor(this, rIter, "MiniStarCheckPoint", false);
    }

    mTalkCtrl = MR::createTalkCtrlDirect(this, "WorldMapEventStarCheckPointRequestStar", TVec3f(0.0f, 0.0f, 0.0f), nullptr);
    MR::setMessageArg(mTalkCtrl, mStarNum);
    MR::setMessageBalloonFollowOffset(mTalkCtrl, sTalkBalloonOffset);

    if (mPointerTarget == nullptr) {
        TVec3f pointerOffset;
        pointerOffset.set(0.0f, 0.0f, 0.0f);
        MR::initStarPointerTarget(this, 800.0f, pointerOffset);
    }

    mChild = new MiniWorldMapMark(this, "MiniStarCheckPointMark");

    initWithoutIter();
    initNerve(&NrvMiniStarCheckPoint::MiniStarCheckPointNrvClose::sInstance, 0);

    if (mStarNum <= MR::getPowerStarNum()) {
        mIsDone = GameDataFunction::isOnWorldMapPointFlag(mUnkA0, mUnk98);
        if (mIsDone) {
            fn_804E5CE0(this);
        } else {
            setNerve(&NrvMiniStarCheckPoint::MiniStarCheckPointNrvStandby::sInstance);
            makeActorAppeared();
        }
    } else {
        makeActorAppeared();
    }
}

void MiniStarCheckPoint::makeActorAppeared() {
    if (mIsDone) {
        fn_804E5CE0(this);
    } else {
        LiveActor::makeActorAppeared();
    }
}

void MiniStarCheckPoint::makeActorDead() {
    mChild->makeActorDead();
    LiveActor::makeActorDead();
}

void MiniStarCheckPoint::kill() {
    MR::tryForceDeleteEffectAll(this);
    LiveActor::kill();
}

// PLACEHOLDER(core): unnamed SMG2 callees (no symbol name yet); resolve when their owners name them
extern "C" void fn_804E5CE0(MiniStarCheckPoint* pActor) {
    pActor->mIsDone = true;
    pActor->LiveActor::makeActorDead();
    pActor->mChild->appear();
    pActor->setNerve(&NrvMiniStarCheckPoint::MiniStarCheckPointNrvDone::sInstance);
}

void MiniStarCheckPoint::control() {
    WorldMapFunction::returnScale(this, 0.05f, 1.0f);
}

void MiniStarCheckPoint::calcAndSetBaseMtx() {
    TPos3f mtx;
    MR::makeMtxTransRotateY(mtx, mPosition.x + sZeroOffset.x, mPosition.y + sZeroOffset.y,
                            mPosition.z + sZeroOffset.z, mRotation.y);
    MR::setBaseTRMtx(this, mtx);
}

void MiniStarCheckPoint::startOpen() {
    setNerve(&NrvMiniStarCheckPoint::MiniStarCheckPointNrvOpen::sInstance);
}

void MiniStarCheckPoint::resetOpen() {
    if (isDone()) {
        GameDataFunction::onWorldMapPointFlag(mUnkA0, mUnk98);
    } else {
        GameDataFunction::offWorldMapPointFlag(mUnkA0, mUnk98);
    }
}

bool MiniStarCheckPoint::isDone() const {
    if (isNerve(&NrvMiniStarCheckPoint::MiniStarCheckPointNrvOpen::sInstance)) {
        return true;
    }
    if (isNerve(&NrvMiniStarCheckPoint::MiniStarCheckPointNrvDone::sInstance)) {
        return true;
    }
    return false;
}

bool MiniStarCheckPoint::isStandby() const {
    if (isDone()) {
        return false;
    }
    if (fn_804E5A10(mKind)) {
        return MR::hasGrandStar(mStarNum);
    }
    return MR::getPowerStarNum() >= mStarNum;
}

void MiniStarCheckPoint::talk() {
    MR::tryTalkForce(mTalkCtrl);
}

bool MiniStarCheckPoint::isOpening() const {
    return WorldMapFunction::isPointing(const_cast<MiniStarCheckPoint*>(this));
}

bool MiniStarCheckPoint::isReadyToOpen() const {
    WorldMapFunction::approachScale(const_cast<MiniStarCheckPoint*>(this), 0.1f, 1.4f);
    MR::tryTalkForce(mTalkCtrl);
    return true;
}

bool MiniStarCheckPoint::update(s32 distance) {
    if (isDone()) {
        return false;
    }
    if (isStandby()) {
        if (distance == 0) {
            MR::startActionSound(this, "Shine", -1, -1, -1);
            return true;
        }
        if (distance == 1) {
            MR::startActionSound(this, "Shine", -1, -1, -1);
            return true;
        }
        return false;
    }
    if (distance == 0) {
        return true;
    }
    if (distance == 1) {
        return true;
    }
    return false;
}

void MiniStarCheckPoint::setEffectDrawOrder(s32 drawOrder) {
    MR::setAllEffectDrawOrder(this, drawOrder);
}
