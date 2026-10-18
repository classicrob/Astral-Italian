#pragma once

#include <JSystem/JGeometry/TVec.hpp>
#include <revolution/types.h>

class LiveActor;

// One CSV row of the ItemGenerator bcsv (0x14 bytes).
class ActorItemGeneratorEntry {
public:
    ActorItemGeneratorEntry();

    /* 0x00 */ const char* mName;
    /* 0x04 */ const char* mJointName;
    /* 0x08 */ s32 mTableIndex;
    /* 0x0C */ s32 mCount;
    /* 0x10 */ bool mIsEnabled;
};

// Spawns items (coins / star pieces) for an actor from its "<actor>ItemGenerator" csv.
class ActorItemGenerator {
public:
    ActorItemGenerator(LiveActor*, const char*);

    static ActorItemGenerator* tryCreate(LiveActor*, const char*);

    bool loadEntries(const char*);
    void declareItems();
    ActorItemGeneratorEntry* findEntry(const char*);
    void emitItem(const char*);
    void emitItemAtPos(const char*, const TVec3f*);
    void emitItemAtPosAndDir(const char*, const TVec3f*, const TVec3f*);
    void enableAllEntries();
    void disableAllEntries();
    void appearItem(ActorItemGeneratorEntry*, const TVec3f*);
    void appearItemToDirection(ActorItemGeneratorEntry*, const TVec3f*, const TVec3f*);
    void appearItemToVelocity(ActorItemGeneratorEntry*, const TVec3f*, const TVec3f*);

    /* 0x00 */ LiveActor* mActor;
    /* 0x04 */ ActorItemGeneratorEntry* mEntries;
    /* 0x08 */ s32 mEntryNum;
};
