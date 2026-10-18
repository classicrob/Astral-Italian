#include "Game/Demo/MarioFaceAfterGrandStarTakeOffDemoObj.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/Functor.hpp"
#include "Game/Demo/MarioFaceDemoParts.hpp"
#include "Game/Demo/UnnamedDemoCallee.hpp"


// Data block: functor/name table at +0x60 (entry array), +0x70 (member fn ptr), +0x7C (name).
extern "C" char lbl_8067FA68[];
extern "C" char lbl_8067FAE4[];
extern "C" const char* lbl_807CF710;
extern "C" const char* lbl_807CF714;
extern "C" s32 lbl_807E3018;

namespace {
    typedef void (MarioFaceAfterGrandStarTakeOffDemoObj::*ExeFn)();
};  // namespace

namespace NrvMarioFaceAfterGrandStarTakeOffDemoObj {
    class MarioFaceAfterGrandStarTakeOffDemoObjNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static MarioFaceAfterGrandStarTakeOffDemoObjNrvWait sInstance;
    };
    MarioFaceAfterGrandStarTakeOffDemoObjNrvWait MarioFaceAfterGrandStarTakeOffDemoObjNrvWait::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(MarioFaceAfterGrandStarTakeOffDemoObjNrvDemo, MarioFaceAfterGrandStarTakeOffDemoObj, Demo);
};  // namespace NrvMarioFaceAfterGrandStarTakeOffDemoObj

MarioFaceAfterGrandStarTakeOffDemoObj::MarioFaceAfterGrandStarTakeOffDemoObj(const char* pName) : LiveActor(pName), mCameraInfo(nullptr), mParts(nullptr), mCount(0) {
}

// NONMATCHING: functor copied via a stack temporary; target builds it directly from the table
void MarioFaceAfterGrandStarTakeOffDemoObj::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, reinterpret_cast<const char*>(lbl_8067FA68 + 0x7C), false);
    mCameraInfo = MR::createActorCameraInfo(rIter);
    MR::initAnimCamera(this, mCameraInfo, lbl_807CF714);
    MR::tryRegisterDemoCast(this, rIter);
    MR::FunctorV0M< MarioFaceAfterGrandStarTakeOffDemoObj*, ExeFn > functor(this, *reinterpret_cast< ExeFn* >(lbl_8067FA68 + 0x70));
    MR::registerDemoActionFunctor(this, functor, nullptr);
    s32 nFixed = MarioFaceDemoParts::getFixedPartCount();
    mCount = nFixed + 1;
    mParts = new LiveActor*[mCount];
    for (s32 i = 0; i < mCount; i++) {
        if (i < nFixed) {
            mParts[i] = MarioFaceDemoParts::createPart(this, MarioFaceDemoParts::getPartInfo(i), lbl_807CF710, rIter);
        } else {
            mParts[i] = MarioFaceDemoParts::createPart(this, reinterpret_cast< const MarioFaceDemoParts::PartInfo* >(lbl_8067FA68 + 0x60 + (i - nFixed) * 0x10), lbl_807CF710, rIter);
        }
    }
    MR::invalidateClipping(this);
    initNerve(&NrvMarioFaceAfterGrandStarTakeOffDemoObj::MarioFaceAfterGrandStarTakeOffDemoObjNrvDemo::sInstance, 0);
    makeActorDead();
}

// NONMATCHING: negation of the joint dir uses scalar fneg, target pairs x/y with ps_neg; functor copy in init differs
void MarioFaceAfterGrandStarTakeOffDemoObj::exeDemo() {
    if (MR::isFirstStep(this)) {
        MR::setPlayerBaseMtx(getBaseMtx());
        for (s32 i = 0; i < mCount; i++) {
            mParts[i]->makeActorAppeared();
        }
    }
    if (MR::isStep(this, lbl_807E3018)) {
        UnnamedDemoCallee::fn_8001BFE0();
    }
    TVec3f dir;
    MR::extractMtxYDir(MR::getJointMtx(this, lbl_8067FAE4), &dir);
    dir.x = -dir.x;
    dir.y = -dir.y;
    dir.z = -dir.z;
    for (s32 i = 0; i < mCount; i++) {
        mParts[i]->mGravity = dir;
    }
    if (!MR::isDemoActive()) {
        kill();
    }
}

void MarioFaceDemoParts::closeWhiteFadeAfterGrandStar() {
    MR::closeSystemWipeWhiteFade(100);
}

MarioFaceAfterGrandStarTakeOffDemoObj::~MarioFaceAfterGrandStarTakeOffDemoObj() {
}
