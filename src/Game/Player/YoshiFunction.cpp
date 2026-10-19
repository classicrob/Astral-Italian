#include "Game/Player/YoshiFunction.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/MarioHolder.hpp"
#include "Game/Player/Yoshi.hpp"
#include "Game/Player/YoshiHolder.hpp"
#include "Game/Player/YoshiLockOnManager.hpp"
#include "Game/Player/YoshiLockOnTarget.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"

namespace YoshiFunction {
#pragma global_optimizer off
    bool tryYoshiPushEat() {
        if (!MR::getMarioHolder()->getMarioActor()->fn_803B9840()) return false;
        if (!MR::isPlayerElementModeYoshi()) return false;
        return MR::getMarioHolder()->getMarioActor()->fn_803B9810()->fn_8041F970();
    }
#pragma global_optimizer reset
#pragma global_optimizer off
    bool tryYoshiEat(YoshiLockOnTarget* target) {
        if (!MR::getMarioHolder()->getMarioActor()->fn_803B9840()) return false;
        if (!MR::isPlayerElementModeYoshi()) return false;
        target->mValid=true;
        if (!MR::isStarPointerPointing1P(target->mActor,"弱",true,false)) return false;
        return static_cast<YoshiLockOnManager*>(MR::getMarioHolder()->getMarioActor()->fn_803B9820())->fn_80419F90(target);
    }
#pragma global_optimizer reset
#pragma global_optimizer off
    bool tryYoshiLick(YoshiLockOnTarget* target) {
        if (!MR::getMarioHolder()->getMarioActor()->fn_803B9840()) return false;
        if (!MR::isPlayerElementModeYoshi()) return false;
        target->mValid=true;
        if (!MR::isStarPointerPointing1P(target->mActor,"弱",true,false)) return false;
        return static_cast<YoshiLockOnManager*>(MR::getMarioHolder()->getMarioActor()->fn_803B9820())->fn_80419FB0(target);
    }
#pragma global_optimizer reset
#pragma global_optimizer off
    bool tryYoshiCapture(YoshiLockOnTarget* target, bool flag) {
        if (!MR::getMarioHolder()->getMarioActor()->fn_803B9840()) return false;
        if (!MR::isPlayerElementModeYoshi()) return false;
        target->mValid=true;
        if (!MR::isStarPointerPointing1P(target->mActor,"弱",true,false)) return false;
        return static_cast<YoshiLockOnManager*>(MR::getMarioHolder()->getMarioActor()->fn_803B9820())->fn_80419FD0(target, flag);
    }
#pragma global_optimizer reset
#pragma global_optimizer off
    bool tryYoshiPull(YoshiLockOnTarget* target) {
        if (!MR::getMarioHolder()->getMarioActor()->fn_803B9840()) return false;
        if (!MR::isPlayerElementModeYoshi()) return false;
        target->mValid=true;
        if (!MR::isStarPointerPointing1P(target->mActor,"弱",true,false)) return false;
        return static_cast<YoshiLockOnManager*>(MR::getMarioHolder()->getMarioActor()->fn_803B9820())->fn_80419FF0(target);
    }
#pragma global_optimizer reset
#pragma global_optimizer off
    bool tryYoshiProjectile(YoshiLockOnTarget* target) {
        if (!MR::getMarioHolder()->getMarioActor()->fn_803B9840()) return false;
        if (!MR::isPlayerElementModeYoshi()) return false;
        target->mValid=true;
        if (!MR::isStarPointerPointing1P(target->mActor,"弱",true,false)) return false;
        return static_cast<YoshiLockOnManager*>(MR::getMarioHolder()->getMarioActor()->fn_803B9820())->fn_8041A000(target);
    }
#pragma global_optimizer reset
    void emitGlowEffect() {
        if (MR::isPlayerElementModeYoshi()) MR::getMarioHolder()->getMarioActor()->fn_803B97F0()->fn_8040F030();
    }
    void deleteGlowEffect() {
        if (MR::isPlayerElementModeYoshi()) MR::getMarioHolder()->getMarioActor()->fn_803B97F0()->fn_8040F050();
    }
}

