#pragma once

#include <revolution/types.h>

class LiveActor;
class JMapInfo;

// One row of the ActorParam csv (0xC bytes). mValue holds the value itself (s32/f32) or a pointer
// to a small float block, depending on mType.
class ActorParamEntry {
public:
    ActorParamEntry();

    /* 0x0 */ const char* mName;
    /* 0x4 */ s32 mType;
    union {
        /* 0x8 */ s32 mS32;
        f32 mF32;
        void* mPtr;
    } mValue;
};

class ActorParam {
public:
    ActorParam(LiveActor*);

    static ActorParam* tryCreate(LiveActor*);

    /* 0x0 */ s32 mEntryNum;
    /* 0x4 */ ActorParamEntry* mEntries;
};

// SMG2 default-block reset (named in ActorParam.cpp; 0x1C bytes: pos(0,0,0), quat(0,0,0,1)).
void initActorParamDefault(f32* pParam);
// Additive (objects lane): per-actor ActorParam csv accessors (symbols exist in the target, defined elsewhere).
namespace MR {
    const f32* getActorParamF32(const LiveActor*, const char*);
    const s32* getActorParamS32(const LiveActor*, const char*);
    const f32* getActorParamMove(const LiveActor*, const char*);
    const f32* getActorParamSight(const LiveActor*, const char*);
    const f32* getActorParamJump(const LiveActor*, const char*);
};  // namespace MR
