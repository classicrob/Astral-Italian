#pragma once

#include <revolution.h>

#include "Game/Map/HitInfo.hpp"
#include "Game/Util/JMapInfo.hpp"

class CodeTable;

// Petari (SMG1) code enums; SMG2 values unverified
enum CollisionFloorCode {
    CollisionFloorCode_Normal,
    CollisionFloorCode_Death,
    CollisionFloorCode_Slip,
    CollisionFloorCode_NoSlip,
    CollisionFloorCode_DamageNormal,
    CollisionFloorCode_Ice,
    CollisionFloorCode_JumpLow,
    CollisionFloorCode_JumpMiddle,
    CollisionFloorCode_JumpHigh,
    CollisionFloorCode_Slider,
    CollisionFloorCode_DamageFire,
    CollisionFloorCode_JumpNormal,
    CollisionFloorCode_FireDance,
    CollisionFloorCode_Sand,
    CollisionFloorCode_Glass,
    CollisionFloorCode_DamageElectric,
    CollisionFloorCode_PullBack,
    CollisionFloorCode_Sink,
    CollisionFloorCode_SinkPoison,
    CollisionFloorCode_Slide,
    CollisionFloorCode_WaterBottomH,
    CollisionFloorCode_WaterBottomM,
    CollisionFloorCode_WaterBottomL,
    CollisionFloorCode_Wet,
    CollisionFloorCode_Needle,
    CollisionFloorCode_SinkDeath,
    CollisionFloorCode_Snow,
    CollisionFloorCode_RailMove,
    CollisionFloorCode_AreaMove,
    CollisionFloorCode_Press,
    CollisionFloorCode_NoStampSand,
    CollisionFloorCode_SinkDeathMud,
    CollisionFloorCode_Brake,
    CollisionFloorCode_GlassIce,
    CollisionFloorCode_JumpParasol,

    CollisionFloorCode_End,
};

enum CollisionWallCode {
    CollisionWallCode_Normal,
    CollisionWallCode_NotWallJump,
    CollisionWallCode_NotWallSlip,
    CollisionWallCode_NotGrab,
    CollisionWallCode_GhostThroughCode,
    CollisionWallCode_NotSideStep,
    CollisionWallCode_Rebound,
    CollisionWallCode_Fur,
    CollisionWallCode_NoAction,

    CollisionWallCode_End,
};

enum CollisionSoundCode {
    CollisionSoundCode_Null,
    CollisionSoundCode_Soil,
    CollisionSoundCode_Lawn,
    CollisionSoundCode_Stone,
    CollisionSoundCode_Marble,
    CollisionSoundCode_WoodThick,
    CollisionSoundCode_WoodThin,
    CollisionSoundCode_Metal,
    CollisionSoundCode_Snow,
    CollisionSoundCode_Ice,
    CollisionSoundCode_Shallow,
    CollisionSoundCode_Sand,
    CollisionSoundCode_Beach,
    CollisionSoundCode_Carpet,
    CollisionSoundCode_Mud,
    CollisionSoundCode_Honey,

    CollisionSoundCode_End,
};

enum CollisionCameraCode {
    CollisionCameraCode_NoThrough,
    CollisionCameraCode_Through,

    CollisionCameraCode_End,
};

class CollisionCode {
public:
    CollisionCode();

    const char* getFloorCodeString(const Triangle&);
    const char* getWallCodeString(const Triangle&);
    const char* getSoundCodeString(const Triangle&);
    const char* getSoundCodeString(const JMapInfoIter&);

    u32 getCameraID(const Triangle&);
    u32 getCode(const JMapInfoIter&, CodeTable*, const char*);

    void createFloorTable();
    void createWallTable();
    void createSoundTable();
    void createCameraTable();

    u32 getFloorCode(const JMapInfoIter&);
    u32 getWallCode(const JMapInfoIter&);
    u32 getSoundCode(const JMapInfoIter&);
    u32 getCameraCode(const JMapInfoIter&);
    u32 getCameraID(const JMapInfoIter&);

    CodeTable* mFloorTable;   // 0x0
    CodeTable* mWallTable;    // 0x4
    CodeTable* mSoundTable;   // 0x8
    CodeTable* mCameraTable;  // 0xC
};

class CodeTable {
public:
    CodeTable(u32);

    void add(const char*, u32);
    const char* getString(u32);

    u32 mNumCodes;     // 0x0
    u32 mMaxNumCodes;  // 0x4
    u32* mHashCodes;   // 0x8
    u32* _C;
    const char** mNames;  // 0x10
};
