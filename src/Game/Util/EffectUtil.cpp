#include "Game/Util/EffectUtil.hpp"
#include "Game/Util.hpp"
#include "Game/Effect/EffectSystemUtil.hpp"
#include "Game/Effect/MultiEmitter.hpp"
#include "Game/Effect/AutoEffectInfo.hpp"
#include "Game/Effect/MultiEmitterParticleCallBack.hpp"
#include "Game/LiveActor/EffectKeeper.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Screen/LayoutActor.hpp"
#include "Game/Screen/PaneEffectKeeper.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MtxUtil.hpp"

namespace {
    const s32 sAttributeEffectCodes[] = {0x17, 0xD, 5, 0xA, 0x20, 0x1A, 0};
    EffectKeeper* getEffectKeeper(const LiveActor* pActor) {
        return pActor->mEffectKeeper;
    }

};  // namespace

namespace MR {
    bool isExistEffect(const LiveActor*, const char*) NO_INLINE;
    bool isExistEffect(const LiveActor* pActor, const char* pEffectName) {
        return isExistEffectKeeper(pActor) && isRegisteredEffect(pActor, pEffectName);
    }

    void requestEffectStopSceneStartAndDelete() {
        Effect::forceDeleteAllOneTimeEmitter();
        Effect::requestMovementOffAllEmitters();
    }

    void requestEffectStopSceneStart() {
        Effect::requestMovementOffAllLoopEmitters();
    }

    void requestEffectStopSceneEnd() {
        Effect::requestMovementOnAllEmitters();
    }

    void addEffect(LiveActor* pActor, const char* pEffectName) {
        ::getEffectKeeper(pActor)->addEffect(pEffectName, pActor);
    }

    MultiEmitter* getEffect(const LiveActor* pActor, const char* pEffectName) {
        const EffectKeeper* pKeeper = ::getEffectKeeper(pActor);
        return pKeeper->getEmitter(pEffectName);
    }

    MultiEmitter* getEffect(const LayoutActor* pActor, const char* pEffectName) {
        return pActor->mEffectKeeper->getEmitter(pEffectName);
    }

    bool isExistEffectKeeper(const LiveActor* pActor) {
        return ::getEffectKeeper(pActor) != nullptr;
    }

    bool isExistEffectKeeper(const LayoutActor* pActor) {
        return pActor->mEffectKeeper != nullptr;
    }

    MultiEmitter* emitEffect(LiveActor* pActor, const char* pEffectName) {
        return ::getEffectKeeper(pActor)->createEmitter(pEffectName);
    }

    MultiEmitter* emitEffectWithScale(LiveActor* pActor, const char* pEffectName, f32 a3, s32 a4) {
        MultiEmitter* pEmitter;

        pEmitter = emitEffect(pActor, pEffectName);
        pEmitter->setGlobalScale(TVec3f(a3, a3, a3), a4);

        return pEmitter;
    }

    MultiEmitter* emitEffectWithEmitterCallBack(LiveActor* pActor, const char* pEffectName, MultiEmitterCallBackBase* pCallBack) {
        MultiEmitter* pEmitter;

        pEmitter = getEffect(pActor, pEffectName);
        pEmitter->createEmitterWithCallBack(pCallBack);

        return pEmitter;
    }

    MultiEmitter* emitEffectWithParticleCallBack(LiveActor* pActor, const char* pEffectName, MultiEmitterParticleCallBack* pCallBack) {
        MultiEmitter* pEmitter;

        pEmitter = emitEffect(pActor, pEffectName);
        pEmitter->setParticleCallBackPtr(pCallBack, -1);

        return pEmitter;
    }


    // NONMATCHING: register/stack scheduling or equivalent branch lowering; calls, values and fields audited (FP order/grouping checked: no FP instructions).
    MultiEmitter* tryEmitEffect(LiveActor* pActor, const char* pEffectName) {
        if (!isExistEffect(pActor, pEffectName)) {
            return nullptr;
        }
        return ::getEffectKeeper(pActor)->createEmitter(pEffectName);
    }

    bool tryDeleteEffect(LiveActor* pActor, const char* pEffectName) {
        if (isExistEffect(pActor, pEffectName) == false) {
            return false;
        }

        ::getEffectKeeper(pActor)->deleteEmitter(pEffectName);

        return true;
    }

    bool tryForceDeleteEffect(LiveActor* pActor, const char* pEffectName) {
        if (!isExistEffect(pActor, pEffectName)) {
            return false;
        }
        ::getEffectKeeper(pActor)->forceDeleteEmitter(pEffectName);
        return true;
    }

    bool tryDeleteEffectAll(LiveActor* pActor) {
        EffectKeeper* keeper = ::getEffectKeeper(pActor);
        if (!keeper) return false;
        keeper->deleteEmitterAll();
        return true;
    }

