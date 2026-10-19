#include "Game/Player/GhostPlayer.hpp"
#include "Game/Animation/XanimeCore.hpp"
#include "Game/Animation/XanimePlayer.hpp"
#include "Game/LiveActor/ModelManager.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include <cmath>
#include <cstring>

#pragma auto_inline off
f32 getShiftRatio(s8 shiftValue) {
    if (shiftValue > 0) return static_cast<f32>(256 << shiftValue) / 256.0f;
    return static_cast<f32>(256 >> -shiftValue) / 256.0f;
}

namespace MR {
    void fn_8037F4B0(f32* angle) {
        *angle = repeat(*angle, 0.0f, TWO_PI);
        if (*angle > PI) *angle -= TWO_PI;
        *angle *= 1.0f / PI;
    }
    // NONMATCHING: register allocation and integer-extension/store scheduling.
    bool fn_8037F5B0(const TVec3f& input, s8 shift, TVec3s* output) {
        f32 factor = getShiftRatio(shift);
        f32 x = input.x * factor, y = input.y * factor, z = input.z * factor;
        // Supported Wii/Mac integer narrowing retains the low16 bits after signed32 truncation.
        s16 ix = static_cast<s16>(static_cast<s32>(x));
        s16 iy = static_cast<s16>(static_cast<s32>(y));
        s16 iz = static_cast<s16>(static_cast<s32>(z));
        output->x = ix;
        output->y = iy;
        output->z = iz;
        if (ix != static_cast<s32>(x) || iy != static_cast<s32>(y) || iz != static_cast<s32>(z)) return true;
        return false;
    }
    // NONMATCHING: register allocation and byte-extension scheduling.
    bool fn_8037F6F0(const TVec3f& input, s8 shift, TVec3Sc* output) {
        f32 factor = getShiftRatio(shift);
        f32 x = input.x * factor, y = input.y * factor, z = input.z * factor;
        s32 ix = static_cast<s32>(x), iy = static_cast<s32>(y), iz = static_cast<s32>(z);
        output->x = ix;
        output->y = iy;
        output->z = iz;
        if (static_cast<s8>(static_cast<u8>(ix)) != static_cast<s32>(x) || static_cast<s8>(static_cast<u8>(iy)) != static_cast<s32>(y) || static_cast<s8>(static_cast<u8>(iz)) != static_cast<s32>(z)) return true;
        return false;
    }
    // NONMATCHING: integer register allocation and load/store scheduling after signed32 truncation.
    bool fn_8037F680(f32 input, s8 shift, s16* output) {
        f32 value = input * getShiftRatio(shift);
        // Supported Wii/Mac integer narrowing retains the low16 bits after signed32 truncation.
        s16 result = static_cast<s16>(static_cast<s32>(value));
        *output = result;
        return result != static_cast<s32>(value);
    }
    bool fn_8037F7D0(f32 input, s8 shift, s8* output) {
        f32 value = input * getShiftRatio(shift);
        u8 result = static_cast<s8>(static_cast<u8>(static_cast<s32>(value)));
        *output = result;
        return static_cast<s8>(result) != static_cast<s32>(value);
    }
    // NONMATCHING: floating-point register allocation.
    void convToFloat(TVec3s& input, s8 shift, TVec3f* output) {
        f32 factor = 1.0f / getShiftRatio(shift);
        f32 x = static_cast<f32>(input.x) * factor;
        f32 y = static_cast<f32>(input.y) * factor;
        f32 z = static_cast<f32>(input.z) * factor;
        output->x = x;
        output->y = y;
        output->z = z;
    }
    // NONMATCHING: floating-point register allocation.
    void convToFloat(TVec3Sc& input, s8 shift, TVec3f* output) {
        f32 factor = 1.0f / getShiftRatio(shift);
        f32 x = static_cast<f32>(input.x) * factor;
        f32 y = static_cast<f32>(input.y) * factor;
        f32 z = static_cast<f32>(input.z) * factor;
        output->x = x;
        output->y = y;
        output->z = z;
    }
    void convToFloat(s16 input, s8 shift, f32* output) {
        f32 factor = 1.0f / getShiftRatio(shift);
        *output = static_cast<f32>(input) * factor;
    }

    bool fn_8037FA30(TVec3s* output, MtxPtr matrix) {
        TVec3f position;
        extractMtxTrans(matrix, &position);
        return fn_8037F5B0(position, -2, output);
    }

    void fn_8037FA80(TVec3Sc* output, const TVec3f& rotation) {
        TVec3f normalized(rotation);
        fn_8037F4B0(&normalized.x);
        fn_8037F4B0(&normalized.y);
        fn_8037F4B0(&normalized.z);
        fn_8037F6F0(normalized, 7, output);
    }


