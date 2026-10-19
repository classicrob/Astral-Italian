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
        return mIsLongLived && !isBound();
    }

    bool isNotBound() {
        return isNotBoundInline() || !mIsLongLived;
    }

    void exeFly();
    void exeSpinKill();
    bool startHostCamera() const;
    bool endHostCamera() const;
    bool inviteMario(HitSensor*);
    void updateSuffererMtx();

    /* 0x90 */ TVec3f mFrontDir;
    /* 0x9C */ TVec3f mSideDir;
    /* 0xA8 */ LiveActor* mShooterActor;
    /* 0xAC */ f32 mSpeed;
    /* 0xB0 */ LiveActor* mHostActor;
    /* 0xB4 */ bool mIsNoGravity;
    /* 0xB5 */ bool mCanSlideOnGround;
    /* 0xB6 */ bool mIsLongLived;
    /* 0xB7 */ u8 _B3;
    /* 0xB8 */ ActorStateParamScale* mParamScale;
    /* 0xBC */ ActorCameraInfo** mCameraInfo;
};
