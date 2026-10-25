#include "Game/Player/MarioAnimator.hpp"
#include "Game/Util.hpp"
#include "Game/Animation/XanimeCore.hpp"
#include "Game/Animation/XanimePlayer.hpp"
#include "Game/Animation/XanimeResource.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/ModelManager.hpp"
#include "Game/Player/J3DModelX.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/MarioConst.hpp"
#include "Game/Player/MarioState.hpp"
#include "Game/Player/Yoshi.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Player/MarioActorHelpers.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <JSystem/JMath/JMATrigonometric.hpp>
#include <cstring>
#include "Game/Util/HashUtil.hpp"
#include "Game/Player/Yoshi.hpp"

// ---- enum names missing from the SMG2 ActorSensorUtil.hpp ----
static const s32 ACTMES_STAR_PIECE_GIFT_1 = 0x10;  // Petari value, SMG2 unverified


struct MarioAnimationCallback {
    const char* mAnimation;
    s32 mType;
    void (MarioAnimator::*mEntry)();
    void (MarioAnimator::*mUpdate)();
    void (MarioAnimator::*mClose)();
    u32 _2C;
};

MarioAnimationCallback marioCallbackTable[] = {
    {"空中ひねり", 0, &MarioAnimator::spinEntry, &MarioAnimator::spinUpdate, &MarioAnimator::spinClose, 0},
    {"地上ひねり", 0, &MarioAnimator::spinEntry, nullptr, &MarioAnimator::spinClose, 0},
    {"アイスひねり", 1, &MarioAnimator::spinEntry, nullptr, &MarioAnimator::spinClose, 0},
    {"アイスひねり静止", 1, &MarioAnimator::spinEntry, nullptr, &MarioAnimator::spinClose, 0},
    {"ファイアスピン", 2, &MarioAnimator::spinEntry, nullptr, &MarioAnimator::spinClose, 0},
    {"ファイアスピン空中", 2, &MarioAnimator::spinEntry, nullptr, &MarioAnimator::spinClose, 0},
    {"ハチスピン", 3, &MarioAnimator::spinEntry, nullptr, &MarioAnimator::spinClose, 0},
    {"ハチスピン空中", 3, &MarioAnimator::spinEntry, nullptr, &MarioAnimator::spinClose, 0},
    {"雲スピン空中", 0, &MarioAnimator::spinEntry, nullptr, &MarioAnimator::spinClose, 0},
    {"雲スピン", 0, &MarioAnimator::spinEntry, nullptr, &MarioAnimator::spinClose, 0},
    {"ステージインA", 0, nullptr, &MarioAnimator::stageInCheck, nullptr, 0},
    {"投げ", 0, &MarioAnimator::throwEntry, &MarioAnimator::throwCheck, &MarioAnimator::throwClose, 0},
    {"ファイア投げ", 1, &MarioAnimator::throwEntry, nullptr, &MarioAnimator::throwClose, 0},
    {"サマーソルト", 0, nullptr, &MarioAnimator::squatSpinCheck, nullptr, 0},
    {"ウォークイン", 0, nullptr, nullptr, &MarioAnimator::walkinClose, 0},
    {"見る", 0, nullptr, nullptr, &MarioAnimator::walkinClose, 0},
    {"ResultWait", 0, nullptr, nullptr, &MarioAnimator::walkinClose, 0},
    {"ResultWaitGrandStar", 0, nullptr, nullptr, &MarioAnimator::walkinClose, 0},
    {"WatchUpMore", 0, nullptr, nullptr, &MarioAnimator::walkinClose, 0},
    {"", 0, nullptr, nullptr, nullptr, 0},
};

void MarioAnimator::initCallbackTable() {
    u32 count = 0;
    MarioAnimationCallback* callback = marioCallbackTable;
    for (;; count++, callback++) {
        if (!*callback->mAnimation) {
            break;
        }
    }

    mCallbackTable = new HashSortTable(count);
    callback = marioCallbackTable;
    for (u32 i = 0; i < count; callback++, i++) {
        mCallbackTable->add(callback->mAnimation, i, false);
    }

    mCallbackTable->sort();
    mCallbackId = -1;
    mCallbackEnded = false;
}

void MarioAnimator::entryCallback(const char* pName) {
    mCallbackEnded = false;
    closeCallback();
    uintptr_t index;
    if (mCallbackTable->search(pName, &index)) {
        mCallbackId = static_cast<s32>(index);
        if (marioCallbackTable[mCallbackId].mEntry != nullptr) {
            (this->*marioCallbackTable[mCallbackId].mEntry)();
        }
    }
}

