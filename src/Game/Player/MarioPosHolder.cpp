#include "Game/Player/MarioPosHolder.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"

MarioPosHolder::MarioPosHolder() :
    mPreviousPosition(0.0f), mPos(0.0f, 0.0f, 0.0f), _18(false), _19(false),
    mPushDepth(0), mSavedPosition(0.0f, 0.0f, 0.0f), mLocked(false) {}

void MarioPosHolder::fn_80405EB0() {
    mPreviousPosition = mPos;
    _19 = true;
    _18 = false;
}

void MarioPosHolder::fn_80405EF0(s32 status) {
    if (!_19) {
        return;
    }
    _19 = false;
    if (static_cast<u32>(status) == 0x13 || static_cast<u32>(status) == 0x21 || static_cast<u32>(status) == 6 || static_cast<u32>(status) == 2) {
        return;
    }
    if (fn_804060D0()) {
        mPos = mPreviousPosition;
    }
    if (static_cast<u32>(status - 8) <= 2 || status == 0) {
        fn_804062A0();
    }
}

void MarioPosHolder::fn_80405F90(const TVec3f& pos) {
    if (!mLocked) {
        if (_19 && PSVECDistance(&mPos, &pos) > 200.0f) {
            _18 = true;
        }
        mPos.set(pos);
    }
}

void MarioPosHolder::fn_80406010(const TVec3f& offset) {
    if (!mLocked) {
        mPos += offset;
    }
}

void MarioPosHolder::fn_80406040() {
    mSavedPosition = mPos;
    mPushDepth++;
}

void MarioPosHolder::fn_80406080() {
    mPos = mSavedPosition;
    mPushDepth--;
}

void MarioPosHolder::fn_804060C0() {
    mLocked = true;
}

bool MarioPosHolder::fn_804060D0() {
    TVec3f oldTop(mPreviousPosition - *MR::getPlayerGravity() * 80.0f);
    TVec3f newTop(mPos - *MR::getPlayerGravity() * 80.0f);
    if (MR::isPlayerElementModeYoshi()) {
        if (Collision::checkStrikeLineToDashYoshiStepAndMap(oldTop, mPreviousPosition - oldTop, 0, nullptr, nullptr)) {
            return false;
        }
    } else {
        if (Collision::checkStrikeLineToMap(oldTop, mPreviousPosition - oldTop, 0, nullptr, nullptr)) {
            return false;
        }
    }
    if (MR::isPlayerElementModeYoshi()) {
        if (Collision::checkStrikeLineToDashYoshiStepAndMap(oldTop, newTop - oldTop, 0, nullptr, nullptr)) {
            if (!Collision::checkStrikeLineToDashYoshiStepAndMap(newTop, oldTop - newTop, 0, nullptr, nullptr)) {
                return true;
            }
        }
    } else {
        if (Collision::checkStrikeLineToMap(oldTop, newTop - oldTop, 0, nullptr, nullptr)) {
            if (!Collision::checkStrikeLineToMap(newTop, oldTop - newTop, 0, nullptr, nullptr)) {
                return true;
            }
        }
    }
    return false;
}

void MarioPosHolder::fn_804062A0() {
    TVec3f top(mPos - *MR::getPlayerGravity() * 80.0f);
    TVec3f hitPos;
    Triangle triangle;
    if (MR::isPlayerElementModeYoshi()) {
        if (!MR::getFirstPolyOnLineToDashYoshiStepAndMap(&hitPos, &triangle, top, mPos - top)) {
            return;
        }
    } else {
        if (!MR::getFirstPolyOnLineToMap(&hitPos, &triangle, top, mPos - top)) {
            return;
        }
    }
    if (PSVECDistance(&mPos, &hitPos) < 2.0f) {
        return;
    }
    if (!triangle.isHostMoved()) {
        mPos = hitPos;
    }
}