namespace MR {
#pragma global_optimizer off
    bool fn_80422150() {
        if (!isPlayerElementModeYoshi()) return false;
        return getMarioHolder()->getMarioActor()->fn_803B97F0()->fn_8040F070();
    }
#pragma global_optimizer reset
    void fn_80422190() {
        if (!isPlayerElementModeYoshi()) return;
        getMarioHolder()->getMarioActor()->fn_803B97F0()->fn_8040F080();
    }
    void fn_804221D0() {
        if (!isPlayerElementModeYoshi()) return;
        getMarioHolder()->getMarioActor()->fn_803B97F0()->fn_8040F0A0();
    }
    void fn_80422210() {
        if (!isPlayerElementModeYoshi()) return;
        getMarioHolder()->getMarioActor()->fn_803B97F0()->fn_8040F0B0();
    }
    void fn_80422250() {
        if (!isPlayerElementModeYoshi()) return;
        getMarioHolder()->getMarioActor()->fn_803B97F0()->fn_8040F0D0();
    }
#pragma global_optimizer off
    bool fn_80422290() {
        if (!isPlayerElementModeYoshi()) return false;
        return getMarioHolder()->getMarioActor()->fn_803B9810()->fn_8041FB60();
    }
#pragma global_optimizer reset
#pragma global_optimizer off
    bool fn_804222D0() {
        if (!isPlayerElementModeYoshi()) return false;
        return getMarioHolder()->getMarioActor()->fn_803B9810()->fn_8041FBD0();
    }
#pragma global_optimizer reset
#pragma global_optimizer off
    bool fn_80422310() {
        if (!isPlayerElementModeYoshi()) return false;
        return getMarioHolder()->getMarioActor()->fn_803B9810()->fn_8041FC60();
    }
#pragma global_optimizer reset
    f32 fn_80422390() {
        return static_cast<YoshiTongue*>(getMarioHolder()->getMarioActor()->fn_803B9800())->fn_8041CEC0();
    }
    void fn_804224F0() {
        getMarioHolder()->getMarioActor()->fn_803B9830()->fn_8041B920();
    }
    void fn_80422520() {
        getMarioHolder()->getMarioActor()->fn_803B9830()->fn_8041B970();
    }
    void fn_80422550() {
        getMarioHolder()->getMarioActor()->fn_803B9830()->fn_8041B9C0();
    }
    void fn_80422580() {
        if (!getMarioHolder()->getMarioActor()->fn_803B9840()) return;
        getMarioHolder()->getMarioActor()->fn_803B9810()->fn_8041FE00();
    }
    bool fn_804225C0() {
        return getMarioHolder()->getMarioActor()->fn_803B9810()->fn_8041FF60();
    }
    bool fn_80422710() {
        return getMarioHolder()->getMarioActor()->fn_803B9810()->fn_8041FA40();
    }
    void resetYoshiTongueForDemo() {
        if (!getMarioHolder()->getMarioActor()->fn_803B9840()) return;
        if (!isPlayerElementModeYoshi()) return;
        getMarioHolder()->getMarioActor()->fn_803B9810()->fn_8041FEB0();
    }
    void fn_804228B0() {
        if (!getMarioHolder()->getMarioActor()->fn_803B9840()) return;
        getMarioHolder()->getMarioActor()->fn_803B97F0()->fn_8040F310();
    }
    void fn_80422350(TPos3f* out) { static_cast<YoshiTongue*>(getMarioHolder()->getMarioActor()->fn_803B9800())->fn_8041CD10(out); }
    void fn_804223C0(const JMapInfoIter& iter) { getMarioHolder()->getMarioActor()->fn_803B97F0()->fn_8040F230(iter); }
    void fn_80422400(LiveActor* actor) { getMarioHolder()->getMarioActor()->fn_803B9830()->fn_8041B820(actor); }
    void fn_80422440(LiveActor* actor) { getMarioHolder()->getMarioActor()->fn_803B9830()->fn_8041B840(actor); }
    void fn_80422480(LiveActor* actor) { getMarioHolder()->getMarioActor()->fn_803B9830()->fn_8041B8B0(actor); }
    bool fn_804224C0() { return getMarioHolder()->getMarioActor()->fn_803B9830()->_34; }
    void startDashYoshiMode() { getMarioHolder()->getMarioActor()->fn_803B97F0()->fn_8040F110(); }
#pragma global_optimizer off
    bool isPlayerElementModeYoshiDash() { if (!isPlayerElementModeYoshi()) return false;return getMarioHolder()->getMarioActor()->fn_803B97F0()->mDashTimer>0; }
#pragma global_optimizer reset
#pragma global_optimizer off
    bool isPlayerElementModeYoshiLight() { if (!isPlayerElementModeYoshi()) return false;return getMarioHolder()->getMarioActor()->fn_803B97F0()->fn_8040F070(); }
#pragma global_optimizer reset
    const TVec3f& fn_804226B0() { return getMarioHolder()->getMarioActor()->fn_803B97F0()->mPosition; }
    const TVec3f& fn_804226E0() { return getMarioHolder()->getMarioActor()->fn_803B97F0()->mRotation; }
    void fn_80422790(MtxPtr mtx,LiveActor* actor) { if (getMarioHolder()->getMarioActor()->fn_803B9840()) getMarioHolder()->getMarioActor()->fn_803B9830()->fn_8041BA20(mtx,actor); }
    bool isMsgYoshiStartTongue(u32 msg) { return msg==0xf8; }
    bool isMsgYoshiLoopTongue(u32 msg) { return msg==0xf9; }
    bool isMsgYoshiReleaseEaten(u32 msg) { return msg==0xfa; }
    bool isMsgYoshiStartEaten(u32 msg) { return msg==0xfb; }
    bool isMsgYoshiCancelEaten(u32 msg) { return msg==0xfc; }
    bool isMsgYoshiReleaseOrCancelEaten(u32 msg) { return msg==0xFA || msg==0xFC; }
    bool isMsgYoshiAutoEatStart(u32 msg) { return msg==0xff; }
    bool isMsgYoshiAutoEatEnd(u32 msg) { return msg==0x100; }
    bool fn_80422880(u32 msg) { return msg==0xfd; }
    bool fn_80422890(u32 msg) { return msg==0xfe; }
    bool isMsgDashYoshiAttack(u32 msg) { return msg==0xf7; }
#pragma global_optimizer off
    MtxPtr getYoshiMouthMtx() { if (!getMarioHolder()->getMarioActor()->fn_803B9840()) return nullptr;return getMarioHolder()->getMarioActor()->fn_803B97F0()->fn_8040F300(); }
#pragma global_optimizer reset
// NONMATCHING: equivalent final early-return blocks are merged.
    void incYoshiFruitEaten() { if (!getMarioHolder()->getMarioActor()->fn_803B9840()) return;if (!isPlayerElementModeYoshi()) return;getMarioHolder()->getMarioActor()->fn_803B9830()->fn_8041BA10(); }
}