    void fn_8037FAF0(TVec3Sc* output, MtxPtr matrix) {
        TVec3f rotation;
        TPos3f transform;
        transform.set(matrix);
        transform.getEuler(rotation);
        fn_8037FA80(output, rotation);
    }

#pragma auto_inline off
    void fn_8037FC30(TVec3Sc* output, const TVec3f& scale) { fn_8037F6F0(scale, 3, output); }
    void fn_8037FC40(TVec3Sc* output, const TVec3f& velocity) { fn_8037F6F0(velocity, 0, output); }
    void fn_8037FC50(s16* output, f32 frame) { fn_8037F680(frame, 5, output); }
    void fn_8037FC60(s8* output, f32 weight) { fn_8037F7D0(weight, 7, output); }
    void fn_8037FC70(s8* output, f32 rate) { fn_8037F7D0(rate, 3, output); }

    void convToFloatRotation(f32* output, s8 input) {
        f32 value = static_cast<f32>(input) * (1.0f / getShiftRatio(7));
        value *= 180.0f;
        *output = clamp(value, -180.0f, 180.0f);
    }
    void convToFloatTrackWeight(f32* output, s8 input) {
        if (static_cast<u8>(input) == 128) { *output = 1.0f; return; }
        *output = static_cast<f32>(input) * (1.0f / getShiftRatio(7));
    }
    void convToFloatBckRate(f32* output, s8 input) { *output = static_cast<f32>(input) * (1.0f / getShiftRatio(3)); }

    u32 fn_8037FE20(u32 flags) {
        u32 length = 4;
        if (flags & 0x1) length += 6;
        if (flags & 0x4000) length += 12;
        if (flags & 0x800) length += 3;
        if (flags & 0x400) length += 3;
        if (flags & 0x2) length += 1;
        if (flags & 0x4) length += 1;
        if (flags & 0x8) length += 1;
        if (flags & 0x2000) length += 4;
        if (flags & 0x20) length += 2;
        if (flags & 0x40) length += 1;
        if (flags & 0x80) length += 1;
        if (flags & 0x100) length += 1;
        if (flags & 0x200) length += 1;
        if (flags & 0x1000) length += 1;
        return length;
    }

    bool fn_8037FEE0(const char* name) {
        const char* special[] = {"基本", "しゃがみ基本", "壁押し", "壁右歩き", "壁左歩き", "しゃがみ歩き", "がんばり走り", "スケートアクセルジャンプ", "スケートジャンプ2", "スケートジャンプ3", "GhostStartWait", "GhostStarWait", "GhostStarWaitAir"};
        for (u32 index = 0; index < 13; ++index) if (isEqualString(name, special[index])) return true;
        return false;
    }

    XanimePlayer* fn_8037FF80(LiveActor* actor, XanimeResourceTable* resource) {
        if (resource == nullptr) resource = getPlayerXanimeResource();
        XanimePlayer* player = new XanimePlayer(getJ3DModel(actor), resource);
        player->duplicateSimpleGroup();
        actor->mModelManager->mXanimePlayer = player;
        XanimeCore* core = player->mCore;
        core->enableJointTransform(getJ3DModelData(actor));
        player->mFrameCtrl->mRate = 0.33f;
        return player;
    }
    XanimePlayer* fn_80380030(LiveActor* actor, XanimePlayer* main, XanimeResourceTable* resource) {
        if (resource == nullptr) resource = getPlayerXanimeResource();
        XanimePlayer* player = new XanimePlayer(getJ3DModel(actor), resource, main);
        player->changeAnimationBck("基本");
        return player;
    }
    void fn_803800C0(XanimePlayer* player, const char* name) {
        player->changeAnimationBck(name);
        if (player->mFrameCtrl->mAttribute == 0) player->mFrameCtrl->mAttribute = 1;
        if (!fn_8037FEE0(name)) player->mFrameCtrl->mAttribute = 1;
    }
    void fn_80380130(XanimePlayer* player, XanimePlayer* upper, LiveActor* actor, u32 hash, bool* hasTurtle) {
        player->changeAnimationByHash(hash);
        bool turtle = false;
        const char* name = player->getCurrentAnimationName();
        if (strstr(name, "水泳ジェット")) turtle = true;
        else if (strstr(name, "カメ持ち")) turtle = true;
        else if (strstr(name, "投げ")) {
            turtle = false;
            upper->stopAnimation();
            u16 joint = getJointIndex(actor, "Spine1");
            getJ3DModelData(actor)->getJointNodePointer(joint)->setMtxCalc(nullptr);
        }
        if (turtle) {
            if (isEqualString(name, "基本") || isEqualString(name, "ジャンプ")) {
                upper->changeAnimationBck("ひろいウエイト");
                upper->overWriteMtxCalc(getJointIndex(actor, "PartsControl"));
            }
            if (hasTurtle != nullptr) *hasTurtle = turtle;
        }
        if (!fn_8037FEE0(name)) player->mFrameCtrl->mAttribute = 1;
    }
}

