#pragma once

#include "Game/NPC/NPCActor.hpp"

class HoneyBee : public NPCActor {
public:
    enum InitAction {
        /* 0x8 */ InitAction_Wait,
        /* 0x9 */ InitAction_JumpLecture,
        /* 0xA */ InitAction_Fly,
        /* 0xB */ InitAction_Sleep,
        /* 0xC */ InitAction_Stick,
        /* 0xD */ InitAction_Gatekeeper,
        /* 0xE */ InitAction_DropLecture,
        /* 0xF */ InitAction_Flustered,
        /* 0x10 */ InitAction_FlyLectureA,
        /* 0x11 */ InitAction_FlyLectureB,
    };

    /// @brief Creates a new `HoneyBee`.
    /// @param pName A pointer to the null-terminated name of the object.
    HoneyBee(const char* pName);

    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void calcAndSetBaseMtx();

    static void makeArchiveList(NameObjArchiveListCollector*, const JMapInfoIter&);

    void exeWait();
    void exeReaction();
    void exeFly();
    void exeJumpLecture();

    /* 0x164 */ s32 mCurrentRailPoint;
    /* 0x168 */ TMtx34f mSmoothedCenterMtx;
    /* 0x198 */ TVec3f mCenterPosition;
    /* 0x1A4 */ s32 mArg0;
    /* 0x1A8 */ f32 mFloatOffset;
};
