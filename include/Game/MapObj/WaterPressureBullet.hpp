#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class ActorCameraInfo;
class ActorStateParamScale;

class WaterPressureBullet : public LiveActor {
public:
    WaterPressureBullet(const char*);

    virtual ~WaterPressureBullet();
    virtual void init(const JMapInfoIter&);
    virtual void kill();
    virtual void movement();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void shotWaterBullet(LiveActor*, const TPos3f&, f32, bool, bool, bool, ActorCameraInfo**);
    void shotWaterBullet(LiveActor*, const ActorStateParamScale*, const TPos3f&, f32, bool, bool, bool, ActorCameraInfo**);

    bool isBound() const {
        return mHostActor != nullptr;
    }

    bool isNotBoundInline() const {
        return _B2 && !isBound();
    }

    bool isNotBound() {
        return isNotBoundInline() || !_B2;
    }

    void exeFly();
    void exeSpinKill();
    bool startHostCamera() const;
    bool endHostCamera() const;
    bool inviteMario(HitSensor*);
    void updateSuffererMtx();

    /* 0x90 */ TVec3f _8C;
    /* 0x9C */ TVec3f _98;
    /* 0xA8 */ LiveActor* _A4;
    /* 0xAC */ f32 _A8;
    /* 0xB0 */ LiveActor* mHostActor;
    /* 0xB4 */ bool _B0;
    /* 0xB5 */ bool _B1;
    /* 0xB6 */ bool _B2;
    /* 0xB7 */ u8 _B3;
    /* 0xB8 */ ActorStateParamScale* mParamScale;
    /* 0xBC */ ActorCameraInfo** mCameraInfo;
};
