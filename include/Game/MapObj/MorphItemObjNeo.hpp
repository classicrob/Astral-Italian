#pragma once

#include "Game/Effect/SimpleEffectObj.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class NameObjArchiveListCollector;
class ActorCameraInfo;
class ModelObj;
class CameraTargetMtx;

class MapPartsRailMover;

class MorphItemObjNeo : public LiveActor {
public:
    MorphItemObjNeo(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void appear();
    virtual void makeActorAppeared();
    virtual void kill();
    virtual void makeActorDead();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    virtual bool isDemo() const;
    virtual int getPowerUp() = 0;
    virtual void emitAppearEffects();
    virtual void appearTransformation();
    virtual void killTransformation();

    static const char* getMorphModelName(int);
    static void glowEffect(LiveActor*, int);

    void calcContainerBaseMtx();         // fn_802D2A70
    bool tryGetItem(HitSensor*, HitSensor*);  // fn_802D2F80
    void resetToSpawn();                 // fn_802D34A0
    void vanishByGet();                  // fn_802D34F0
    void recover();                      // fn_802D3580
    void updateRailMover();              // fn_802D3600
    bool tryStartAppearDemo();           // fn_802D3800
    void updateRotateY();
    bool isPowerUp(int);

    void exeWait();
    void exeAppear();
    void exeSwitchAppear();
    void exeWait2();
    void exeFly();
    void exeFall() NO_INLINE;            // fn_802D3FE0
    void endHide();                      // fn_802D4190

    /* 0x90 */ u16 mAppearMode;
    /* 0x92 */ u16 mAppearFrames;
    /* 0x94 */ bool mIsUseSwitch;
    /* 0x95 */ bool mIsFloating;  // Obj_arg2
    /* 0x96 */ bool _92;
    /* 0x97 */ u8 mCameraMode;     // Obj_arg5
    /* 0x98 */ s8 mRotateMode;     // Obj_arg6
    /* 0x99 */ u8 _95;           // Obj_arg0
    /* 0x9A */ u8 mContainerType;  // Obj_arg3
    /* 0x9B */ u8 _97;
    /* 0x9C */ s8 mIsBind;      // Obj_arg1
    /* 0x9D */ u8 mShadowType;  // Obj_arg4
    /* 0xA0 */ TVec3f mZRotation;
    /* 0xAC */ f32 mAngle;
    /* 0xB0 */ TVec3f mSpawnPosition;
    /* 0xBC */ ActorCameraInfo* mActorCameraInfo;
    /* 0xC0 */ ModelObj* mContainerModel;
    /* 0xC4 */ CameraTargetMtx* mCameraTargetMtx;
    /* 0xC8 */ MapPartsRailMover* mRailMover;
    /* 0xCC */ TVec3f mRailClipping;
    /* 0xD8 */ TVec3f mFallPos;
};

class MorphItemNeoHopper : public MorphItemObjNeo {
public:
    MorphItemNeoHopper(const char* pName) : MorphItemObjNeo(pName) {
    }

    virtual int getPowerUp() {
        return 1;
    }
};

class MorphItemNeoIce : public MorphItemObjNeo {
public:
    MorphItemNeoIce(const char* pName) : MorphItemObjNeo(pName) {
    }

    virtual int getPowerUp() {
        return 4;
    }
};

class MorphItemNeoFire : public MorphItemObjNeo {
public:
    MorphItemNeoFire(const char* pName) : MorphItemObjNeo(pName) {
    }

    virtual int getPowerUp() {
        return 5;
    }
};

class MorphItemNeoFoo : public MorphItemObjNeo {
public:
    MorphItemNeoFoo(const char* pName) : MorphItemObjNeo(pName) {
    }

    virtual int getPowerUp() {
        return 6;
    }
};

class MorphItemNeoBee : public MorphItemObjNeo {
public:
    MorphItemNeoBee(const char* pName) : MorphItemObjNeo(pName) {
    }

    virtual int getPowerUp() {
        return 2;
    }
};

class MorphItemNeoTeresa : public MorphItemObjNeo {
public:
    MorphItemNeoTeresa(const char* pName) : MorphItemObjNeo(pName) {
    }

    virtual int getPowerUp() {
        return 3;
    }
};

// Why is this here?

class EffectObjR100F50SyncClipping : public SimpleEffectObj {
public:
    EffectObjR100F50SyncClipping(const char* pName) : SimpleEffectObj(pName) {
    }

    virtual f32 getClippingRadius() const {
        return 100.0f;
    }

    virtual f32 getFarClipDistance() const {
        return 50.0f;
    }

    virtual bool isSyncClipping() const {
        return true;
    }
};

namespace NrvMorphItemObjNeo {
    NEW_NERVE(MorphItemObjNeoNrvWait, MorphItemObjNeo, Wait);
    NEW_NERVE(MorphItemObjNeoNrvWait2, MorphItemObjNeo, Wait2);
    NEW_NERVE(MorphItemObjNeoNrvAppear, MorphItemObjNeo, Appear);
    NEW_NERVE(MorphItemObjNeoNrvSwitchAppear, MorphItemObjNeo, SwitchAppear);
    NEW_NERVE(MorphItemObjNeoNrvFly, MorphItemObjNeo, Fly);

    class MorphItemObjNeoNrvDemo : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            reinterpret_cast< MorphItemObjNeo* >(pSpine->mExecutor)->mAngle = 0.0f;
        }
        static MorphItemObjNeoNrvDemo sInstance;
    };
    MorphItemObjNeoNrvDemo MorphItemObjNeoNrvDemo::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(MorphItemObjNeoNrvFall, MorphItemObjNeo, Fall);

    class MorphItemObjNeoNrvHide : public Nerve {
    public:
        virtual void execute(Spine*) const {
        }
        virtual void executeOnEnd(Spine* pSpine) const {
            reinterpret_cast< MorphItemObjNeo* >(pSpine->mExecutor)->endHide();
        }
        static MorphItemObjNeoNrvHide sInstance;
    };
    MorphItemObjNeoNrvHide MorphItemObjNeoNrvHide::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvMorphItemObjNeo
