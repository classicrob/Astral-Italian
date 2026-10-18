#include "Game/MapObj/MagmaBoatPlanetIce.hpp"
#include "Game/MapObj/MapObjActorInitInfo.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
namespace NrvMagmaBoatPlanetIce {
    class MagmaBoatPlanetIceNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static MagmaBoatPlanetIceNrvWait sInstance;
    };
    MagmaBoatPlanetIceNrvWait MagmaBoatPlanetIceNrvWait::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(MagmaBoatPlanetIceNrvFreeze,MagmaBoatPlanetIce,Freeze);
    void MagmaBoatPlanetIceNrvWait::execute(Spine* spine) const { static_cast<MagmaBoatPlanetIce*>(spine->mExecutor)->setNerve(&MagmaBoatPlanetIceNrvFreeze::sInstance); }
}
MagmaBoatPlanetIce::MagmaBoatPlanetIce(const char* name) : MapObjActor(name),mIceModel(nullptr) {}
MagmaBoatPlanetIce::~MagmaBoatPlanetIce() {}
void MagmaBoatPlanetIce::init(const JMapInfoIter& iter) {
    MapObjActor::init(iter);
    MapObjActorInitInfo info;
    MapObjActorUtil::setupInitInfoPlanet(&info);
    MapObjActorUtil::setupInitInfoTypical(&info,mObjectName);
    MapObjActorUtil::setupInitInfoColorChangeArg0(&info,iter);
    MapObjActorUtil::setupInitInfoTextureChangeArg1(&info,iter);
    MapObjActorUtil::setupInitInfoShadowLengthArg2(&info,iter);
    info.setupNerve(&NrvMagmaBoatPlanetIce::MagmaBoatPlanetIceNrvWait::sInstance);
    initialize(iter,info);
    mIceModel=MR::createIceModel(this,nullptr,true);
}
void MagmaBoatPlanetIce::connectToScene(const MapObjActorInitInfo&) { MR::connectToScenePlanet(this); }
void MagmaBoatPlanetIce::exeFreeze() {
    if (MR::isFirstStep(this) && mIceModel) MR::tryStartAllAnim(mIceModel,"Freeze");
}
