#include "Game/MapObj/MagmaBoatPlanetMagma.hpp"
#include "Game/MapObj/MapObjActorInitInfo.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
namespace NrvMagmaBoatPlanetMagma {
class MagmaBoatPlanetMagmaNrvWait : public Nerve {
public:
    virtual void execute(Spine*) const;
    static MagmaBoatPlanetMagmaNrvWait sInstance;
};
MagmaBoatPlanetMagmaNrvWait MagmaBoatPlanetMagmaNrvWait::sInstance;
NEW_NERVE(MagmaBoatPlanetMagmaNrvFreeze, MagmaBoatPlanetMagma, Freeze);
class MagmaBoatPlanetMagmaNrvKill : public Nerve {
public:
    virtual void execute(Spine* spine) const { MagmaBoatPlanetMagma* actor = static_cast<MagmaBoatPlanetMagma*>(spine->mExecutor); if (MR::isActionEnd(actor->mIceModel)) actor->kill(); }
    static MagmaBoatPlanetMagmaNrvKill sInstance;
};
MagmaBoatPlanetMagmaNrvKill MagmaBoatPlanetMagmaNrvKill::sInstance;
inline void MagmaBoatPlanetMagmaNrvWait::execute(Spine* spine) const { MagmaBoatPlanetMagma* actor = static_cast<MagmaBoatPlanetMagma*>(spine->mExecutor); if (MR::isValidSwitchA(actor) && MR::isOnSwitchA(actor)) actor->setNerve(&MagmaBoatPlanetMagmaNrvFreeze::sInstance); }
}
MagmaBoatPlanetMagma::MagmaBoatPlanetMagma(const char* name) : MapObjActor(name), mIceModel(0) {}
void MagmaBoatPlanetMagma::init(const JMapInfoIter& iter) {
    MapObjActor::init(iter);
    MapObjActorInitInfo info;
    MapObjActorUtil::setupInitInfoPlanet(&info);
    MapObjActorUtil::setupInitInfoTypical(&info, mObjectName);
    MapObjActorUtil::setupInitInfoColorChangeArg0(&info, iter);
    MapObjActorUtil::setupInitInfoTextureChangeArg1(&info, iter);
    MapObjActorUtil::setupInitInfoShadowLengthArg2(&info, iter);
    info.setupSound(3, 0);
    info.setupNerve(&NrvMagmaBoatPlanetMagma::MagmaBoatPlanetMagmaNrvWait::sInstance);
    initialize(iter, info);
    mIceModel = MR::createIceModel(this, 0, false);
    mIceModel->makeActorDead();
    MR::startAction(mIceModel, "Freeze");
    MR::setAllAnimFrame(this, "Freeze", 0.0f);
}
void MagmaBoatPlanetMagma::connectToScene(const MapObjActorInitInfo&) { MR::connectToScenePlanet(this); }
void MagmaBoatPlanetMagma::initCaseUseSwitchA(const MapObjActorInitInfo&) {}
void MagmaBoatPlanetMagma::initCaseUseSwitchB(const MapObjActorInitInfo&) {}
void MagmaBoatPlanetMagma::exeFreeze() {
    if (MR::isFirstStep(this)) {
        mIceModel->appear();
        mIceModel->mCheckHiddenHost = true;
        MR::startActionSound(this, "Freeze");
        MR::startAction(mIceModel, "Freeze");
        MR::emitEffect(mIceModel, "Freeze");
    }
    if (MR::isStep(this, 30)) {
        if (MR::isValidSwitchB(this)) MR::onSwitchB(this);
        MR::hideModel(this);
        MR::invalidateCollisionParts(this);
        setNerve(&NrvMagmaBoatPlanetMagma::MagmaBoatPlanetMagmaNrvKill::sInstance);
    }
}
MagmaBoatPlanetMagma::~MagmaBoatPlanetMagma() {}
