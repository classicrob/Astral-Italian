#include "Game/MapObj/QuicksandPlanet.hpp"
#include "Game/MapObj/MapObjActorInitInfo.hpp"
#include "Game/Map/PlanetMapFunction.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
namespace { extern char sQuicksandMoveSound[]; }
namespace NrvQuicksandPlanet {
    class QuicksandPlanetNrvMove : public Nerve {
    public:
        virtual void execute(Spine* spine) const {
            QuicksandPlanet* actor = static_cast<QuicksandPlanet*>(spine->mExecutor);
            if (actor->isPlayerMoveable()) MR::startActionSoundAuto(actor,sQuicksandMoveSound,-1,-1,-1);
        }
        static QuicksandPlanetNrvMove sInstance;
    };
    QuicksandPlanetNrvMove QuicksandPlanetNrvMove::sInstance ATTRIBUTE_WEAK;
}
QuicksandPlanet::QuicksandPlanet(const char* name) : MapObjActor(name), mEffectCount(0) {}
QuicksandPlanet::~QuicksandPlanet() {}
void QuicksandPlanet::init(const JMapInfoIter& iter) {
    MapObjActor::init(iter);
    MapObjActorInitInfo info;
    MapObjActorUtil::setupInitInfoPlanet(&info);
    MapObjActorUtil::setupInitInfoTypical(&info,mObjectName);
    info.setupSound(4,nullptr);
    info.setupNerve(&NrvQuicksandPlanet::QuicksandPlanetNrvMove::sInstance);
    initialize(iter,info);
    bool reverse=false;
    MR::getJMapInfoArg3NoInit(iter,&reverse);
    if (reverse) MR::tryStartAllAnim(this,"Reverse");
    else MR::tryStartAllAnim(this,"Move");
    PlanetMapFunction::verifyEffects(this,&mEffectCount,mObjectName,16);
    MR::registerDemoSimpleCastAll(this);
}
namespace { char sQuicksandMoveSound[] = "PlayerOnSand"; }
void QuicksandPlanet::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    PlanetMapFunction::tryForceDeleteEffects(this,mObjectName,mEffectCount);
}
void QuicksandPlanet::startClipped() {
    LiveActor::startClipped();
    PlanetMapFunction::tryForceDeleteEffects(this,mObjectName,mEffectCount);
}
void QuicksandPlanet::endClipped() {
    LiveActor::endClipped();
    PlanetMapFunction::tryEmitEffects(this,mObjectName,mEffectCount);
}
bool QuicksandPlanet::isPlayerMoveable() {
    if (MR::isOnPlayer(this)) {
        const Triangle* tri=MR::getPlayerGroundingPolygon();
        if (!tri) return false;
        if (MR::isExistRail(this)) return MR::isGroundCodeRailMove(tri);
        return MR::isGroundCodeAreaMove(tri);
    }
    return false;
}
