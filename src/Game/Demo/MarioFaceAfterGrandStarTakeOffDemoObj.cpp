#include "Game/Demo/MarioFaceAfterGrandStarTakeOffDemoObj.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/BgmUtil.hpp"
#include "Game/Util/Functor.hpp"
#include "Game/Demo/MarioFaceDemoParts.hpp"

// .sbss2 zero object the target reads as the BGM start step.
extern const volatile s32 gMarioFaceAfterGrandStarBgmStartStep;

namespace {
    const char* sPartDemoName = "グランドスター後発進デモ";
    const char* sCameraName = "DemoMarioShipTakeOff";

    // Parts after the fixed ones returned by MarioFaceDemoParts::getPartInfo.
    MarioFaceDemoParts::PartInfo sExtraParts[] = {
        {"デモ用ワープポイント", "MiniWorldWarpPoint", nullptr, nullptr},
    };
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

void MarioFaceAfterGrandStarTakeOffDemoObj::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "MarioShipTakeOff", false);
    mCameraInfo = MR::createActorCameraInfo(rIter);
    MR::initAnimCamera(this, mCameraInfo, sCameraName);
    MR::tryRegisterDemoCast(this, rIter);
    MR::registerDemoActionFunctor(this, MR::FunctorV0M< MarioFaceAfterGrandStarTakeOffDemoObj*, void (MarioFaceAfterGrandStarTakeOffDemoObj::*)() >(this, &MarioFaceAfterGrandStarTakeOffDemoObj::exeDemo), nullptr);
    s32 nFixed = MarioFaceDemoParts::getFixedPartCount();
    mCount = nFixed + 1;
    mParts = new LiveActor*[mCount];
    for (s32 i = 0; i < mCount; i++) {
        if (i < nFixed) {
            mParts[i] = MarioFaceDemoParts::createPart(this, MarioFaceDemoParts::getPartInfo(i), sPartDemoName, rIter);
        } else {
            mParts[i] = MarioFaceDemoParts::createPart(this, &sExtraParts[i - nFixed], sPartDemoName, rIter);
        }
    }
    MR::invalidateClipping(this);
    initNerve(&NrvMarioFaceAfterGrandStarTakeOffDemoObj::MarioFaceAfterGrandStarTakeOffDemoObjNrvDemo::sInstance, 0);
    makeActorDead();
}

void MarioFaceAfterGrandStarTakeOffDemoObj::exeDemo() {
    if (MR::isFirstStep(this)) {
        MR::setPlayerBaseMtx(getBaseMtx());
        for (s32 i = 0; i < mCount; i++) {
            mParts[i]->makeActorAppeared();
        }
    }
    if (MR::isStep(this, gMarioFaceAfterGrandStarBgmStartStep)) {
        MR::startEmptyNameBGMIfNotUnlocked();
    }
    TVec3f dir;
    MR::extractMtxYDir(MR::getJointMtx(this, "MarioShipTakeOff"), &dir);
    dir.negate();
    for (s32 i = 0; i < mCount; i++) {
        mParts[i]->mGravity.set(dir.x, dir.y, dir.z);
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

extern const volatile s32 gMarioFaceAfterGrandStarBgmStartStep = 0;
