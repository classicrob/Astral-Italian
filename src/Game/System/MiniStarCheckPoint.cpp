#include "Game/System/MiniStarCheckPoint.hpp"
#include "Game/System/MiniWorldMapMark.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

class TalkMessageCtrl;


extern "C" bool fn_804E5A10(const char*);
extern "C" bool fn_804D3E20(s32, s32);
extern "C" void fn_804D3E70(s32, s32);
extern "C" void fn_804D3EC0(s32, s32);
extern "C" void fn_804E8C40(LiveActor*);
extern "C" void fn_804E8C50(LiveActor*, f32, f32);
extern "C" void fn_804E8C80(LiveActor*, f32, f32);
extern "C" void fn_804E5CE0(MiniStarCheckPoint*);
extern "C" bool fn_804E5E20(MiniStarCheckPoint*);
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
        mIsDone = fn_804D3E20(mUnkA0, mUnk98);
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

extern "C" void fn_804E5CE0(MiniStarCheckPoint* pActor) {
    pActor->mIsDone = true;
    pActor->LiveActor::makeActorDead();
    pActor->mChild->appear();
    pActor->setNerve(&NrvMiniStarCheckPoint::MiniStarCheckPointNrvDone::sInstance);
}

void MiniStarCheckPoint::control() {
    fn_804E8C80(this, 0.05f, 1.0f);
}

void MiniStarCheckPoint::calcAndSetBaseMtx() {
    TPos3f mtx;
    MR::makeMtxTransRotateY(mtx, mPosition.x + sZeroOffset.x, mPosition.y + sZeroOffset.y,
                            mPosition.z + sZeroOffset.z, mRotation.y);
    MR::setBaseTRMtx(this, mtx);
}

extern "C" void fn_804E5DC0(MiniStarCheckPoint* pActor) {
    pActor->setNerve(&NrvMiniStarCheckPoint::MiniStarCheckPointNrvOpen::sInstance);
}

extern "C" void fn_804E5DD0(MiniStarCheckPoint* pActor) {
    if (fn_804E5E20(pActor)) {
        fn_804D3E70(pActor->mUnkA0, pActor->mUnk98);
    } else {
        fn_804D3EC0(pActor->mUnkA0, pActor->mUnk98);
    }
}

extern "C" bool fn_804E5E20(MiniStarCheckPoint* pActor) {
    if (pActor->isNerve(&NrvMiniStarCheckPoint::MiniStarCheckPointNrvOpen::sInstance)) {
        return true;
    }
    if (pActor->isNerve(&NrvMiniStarCheckPoint::MiniStarCheckPointNrvDone::sInstance)) {
        return true;
    }
    return false;
}

extern "C" bool fn_804E5E80(MiniStarCheckPoint* pActor) {
    if (fn_804E5E20(pActor)) {
        return false;
    }
    if (fn_804E5A10(pActor->mKind)) {
        return MR::hasGrandStar(pActor->mStarNum);
    }
    return MR::getPowerStarNum() >= pActor->mStarNum;
}

extern "C" void fn_804E5EF0(MiniStarCheckPoint* pActor) {
    if (MR::isFirstStep(pActor)) {
        if (pActor->mStarNum <= 3) {
            MR::startAction(pActor, "OpenS");
        } else if (pActor->mStarNum <= 8) {
            MR::startAction(pActor, "OpenM");
        } else {
            MR::startAction(pActor, "OpenL");
        }
        MR::startActionSound(pActor, "Burst", -1, -1, -1);
    }

    if (MR::isLessStep(pActor, 0x8C)) {
        MR::tryRumblePadVeryWeak(pActor, 0);
    }

    if (MR::isStep(pActor, 0x8C)) {
        MR::tryRumblePadWeak(pActor, 0);
        MR::shakeCameraNormal();
        fn_804E5CE0(pActor);
    }
}

extern "C" bool fn_804E5FE0(MiniStarCheckPoint* pActor) {
    return MR::tryTalkForce(pActor->mTalkCtrl);
}

extern "C" void fn_804E5FF0(LiveActor* pActor) {
    fn_804E8C40(pActor);
}

extern "C" bool fn_804E6000(MiniStarCheckPoint* pActor) {
    fn_804E8C50(pActor, 0.1f, 1.4f);
    MR::tryTalkForce(pActor->mTalkCtrl);
    return true;
}

extern "C" bool fn_804E6040(MiniStarCheckPoint* pActor, s32 arg) {
    if (fn_804E5E20(pActor)) {
        return false;
    }
    if (fn_804E5E80(pActor)) {
        if (arg == 0) {
            MR::startActionSound(pActor, "Shine", -1, -1, -1);
            return true;
        }
        if (arg == 1) {
            MR::startActionSound(pActor, "Shine", -1, -1, -1);
            return true;
        }
        return false;
    }
    if (arg == 0) {
        return true;
    }
    if (arg == 1) {
        return true;
    }
    return false;
}

extern "C" void fn_804E6120(LiveActor* pActor, s32 drawOrder) {
    MR::setAllEffectDrawOrder(pActor, drawOrder);
}