    bool tryForceDeleteEffectAll(LiveActor* pActor) {
        EffectKeeper* keeper = ::getEffectKeeper(pActor);
        if (!keeper) return false;
        keeper->forceDeleteEmitterAll();
        return true;
    }

    void deleteEffect(LiveActor* pActor, const char* pEffectName) {
        ::getEffectKeeper(pActor)->deleteEmitter(pEffectName);
    }

    void forceDeleteEffect(LiveActor* pActor, const char* pEffectName) {
        ::getEffectKeeper(pActor)->forceDeleteEmitter(pEffectName);
    }

    void deleteEffectAll(LiveActor* pActor) {
        ::getEffectKeeper(pActor)->deleteEmitterAll();
    }

    void forceDeleteEffectAll(LiveActor* pActor) {
        ::getEffectKeeper(pActor)->forceDeleteEmitterAll();
    }

    bool isRegisteredEffect(const LiveActor* pActor, const char* pEffectName) {
        if (pEffectName != nullptr) {
            return ::getEffectKeeper(pActor)->isRegisteredEmitter(pEffectName);
        } else {
            return isExistEffectKeeper(pActor);
        }
    }

    bool isEffectValid(const LiveActor* pActor, const char* pEffectName) {
        MultiEmitter* pEmitter = getEffect(pActor, pEffectName);

        if (pEmitter != nullptr) {
            return pEmitter->isValid();
        } else {
            return false;
        }
    }

    void onDrawEffect(LiveActor* pActor) {
        ::getEffectKeeper(pActor)->onDraw();
    }

    void offDrawEffect(LiveActor* pActor) {
        ::getEffectKeeper(pActor)->offDraw();
    }

    void onCalcEmitter(LiveActor* pActor) {
        ::getEffectKeeper(pActor)->onCalcEmitter();
    }

    void offCalcEmitter(LiveActor* pActor) {
        ::getEffectKeeper(pActor)->offCalcEmitter();
    }

    void pauseOffEffectAll(LiveActor* pActor) {
        Effect::requestMovementOn(::getEffectKeeper(pActor));
    }

    void setAllEffectDrawOrder(LiveActor* pActor, s32 order) {
        EffectKeeper* keeper = ::getEffectKeeper(pActor);
        if (keeper) {
            s32 count = keeper->_C.size();
            for (s32 i = 0; i < count; i++) {
                keeper->getEmitter(i)->setDrawOrder(order);
            }
        }
    }

    void onEmitEffectSyncClipping(LiveActor* pActor, const char* pEffectName) {
        getEffect(pActor, pEffectName)->onCreateSyncClipping();
    }

    void onForceDeleteEffectSyncClipping(LiveActor* pActor, const char* pEffectName) {
        getEffect(pActor, pEffectName)->onForceDeleteSyncClipping();
    }

    void setEffectName(LiveActor* pActor, const char* pOldName, const char* pNewName) {
        ::getEffectKeeper(pActor)->changeEffectName(pOldName, pNewName);
    }

    void setEffectHostSRT(LiveActor* pActor, const char* pEffectName, const TVec3f* pScale, const TVec3f* pRotate, const TVec3f* pTranslate) {
        if (!::getEffectKeeper(pActor)->isTypeAttributeEffect(pEffectName)) {
            getEffect(pActor, pEffectName)->setHostSRT(pScale, pRotate, pTranslate);
        } else {
            for (u32 i = 0; i < 7; i++) {
                MultiEmitter* emitter = static_cast<const EffectKeeper*>(::getEffectKeeper(pActor))->getEmitter(pEffectName, sAttributeEffectCodes[i]);
                if (emitter) emitter->setHostSRT(pScale, pRotate, pTranslate);
            }
        }
    }

    void setEffectHostMtx(LiveActor* pActor, const char* pEffectName, MtxPtr pMtx) {
        if (!::getEffectKeeper(pActor)->isTypeAttributeEffect(pEffectName)) {
            getEffect(pActor, pEffectName)->setHostMtx(pMtx);
        } else {
            for (u32 i = 0; i < 7; i++) {
                MultiEmitter* emitter = static_cast<const EffectKeeper*>(::getEffectKeeper(pActor))->getEmitter(pEffectName, sAttributeEffectCodes[i]);
                if (emitter) emitter->setHostMtx(pMtx);
            }
        }
    }

    void setAllEffectHostMtx(LiveActor* pActor, MtxPtr pMtx) {
        EffectKeeper* keeper = ::getEffectKeeper(pActor);
        if (keeper) {
            s32 count = keeper->_C.size();
            for (s32 i = 0; i < count; i++) {
                MultiEmitter* emitter = keeper->getEmitter(i);
                if (!emitter->mAutoEffectInfo->mJointName) emitter->setHostMtx(pMtx);
            }
        }
    }

