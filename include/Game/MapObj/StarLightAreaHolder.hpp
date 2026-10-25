#pragma once
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/Util/Array.hpp"
#include <JSystem/JGeometry.hpp>
class ClipArea;
class HitSensor;
class StarLightAreaHolder : public DeriveActorGroup<ClipArea> {
public:
    StarLightAreaHolder(const char*);
    virtual ~StarLightAreaHolder();
    virtual void init(const JMapInfoIter&);
    virtual void movement();
    bool isInStarLight(const TVec3f&);
    bool tryRegisterReceiver(HitSensor*);
    MR::Vector<MR::AssignableArray<HitSensor*> > mReceivers;
};
namespace MR {
    void registerStarLightArea(ClipArea*);
    bool tryRegisterStarLightReceiver(HitSensor*);
}
