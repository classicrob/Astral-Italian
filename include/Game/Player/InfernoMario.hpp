#pragma once
#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TQuat.hpp>
class XanimePlayer;
class GhostPacketHistory;
class GhostPacketData;
class InfernoMarioStateBase;
class InfernoMario : public LiveActor {
public:
    InfernoMario(const char*);
    virtual ~InfernoMario();
    virtual void init(const JMapInfoIter&);
    virtual void draw() const;
    virtual void makeActorAppeared();
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    void fn_80381670();
    void fn_803816C0(s32);
    void fn_80381730();
    void fn_80381760();
    void fn_80381790();
    void setFrameDelay(s32);
    void startActionName(const char*);
    void startActionHash(u32);
    void setHistory(GhostPacketHistory*);
    bool readPacket(s32);
    bool readTransform(s32);
    void applyTransform(const GhostPacketData*);
    void applyPacket(const GhostPacketData*);
    void exeAppearance();
    void exeFollow();
    XanimePlayer* mXanimePlayer;
    XanimePlayer* mUpperPlayer;
    GhostPacketHistory* mHistory;
    InfernoMarioStateBase* mState;
    TQuat4f mTargetQuat;
    TQuat4f mCurrentQuat;
    TQuat4f mInitialQuat;
    TVec3f mInitialPos;
    s32 mFrameDelay;
    s32 mFrameSpacing;
    char mActionName[0x40];
    s32 mArg7;
    bool mPlayExtraSound;
};