    void setEffectBaseScale(LiveActor* pActor, const char* pEffectName, f32 scale) {
        getEffect(pActor, pEffectName)->setBaseScale(scale);
    }

    void setEffectAlpha(LiveActor* pActor, const char* pEffectName, u8 alpha) {
        getEffect(pActor, pEffectName)->setGlobalAlpha(alpha, -1);
    }

    void setEffectColor(LiveActor* pActor, const char* pEffectName, u8 colorPrm1, u8 colorPrm2, u8 colorPrm3, u8 colorEnv1, u8 colorEnv2,
                        u8 colorEnv3) {
        getEffect(pActor, pEffectName)->setGlobalPrmColor(colorPrm1, colorPrm2, colorPrm3, -1);
        getEffect(pActor, pEffectName)->setGlobalEnvColor(colorEnv1, colorEnv2, colorEnv3, -1);
    }

    void setEffectPrmColor(LiveActor* pActor, const char* pEffectName, u8 color1, u8 color2, u8 color3) {
        getEffect(pActor, pEffectName)->setGlobalPrmColor(color1, color2, color3, -1);
    }

    void setEffectEnvColor(LiveActor* pActor, const char* pEffectName, u8 color1, u8 color2, u8 color3) {
        getEffect(pActor, pEffectName)->setGlobalEnvColor(color1, color2, color3, -1);
    }

    void emitEffectHit(LiveActor* pActor, const TVec3f& rPosition, const char* pEffectName) {
        if (pEffectName == nullptr) {
            pEffectName = "HitMarkNormal";
        }

        emitEffect(pActor, pEffectName)->setGlobalTranslation(rPosition, -1);
    }

    void emitEffectHit(LiveActor* pActor, const TVec3f& rPosition, const TVec3f& rDirection, const char* pEffectName) {
        if (pEffectName == nullptr) {
            pEffectName = "HitMarkNormal";
        }

        TPos3f mtx;
        makeMtxUpNoSupportPos(&mtx, rDirection, rPosition);
        emitEffect(pActor, pEffectName)->setGlobalSRTMatrix(mtx, -1);
    }

    void emitEffectHit(LiveActor* pActor, MtxPtr pMtx, const char* pEffectName) {
        if (pEffectName == nullptr) {
            pEffectName = "HitMarkNormal";
        }

        emitEffect(pActor, pEffectName)->setGlobalSRTMatrix(pMtx, -1);
    }

    void emitEffectHitBetweenSensors(LiveActor* pActor, const HitSensor* pSensor1, const HitSensor* pSensor2, f32 flt, const char* pEffectName) {
        TVec3f vecBetween;
        calcPosBetweenSensors(&vecBetween, pSensor1, pSensor2, flt);
        emitEffectHit(pActor, vecBetween, pEffectName);
    }

    // NONMATCHING: register/stack scheduling or equivalent branch lowering; calls, values and fields audited (FP order/grouping checked: no FP instructions).
    void initEffectAfterPlacement(LiveActor* pActor, MtxPtr pMtx) {
        if (!pMtx) pMtx = pActor->getBaseMtx();
        ::getEffectKeeper(pActor)->initAfterPlacementForAttributeEffect(pMtx);
    }

    void updateEffectFloorCode(LiveActor* pActor, const Triangle* pTriangle) {
        ::getEffectKeeper(pActor)->updateFloorCode(pTriangle);
    }

    void updateEffectFloorCodeLineToMap(LiveActor* pActor, const TVec3f& rPosition, const TVec3f& rDirection) {
        Triangle triangle = Triangle();

        if (getFirstPolyOnLineToMap(nullptr, &triangle, rPosition, rDirection)) {
            ::getEffectKeeper(pActor)->updateFloorCode(&triangle);
        } else {
            ::getEffectKeeper(pActor)->updateFloorCode(nullptr);
        }
    }

    // NONMATCHING: register/stack or instruction/branch lowering (FP operand order/grouping checked).
    void updateEffectFloorCodeLineToMap(LiveActor* pActor, f32 a2, f32 a3) {
        TVec3f gravity = pActor->mGravity;
        TVec3f sumPosScaledGrav = pActor->mPosition + gravity * a3;

        updateEffectFloorCodeLineToMap(pActor, sumPosScaledGrav, gravity * (a2 - a3));
    }


    void addEffectHitNormal(LiveActor* pActor, const char* pEffectName) {
        const char* const pHitEffectName = "HitMarkNormal";
        addEffect(pActor, pHitEffectName);

        if (pEffectName != nullptr) {
            setEffectName(pActor, pHitEffectName, pEffectName);
        }
    }
};  // namespace MR
