#pragma once

#include "Game/MapObj/MapObjActor.hpp"

class ActorCameraInfo;
class BenefitItemObj;

class SimpleBreakableObjBase : public MapObjActor {
public:
    SimpleBreakableObjBase(const char* pName) : MapObjActor(pName) {
    }

    virtual ~SimpleBreakableObjBase() {
    }

    virtual void control();
};

class SimpleBreakableObj : public SimpleBreakableObjBase {
public:
    SimpleBreakableObj(const char*);

    virtual ~SimpleBreakableObj() NO_INLINE;
    virtual void init(const JMapInfoIter&) override;
    virtual void control() override;
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*) override;
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*) override;
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*) override;
    virtual void initCaseUseSwitchA(const MapObjActorInitInfo&) override;
    virtual void initCaseNoUseSwitchA(const MapObjActorInitInfo&) override {
    }

    virtual void initCaseUseSwitchB(const MapObjActorInitInfo&) override;
    virtual void initCaseNoUseSwitchB(const MapObjActorInitInfo&) override {
    }

    virtual void controlBreak();

    void setStateBreak();
    void startBreak();
    void endBreak();
    bool isStateBreaking() const;
    void appearItem();
    void exeWait();

    void exeTryStartDemo();
    void exeDemo();

    /* 0xD0 */ ActorCameraInfo* mCameraInfo;
    /* 0xD4 */ s32 mBreakOffsetFrame;
    /* 0xD8 */ s32 _CC;
    /* 0xDC */ s32 mSwitchDeadDelay;
    /* 0xE0 */ u16 mBreakFlags;
    /* 0xE4 */ s32 mItemType;
    /* 0xE8 */ s32 mItemNum;
    /* 0xEC */ BenefitItemObj* mKinokoOneUp;
    /* 0xF0 */ bool mPlayRiddleSound;
    /* 0xF1 */ bool mBreakEnded;
    /* 0xF2 */ bool _E6;
};

class SimpleBreakableIndirectObj : public SimpleBreakableObj {
public:
    SimpleBreakableIndirectObj(const char* pName) : SimpleBreakableObj(pName) {
    }

    virtual ~SimpleBreakableIndirectObj() {
    }

    virtual void connectToScene(const MapObjActorInitInfo&) override;
};

class CollisionParts;

class SimpleBreakableStrongLightObj : public SimpleBreakableObj {
public:
    SimpleBreakableStrongLightObj(const char* pName) : SimpleBreakableObj(pName) {}
    virtual ~SimpleBreakableStrongLightObj();
    virtual void connectToScene(const MapObjActorInitInfo&);
};

class SimpleBreakableCollisionObj : public SimpleBreakableObj {
public:
    SimpleBreakableCollisionObj(const char* pName) : SimpleBreakableObj(pName), mBreakCollision(nullptr) {}
    virtual ~SimpleBreakableCollisionObj();
    virtual void init(const JMapInfoIter&);
    virtual void controlBreak();
    /* 0xF4 */ CollisionParts* mBreakCollision;
};

class SimpleBreakableDeleteShadowObj : public SimpleBreakableObj {
public:
    SimpleBreakableDeleteShadowObj(const char* pName) : SimpleBreakableObj(pName) {}
    virtual ~SimpleBreakableDeleteShadowObj();
    virtual void controlBreak();
};
