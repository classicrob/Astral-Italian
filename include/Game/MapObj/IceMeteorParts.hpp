#pragma once
#include "Game/LiveActor/PartsModel.hpp"
class HitSensor;
class IceMeteorParts : public PartsModel {
public:
    IceMeteorParts(LiveActor*, const char*);
    virtual ~IceMeteorParts();
    virtual void init(const JMapInfoIter&);
    virtual void calcAndSetBaseMtx();
    virtual void exeFall();
    void setOffset(const TVec3f&);
    const TVec3f& getOffset() const;
    TVec3f mOffset; // 0xA0
};
class IceMeteorHolder : public NameObj {
public:
    IceMeteorHolder(const char*);
    virtual ~IceMeteorHolder();
    virtual void init(const JMapInfoIter&);
    virtual void movement();
    void requestFireDance(HitSensor*, HitSensor*);
    void cancelFireDance(HitSensor*, HitSensor*);
    void clearRequest();
    s32 mRequest; // 0x14: 0 none, 1 send, 2 cancelled
    HitSensor* mSender;
    HitSensor* mReceiver;
};
namespace MR {
    void createIceMeteorHolder();
    void requestIceMeteorFireDance(HitSensor*, HitSensor*);
    void cancelIceMeteorFireDance(HitSensor*, HitSensor*);
}
