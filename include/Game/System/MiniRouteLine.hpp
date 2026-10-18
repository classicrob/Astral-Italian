#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class ProjmapEffectMtxSetter;
class WorldMapResultWorld;

/// The glowing line between two map nodes (vtable 0x807023D0, 0xD8 bytes).
class MiniRouteLine : public LiveActor {
public:
    MiniRouteLine(const char* pName, WorldMapResultWorld* pWorld);

    virtual ~MiniRouteLine();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void kill();
    virtual void calcAndSetBaseMtx();

    void setCondition(const char* pStage, s32 starNum, const char* pFlag);
    bool isLocked() const;
    bool hasFlag() const;
    void setup(const TVec3f& rFrom, const TVec3f& rTo);
    void setColorChange(u8);
    void setAlpha(s32, f32);
    void updateShadow(s32, f32);
    void updateEffect(s32, f32);
    void updateTransform(LiveActor*);
    void updateEffectPos();
    void emitProgressEffect();
    void deleteProgressEffect();
    void setEffectDrawOrder(s32);
    bool getNextNode(s32 node, s32* pOther);
    bool open();
    bool isBetween(s32 a, s32 b, bool strict);
    bool getOther(s32 node, s32* pOther) const;
    bool isBetweenNodes(s32 a, s32 b) const;
    bool isDirected(s32 a, s32 b) const;

    s32 mFrom;                       // 0x90
    s32 mTo;                         // 0x94
    const char* mCondStage;          // 0x98
    s32 mCondStarNum;                // 0x9C
    const char* mCondFlag;           // 0xA0
    bool mUnkA4;                     // 0xA4
    u8 mColorChange;                 // 0xA5
    bool mValid;                     // 0xA6
    bool mOpened;                    // 0xA7
    bool mHidden;                    // 0xA8
    f32 mAlpha;                      // 0xAC
    TVec3f mCenter;                  // 0xB0
    TVec3f mEffectPos;               // 0xBC
    const char* mEffectName;         // 0xC8
    LiveActor* mEffectActor;         // 0xCC
    LiveActor* mShadowActor;         // 0xD0
    ProjmapEffectMtxSetter* mMtxSetter;  // 0xD4
};

class MiniRouteLineList {
public:
    MiniRouteLineList(WorldMapResultWorld* pWorld);

    void init();
    void appearAll();
    void killAll();
    void setEffectDrawOrder(s32 order);
    bool isLinked(s32 a, s32 b, bool strict);
    bool isDirected(s32 a, s32 b);
    bool openFrom(s32 node);
    bool hasOpenLine(s32 node);
    bool hasOpenLineNoAlpha(s32 node);
    bool hasLineFlag(s32 a, s32 b);
    void setLineAlpha(s32 a, s32 b, f32 alpha);
    void emitLineEffect(s32 a, s32 b);
    void deleteLineEffect(s32 a, s32 b);
    void setLineEffectTico(s32 a, s32 b);
    void setLineHidden(s32 a, s32 b);
    void load(s32 worldId);
    MiniRouteLine* findLine(s32 a, s32 b);

    WorldMapResultWorld* mWorld;  // 0x00
    s32 mNum;                     // 0x04
    MiniRouteLine** mLines;       // 0x08
};
