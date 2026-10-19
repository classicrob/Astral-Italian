#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class CollectTico;
class PartsModel;

/// @brief Silver Star
class StrayTico : public LiveActor {
public:
    StrayTico(const char*, CollectTico*);

    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPush(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void launch(const TPos3f&);
    bool tryStartBindMove();
    bool tryEndBindMove();
    bool tryNigeroDrop();
    bool isNigeroTakeable() const NO_INLINE;
    void exeBindMove();
    void exeBindEnd();
    void initSensor();
    void initShadow() NO_INLINE;
    void startGlad();
    bool requestCompleteDemo(const TVec3f&, const TVec3f&, f32);
    void updateWarp();
    void updateChase();
    bool isEnableSpin() const;
    bool isRescued() const;
    bool isCompleteDemoEnd() const;
    void noticeEnd();

    void exeWait();
    void exeGlad();
    void exeChase();
    void exeLauncherCamera();
    void exeCompleteDemo();

    /* 0x90 */ CollectTico* mHost;
    /* 0x94 */ PartsModel* mItemBubbleModel;
    /* 0x98 */ TQuat4f mRotateQuat;
    /* 0xA8 */ TVec3f mFrontVec;
    /* 0xB4 */ TVec3f mWarpPosition;
    /* 0xC0 */ TVec3f mCompleteCenter;
    /* 0xCC */ TVec3f mCompleteUp;
    /* 0xD8 */ TVec3f mCompleteFront;
    /* 0xE4 */ TVec3f mCompleteStartPosition;
    /* 0xF0 */ f32 mCompleteStartAngle;
    /* 0xF4 */ f32 mCompleteOriginalAngle;
};
