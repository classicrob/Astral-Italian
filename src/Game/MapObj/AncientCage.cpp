#include "Game/MapObj/AncientCage.hpp"
#include "Game/MapObj/MapObjActorInitInfo.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/Functor.hpp"
namespace NrvAncientCage {
    class AncientCageNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static AncientCageNrvWait sInstance;
    };
    AncientCageNrvWait AncientCageNrvWait::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(AncientCageNrvStartDemo, AncientCage, StartDemo);
    NEW_NERVE(AncientCageNrvOpen, AncientCage, Open);
    class AncientCageNrvDisappear : public Nerve {
    public:
        virtual void execute(Spine* spine) const {
            AncientCage* actor = static_cast<AncientCage*>(spine->mExecutor);
            if (MR::isStep(actor, 60)) actor->kill();
        }
        static AncientCageNrvDisappear sInstance;
    };
    AncientCageNrvDisappear AncientCageNrvDisappear::sInstance ATTRIBUTE_WEAK;
}
namespace { const char* sDemoName = "注目カメラ"; }
AncientCage::AncientCage(const char* name) : MapObjActor(name), mCameraInfo(nullptr) {}
AncientCage::~AncientCage() {}
void AncientCage::init(const JMapInfoIter& iter) {
    MapObjActor::init(iter);
    MapObjActorInitInfo info;
    MapObjActorUtil::setupInitInfoSimpleMapObj(&info);
    info.setupNerve(&NrvAncientCage::AncientCageNrvWait::sInstance);
    initialize(iter, info);
    if (!MR::initActorCamera(this, iter, &mCameraInfo)) mCameraInfo = nullptr;
}
void AncientCage::exeStartDemo() {
    if (MR::isFirstStep(this)) MR::startActorCameraTargetSelf(this, mCameraInfo, -1);
    if (MR::isStep(this, 50)) setNerve(&NrvAncientCage::AncientCageNrvOpen::sInstance);
}
void AncientCage::exeOpen() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Open");
        MR::tryRumblePadWeak(this, 0);
        MR::shakeCameraWeak();
    }
    if (MR::isActionEnd(this)) {
        MR::tryRumblePadWeak(this, 0);
        MR::shakeCameraWeak();
        if (mCameraInfo) {
            setNerve(&NrvAncientCage::AncientCageNrvDisappear::sInstance);
            return;
        }
        kill();
    }
}
void AncientCage::kill() {
    if (mCameraInfo) {
        MR::endActorCamera(this, mCameraInfo, false, -1);
        MR::endDemo(this, sDemoName);
    }
    MapObjActor::kill();
}
void AncientCage::initCaseUseSwitchB(const MapObjActorInitInfo&) {
    MR::listenStageSwitchOnB(this, MR::FunctorV0M<AncientCage*, void (AncientCage::*)()>(this, &AncientCage::startOpenDemo));
}
void AncientCage::startOpenDemo() {
    MR::invalidateClipping(this);
    if (mCameraInfo) {
        MR::requestStartDemoWithoutCinemaFrame(this, sDemoName, &NrvAncientCage::AncientCageNrvStartDemo::sInstance, nullptr);
        return;
    }
    setNerve(&NrvAncientCage::AncientCageNrvOpen::sInstance);
}
void AncientCage::initCaseNoUseSwitchB(const MapObjActorInitInfo&) {}
void AncientCage::initCaseNoUseSwitchA(const MapObjActorInitInfo&) {}
void AncientCage::initCaseUseSwitchA(const MapObjActorInitInfo&) {}
