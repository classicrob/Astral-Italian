#include "Game/MapObj/GoroRockLaneParts.hpp"
#include "Game/MapObj/MapObjActorInitInfo.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/Functor.hpp"
namespace NrvGoroRockLaneParts {
    class GoroRockLanePartsNrvMove : public Nerve {
    public:
        virtual void execute(Spine* spine) const {
            GoroRockLaneParts* actor = static_cast<GoroRockLaneParts*>(spine->mExecutor);
            if (MR::isFirstStep(actor)) actor->startMapPartsFunctions();
        }
        static GoroRockLanePartsNrvMove sInstance;
    };
    GoroRockLanePartsNrvMove GoroRockLanePartsNrvMove::sInstance ATTRIBUTE_WEAK;
}
GoroRockLaneParts::GoroRockLaneParts(const char* name) : MapObjActor(name), mClippingCenter(0.0f,0.0f,0.0f) {}
GoroRockLaneParts::~GoroRockLaneParts() {}
void GoroRockLaneParts::init(const JMapInfoIter& iter) {
    MapObjActor::init(iter);
    MapObjActorInitInfo info;
    info.setupHioNode("地形オブジェ");
    info.setupDefaultPos();
    info.setupConnectToScene();
    info.setupEffect(nullptr,0);
    info.setupSound(6,nullptr);
    info.setupSoundPos(nullptr);
    info.setupGroupClipping(32);
    info.setupSeAppear();
    info.setupRailMover();
    info.setupRotator();
    info.setupShadow(nullptr);
    info.setupBaseMtxFollowTarget();
    info.setupNerve(&NrvGoroRockLaneParts::GoroRockLanePartsNrvMove::sInstance);
    initialize(iter,info);
    if (MR::isConnectedWithRail(iter)) {
        f32 radius=0.0f;
        MR::calcModelBoundingRadius(&radius,this);
        MR::initAndSetRailClipping(&mClippingCenter,this,100.0f,radius);
    }
    MR::startAction(this,"Normal");
    MR::setBrkFrameEndAndStop(this);
}
void GoroRockLaneParts::initCaseUseSwitchB(const MapObjActorInitInfo&) {
    MR::listenStageSwitchOnOffB(this, MR::Functor(this,&GoroRockLaneParts::setChangeColor), MR::Functor(this,&GoroRockLaneParts::setNormalColor));
}
void GoroRockLaneParts::setChangeColor() { MR::startBrk(this,"Change"); }
void GoroRockLaneParts::setNormalColor() { MR::startAction(this,"Normal"); }
void GoroRockLaneParts::initCaseUseSwitchA(const MapObjActorInitInfo&) {}
