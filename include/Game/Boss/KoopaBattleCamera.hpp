#pragma once

#include <JSystem/JGeometry.hpp>

class LiveActor;

// Programmable camera helper of the Bowser battle stages (owned by KoopaBattleStage, built at 0x4C bytes).
class KoopaBattleCamera {
public:
    KoopaBattleCamera(LiveActor*);  // fn_800C9900

    void startFacePlayer();   // fn_800C9980
    void startFromCamera();   // fn_800C9A30
    void end();               // fn_800C9AC0
    void update();            // fn_800C9B00
    void updateQuat();        // fn_800C9B40
    void applyCamera();       // fn_800C9CA0

    /* 0x00 */ LiveActor* mHost;
    /* 0x04 */ TQuat4f mQuat;
    /* 0x14 */ TVec3f mPlayerPos;
    /* 0x20 */ TVec3f mHostPos;
    /* 0x2C */ u8 _2C[0x18];
    /* 0x44 */ f32 _44;
    /* 0x48 */ bool mIsActive;
};

// Programmable camera helper of the last battle (KoopaStateLastBattle, 0x30 bytes).
class KoopaLastBattleCamera {
public:
    KoopaLastBattleCamera(LiveActor*);  // fn_800C9F50

    void start(f32);       // fn_800C9FD0
    void end();            // fn_800CA0B0
    void update();         // fn_800CA0F0
    void updateQuat();     // fn_800CA130
    void applyCamera();    // fn_800CA1F0

    /* 0x00 */ LiveActor* mHost;
    /* 0x04 */ TQuat4f mQuat;
    /* 0x14 */ TVec3f mPlayerPos;
    /* 0x20 */ TVec3f mHostPos;
    /* 0x2C */ bool mIsActive;
};