void MarioAnimator::runningCallback() {
    if (mCallbackId == -1) {
        return;
    }

    mCallbackEnded = true;
    if (isAnimationStop() || isAnimationTerminate(nullptr)) {
        closeCallback();
        return;
    }

    mCallbackEnded = false;
    if (!isAnimationRun(marioCallbackTable[mCallbackId].mAnimation)) {
        closeCallback();
        return;
    }

    if (marioCallbackTable[mCallbackId].mUpdate != nullptr) {
        (this->*marioCallbackTable[mCallbackId].mUpdate)();
    }
}

void MarioAnimator::closeCallback() {
    if (mCallbackId != -1 && marioCallbackTable[mCallbackId].mClose != nullptr) {
        (this->*marioCallbackTable[mCallbackId].mClose)();
    }

    mCallbackId = -1;
}

void MarioAnimator::spinEntry() {
    switch (marioCallbackTable[mCallbackId].mType) {
    case 0:
        if (getPlayer()->isPlayerModeCloud()) mActor->playEffect("雲スピン");
        else mActor->playEffect("スピンライト");
        break;
    case 1:
        mActor->playEffect("アイススピン");
        break;
    case 2:
        mActor->playEffect("ファイアスピン");
        break;
    case 3:
        if (gIsLuigi) {
            mActor->playEffect("ハチルイージスピン");
        } else {
            mActor->playEffect("ハチスピン");
        }

        break;
    }
}

// NONMATCHING: literal relocation labels; scalar and effect bytes verified.
void MarioAnimator::spinUpdate() {
    if (getFrame() > 30.0f) {
        mActor->stopEffect("スピンライト");
    }
}

void MarioAnimator::spinClose() {
    switch (marioCallbackTable[mCallbackId].mType) {
    case 0:
        if (getPlayer()->isPlayerModeCloud()) mActor->stopEffect("雲スピン");
        else mActor->stopEffect("スピンライト");
        break;
    case 1:
        mActor->stopEffect("アイススピン");
        break;
    case 2:
        mActor->stopEffect("ファイアスピン");
        break;
    case 3:
        if (gIsLuigi) {
            mActor->stopEffect("ハチルイージスピン");
        } else {
            mActor->stopEffect("ハチスピン");
        }

        break;
    }
}

#pragma auto_inline off
#pragma dont_inline on
// NONMATCHING: literal-label naming.
void MarioAnimator::stageInCheck() {
    if (static_cast< s32 >(getFrame()) == 50) {
        Mario* player = getPlayer();
        MarioActor* actor = mActor;
        const TVec3f& translation = getPlayer()->MarioModule::getTrans();
        actor->playEffectRT("属性ステージイン", player->_368, translation);
    }
}

void MarioAnimator::throwCheck() {
    if (mActor->_38C == 0 && getPlayer()->MarioModule::getStickP() && !((getPlayer()->mMovementStates.mLowWord & Mario::MovementStates::Jumping) >> 31)) {
        getPlayer()->MarioModule::stopAnimation(nullptr, static_cast<const char*>(nullptr));
    }
}

void MarioAnimator::throwEntry() {
    switch (marioCallbackTable[mCallbackId].mType) {
    case 0:
        mActor->playEffect("こうら投げ");
        break;
    case 1:
        mActor->playEffect("ファイアボール投げ");
        break;
    }
}

void MarioAnimator::throwClose() {
    switch (marioCallbackTable[mCallbackId].mType) {
    case 0:
        mActor->stopEffect("こうら投げ");
        break;
    case 1:
        mActor->stopEffect("ファイアボール投げ");
        break;
    }
}

void MarioAnimator::squatSpinCheck() {
    if (!((getPlayer()->mMovementStates.mLowWord >> 21) & 1U) && getFrame() >= 40.0f) {
        getPlayer()->MarioModule::stopAnimation(nullptr, static_cast<const char*>(nullptr));
    }
}

void MarioAnimator::walkinClose() {
    if (mCallbackEnded) {
        getPlayer()->MarioModule::stopAnimation(nullptr, static_cast<const char*>(nullptr));
        getPlayer()->changeAnimationInterpoleFrame(16);
    }
}
#pragma dont_inline off
#pragma auto_inline on
