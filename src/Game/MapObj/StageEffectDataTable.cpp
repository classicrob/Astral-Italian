#include "Game/MapObj/StageEffectDataTable.hpp"
#include "Game/MapObj/MapObjActor.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/StringUtil.hpp"

namespace {
    const char* const cCameraShakeTypeNames[] = {"なし", "最強", "強", "中の強", "中", "中の弱", "弱", "微弱"};

    inline s32 getCameraShakeTypeIndex(const char* type) {
        if (type == nullptr) return 0;
        for (u32 index = 0; index < 8; index++) {
            if (MR::isEqualString(cCameraShakeTypeNames[index], type)) return index;
        }
        return 0;
    }

    void shakeCamera(const char* type) {
        s32 index = getCameraShakeTypeIndex(type);
        switch (index) {
        case 1: MR::shakeCameraVeryStrong(); break;
        case 2: MR::shakeCameraStrong(); break;
        case 3: MR::shakeCameraNormalStrong(); break;
        case 4: MR::shakeCameraNormal(); break;
        case 5: MR::shakeCameraNormalWeak(); break;
        case 6: MR::shakeCameraWeak(); break;
        case 7: MR::shakeCameraVeryWeak(); break;
        }
    }

    bool isDistanceValidCameraShake(const MapObjActor* actor) {
        f32 distance = actor->mStageEffectCamera->_10;
        if (distance == -1.0f) {
            return MR::isOnPlayer(actor);
        }
        f32 playerDistance = MR::calcDistanceToPlayer(actor);
        return 0.0f < playerDistance && playerDistance < distance;
    }

    bool isDistanceValidPadRumble(const MapObjActor* actor) {
        f32 distance = actor->mStageEffectPad->_C;
        if (distance == -1.0f) {
            return MR::isOnPlayer(actor);
        }
        f32 playerDistance = MR::calcDistanceToPlayer(actor);
        return 0.0f < playerDistance && playerDistance < distance;
    }
}

namespace MR {
    void StageEffect::shakeStartCamera(LiveActor* actor, const char*) {
        MapObjActor* mapActor = static_cast< MapObjActor* >(actor);
        if (mapActor->mStageEffectCamera != nullptr && isDistanceValidCameraShake(mapActor)) {
            shakeCamera(mapActor->mStageEffectCamera->_0);
        }
    }
    void StageEffect::shakeStopCamera(LiveActor* actor, const char*) {
        MapObjActor* mapActor = static_cast< MapObjActor* >(actor);
        if (mapActor->mStageEffectCamera != nullptr && isDistanceValidCameraShake(mapActor)) {
            shakeCamera(mapActor->mStageEffectCamera->_C);
        }
    }
    void StageEffect::shakeCameraMoving(NameObj* actor, const char*) {
        MapObjActor* mapActor = static_cast< MapObjActor* >(actor);
        if (mapActor->mStageEffectCamera == nullptr) return;
        f32 intensity = mapActor->mStageEffectCamera->_4;
        if (intensity <= 0.0f) return;
        MR::shakeCameraInfinity(actor, intensity, mapActor->mStageEffectCamera->_8);
    }
    void StageEffect::stopShakingCameraMoving(NameObj* actor, const char*) {
        MapObjActor* mapActor = static_cast< MapObjActor* >(actor);
        if (mapActor->mStageEffectCamera == nullptr) return;
        f32 intensity = mapActor->mStageEffectCamera->_4;
        if (intensity <= 0.0f) return;
        MR::stopShakingCamera(actor);
    }
    void StageEffect::rumblePadStart(LiveActor* actor, const char*) {
        MapObjActor* mapActor = static_cast< MapObjActor* >(actor);
        if (isDistanceValidPadRumble(mapActor)) {
            const char* pattern = mapActor->mStageEffectPad->_0;
            if (pattern != nullptr) MR::tryRumblePad(actor, pattern, 0);
        }
    }
    void StageEffect::rumblePadStop(LiveActor* actor, const char*) {
        MapObjActor* mapActor = static_cast< MapObjActor* >(actor);
        if (isDistanceValidPadRumble(mapActor)) {
            const char* pattern = mapActor->mStageEffectPad->_8;
            if (pattern != nullptr) MR::tryRumblePad(actor, pattern, 0);
        }
    }
    void StageEffect::rumblePadMoving(LiveActor* actor, const char*) {
        MapObjActor* mapActor = static_cast< MapObjActor* >(actor);
        if (isDistanceValidPadRumble(mapActor)) {
            const char* pattern = mapActor->mStageEffectPad->_4;
            if (pattern != nullptr) MR::tryRumblePad(actor, pattern, 0);
        }
    }
    bool StageEffect::tryStageEffectStart(LiveActor* actor, const char* name) {
        if (MR::isRegisteredEffect(actor, "Start")) MR::emitEffect(actor, "Start");
        rumblePadStart(actor, name);
        shakeStartCamera(actor, name);
        return true;
    }
    bool StageEffect::tryStageEffectMoving(LiveActor* actor, const char* name) {
        if (MR::isRegisteredEffect(actor, "Moving")) MR::emitEffect(actor, "Moving");
        rumblePadMoving(actor, name);
        return true;
    }
    bool StageEffect::tryStageEffectMoving(LiveActor* actor, const char* name, bool isSlow) {
        if (isSlow) {
            if (MR::isRegisteredEffect(actor, "StartNormal") && MR::isEffectValid(actor, "StartNormal")) MR::deleteEffect(actor, "StartNormal");
            if (MR::isRegisteredEffect(actor, "StartSlow") && !MR::isEffectValid(actor, "StartSlow")) MR::emitEffect(actor, "StartSlow");
        } else {
            if (MR::isRegisteredEffect(actor, "StartSlow") && MR::isEffectValid(actor, "StartSlow")) MR::deleteEffect(actor, "StartSlow");
            if (MR::isRegisteredEffect(actor, "StartNormal") && !MR::isEffectValid(actor, "StartNormal")) MR::emitEffect(actor, "StartNormal");
        }
        rumblePadMoving(actor, name);
        return true;
    }
    bool StageEffect::tryStageEffectStop(LiveActor* actor, const char* name) {
        if (MR::isRegisteredEffect(actor, "Stop")) MR::emitEffect(actor, "Stop");
        rumblePadStop(actor, name);
        shakeStopCamera(actor, name);
        return true;
    }
}
