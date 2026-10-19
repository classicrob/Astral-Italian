#include "Game/Player/MarioEffect.hpp"
#include "Game/Util.hpp"
#include "Game/Effect/MultiEmitter.hpp"
#include "Game/Effect/ParticleEmitter.hpp"
#include "Game/LiveActor/EffectKeeper.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Player/Mario.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/MarioSwim.hpp"
#include "Game/Util/AreaObjUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/HashUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

// SB4E01 calls the unnamed MarioModule::getShadowPos (no symbol name in symbols.txt)
#include <JSystem/JParticle/JPAEmitter.hpp>
#include <JSystem/JParticle/JPAMath.hpp>
#include <cstdio>
#include <cstring>

void MarioEffect::execute(JPABaseEmitter* pEmitter) {
    s32 idx = 0;
    for (s32 i = 0; i < 256; i++) {
        if (_54[i]._68 == pEmitter) {
            break;
        }

        idx++;
    }

    updateFollowMtx(&_54[idx]);
}

JPABaseEmitter* MarioEffect::addRequest(const char* pEffectName, MtxPtr pMtx) {
    u32 requestIndex = 0;
    for (; requestIndex < 256; requestIndex++) {
        if (_54[requestIndex]._68 == nullptr) {
            break;
        }
    }

    if (requestIndex == 256) {
        requestIndex = 0;
        for (s32 i = 1; i < 256; i++) {
            if (_54[i]._0 > _54[requestIndex]._0) {
                requestIndex = i;
            }
        }
    }

    MovingFollowMtx* target = &_54[requestIndex];

    MultiEmitter* emitter = MR::emitEffectWithEmitterCallBack(mActor, pEffectName, this);
    if (emitter->getParticleEmitter(0) == nullptr) {
        return nullptr;
    }

    JPABaseEmitter* baseEmitter = emitter->getParticleEmitter(0)->mEmitter;
    target->_68 = baseEmitter;

    for (s32 i = 0; i < 256; i++) {
        if (requestIndex != i && _54[i]._68 == target->_68) {
            _54[i]._68 = nullptr;
        }
    }

    target->_0 = 0;
    target->_64 = pMtx;
    return target->_68;
}

void MarioEffect::updateFollowMtx(MovingFollowMtx* pFollow) {
    if (pFollow->_68->isEnableDeleteEmitter()) {
        pFollow->_68 = nullptr;
        return;
    }

    if (pFollow->_0 == 0) {
        Mtx temp;
        PSMTXCopy(_24, temp);

        const TVec3f& rTrans = getTrans();
        MR::setMtxTrans(temp, rTrans.x, rTrans.y, rTrans.z);

        if (pFollow->_64 == nullptr) {
            pFollow->_64 = reinterpret_cast< MtxPtr >(getPlayer()->getMoveBaseMtx());
            if (pFollow->_64 == nullptr) {
                return;
            }
        }

        Mtx inv;
        PSMTXInverse(pFollow->_64, inv);
        MR::multMtx(pFollow->_4, temp, inv);
        PSMTXCopy(temp, pFollow->_34);
    } else {
        MR::multMtx(pFollow->_34, pFollow->_4, pFollow->_64);
    }

    pFollow->_68->setGlobalRTMatrix(pFollow->_34);

    pFollow->_0++;
    if (pFollow->_0 == 2) {
        pFollow->_68->playCalcEmitter();
        pFollow->_68->becomeInvalidEmitter();
    }
}

MarioEffect::MarioEffect(MarioActor* pActor) : MarioModule(pActor) {
    _C = -1;
    _10 = -1;
    _14 = -1;
    _18 = 0;
    _1C.set(0, 0, 0, 0);
    _20.set(0, 0, 0, 0);
    PSMTXIdentity(_24.toMtxPtr());
}

void MarioEffect::playSwingEffect() {
    if (_C == 1) {
        switch (_10) {
        default:
            break;
        case 0:
            playEffectTrans("FlowerSpin", this->fn_803EF5B0());
            break;
        }
    }

    if (!((getPlayer()->mMovementStates.mLowWord >> 30) & 1U)) {
        if (!(getPlayer()->mVerticalSpeed < 50.0f)) {
            return;
        }
    }

    Mario* mario = getPlayer();
    playEffectRT("属性スピン", mario->_368, getTrans());
}

void MarioEffect::doCubeEffect() {
    Mario* mario = getPlayer();
    if (((mario->mMovementStates.mLowWord & Mario::MovementStates::DebugMode) >> 9)) {
        return;
    }

    s32 unsetArg = -1;
    _10 = unsetArg;
    _C = unsetArg;

    AreaObj* area = MR::getAreaObj("Effect", mActor->mPosition);
    if (area != nullptr) {
        s32 arg0 = MR::getAreaObjArg(area, 0);
        _10 = MR::getAreaObjArg(area, 1);
        _C = arg0;
    } else {
        _14 = unsetArg;
    }

    AreaObj* colorArea = MR::getAreaObj("SmokeEffectColorArea", mActor->mPosition);
    if (colorArea != nullptr) {
        const s32 prmR = MR::getAreaObjArg(colorArea, 0);
        const s32 prmG = MR::getAreaObjArg(colorArea, 1);
        const s32 prmB = MR::getAreaObjArg(colorArea, 2);
        _1C.set(prmR, prmG, prmB, 0);

        const s32 envR = MR::getAreaObjArg(colorArea, 3);
        const s32 envG = MR::getAreaObjArg(colorArea, 4);
        const s32 envB = MR::getAreaObjArg(colorArea, 5);
        _20.set(envR, envG, envB, 0);
        _18 = 1;
    } else {
        _18 = 0;
    }

    if (((getPlayer()->mFlags14.mLowWord >> 30) & 1U)) {
        playSwingEffect();
        getPlayer()->mFlags14.mLowWord &= 0xBFFFFFFFU;
    }
}

MarioEffect::~MarioEffect() {
}
