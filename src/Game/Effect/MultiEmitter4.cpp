#include "Game/LiveActor/EffectKeeper.hpp"
#include "Game/Effect/AutoEffectInfo.hpp"
#include "Game/Effect/EffectSystemUtil.hpp"
#include "Game/Effect/MultiEmitter.hpp"
#include "Game/Effect/SyncBckEffectChecker.hpp"
#include "Game/LiveActor/Binder.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/System/ResourceHolder.hpp"
#include "Game/Util/HashUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <algorithm>
#include <cstdio>
#include "Game/Effect/MultiEmitterCallBack.hpp"
#include "Game/Effect/SingleEmitter.hpp"
#include <JSystem/JParticle/JPAEmitter.hpp>
#include "Game/Effect/EffectSystem.hpp"
#include "Game/Effect/MultiEmitterParticleCallBack.hpp"
#include "Game/Effect/SyncBckEffectInfo.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include <cstring>
#include "Game/Screen/PaneEffectKeeper.hpp"
#include "Game/Effect/ParticleResourceHolder.hpp"
#include "Game/Screen/LayoutActor.hpp"
#include "Game/Screen/LayoutManager.hpp"
#include "Game/Util/SystemUtil.hpp"
#include "Game/Scene/MultiSceneEffectKeeper.hpp"
#include "Game/LiveActor/ModelManager.hpp"
#include <JSystem/JGeometry/TVec.hpp>
#include "Game/Player/MarioEffect.hpp"
#include "Game/Effect/ParticleEmitter.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/Player/Mario.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/MarioSwim.hpp"
#include "Game/Util/AreaObjUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include <JSystem/JParticle/JPAMath.hpp>

namespace {
    inline s16 toS16(f32 deg) {
        return DEGREE_TO_S16 * deg;
    }
};  // namespace




void MultiEmitter::setGlobalRotation(const TVec3s& rRotation, s32 idx) {
    if (idx == -1) {
        for (s32 i = 0; i < mEmitters.size(); i++) {
            SingleEmitter* pEmitter = getValidEmitter(i, false);
            if (pEmitter != nullptr) {
                pEmitter->mEmitter->setGlobalRotation(rRotation);
            }
        }

        return;
    }

    SingleEmitter* pEmitter = getValidEmitter(idx, false);
    if (pEmitter != nullptr) {
        pEmitter->mEmitter->setGlobalRotation(rRotation);
    }
}

void MultiEmitter::setGlobalTranslation(const TVec3f& rTranslation, s32 idx) {
    if (idx == -1) {
        for (s32 i = 0; i < mEmitters.size(); i++) {
            SingleEmitter* pEmitter = getValidEmitter(i, false);
            if (pEmitter != nullptr) {
                pEmitter->mEmitter->mEmitter->mGlobalTrs.set(rTranslation);
            }
        }

        return;
    }

    SingleEmitter* pEmitter = getValidEmitter(idx, false);
    if (pEmitter != nullptr) {
        pEmitter->mEmitter->mEmitter->mGlobalTrs.set(rTranslation);
    }
}

void MultiEmitter::setGlobalScale(const TVec3f& rScale, s32 idx) {
    if (idx == -1) {
        for (s32 i = 0; i < mEmitters.size(); i++) {
            SingleEmitter* pEmitter = getValidEmitter(i, false);
            if (pEmitter != nullptr) {
                pEmitter->mEmitter->setGlobalScale(rScale);
            }
        }

        return;
    }

    SingleEmitter* pEmitter = getValidEmitter(idx, false);
    if (pEmitter != nullptr) {
        pEmitter->mEmitter->setGlobalScale(rScale);
    }
}

void MultiEmitter::setGlobalSRTMatrix(const Mtx pMtx, s32 idx) {
    if (idx == -1) {
        for (s32 i = 0; i < mEmitters.size(); i++) {
            SingleEmitter* pEmitter = getValidEmitter(i, false);
            if (pEmitter != nullptr) {
                pEmitter->mEmitter->setGlobalSRTMatrix(pMtx);
            }
        }

        return;
    }

    SingleEmitter* pEmitter = getValidEmitter(idx, false);
    if (pEmitter != nullptr) {
        pEmitter->mEmitter->setGlobalSRTMatrix(pMtx);
    }
}

void MultiEmitter::setGlobalPrmColor(u8 r, u8 g, u8 b, s32 idx) {
    bool b1 = !MR::isNearZero(_2C);

    if (idx == -1) {
        mCallBack->mPrmColor.set(r, g, b, -1);

        if (!b1) {
            for (s32 i = 0; i < mEmitters.size(); i++) {
                SingleEmitter* pEmitter = getValidEmitter(i, false);
                if (pEmitter != nullptr) {
                    pEmitter->mEmitter->setGlobalPrmColor(r, g, b);
                }
            }
        }

        return;
    }

    SingleEmitter* pEmitter = getValidEmitter(idx, false);
    if (pEmitter != nullptr) {
        pEmitter->mEmitter->setGlobalPrmColor(r, g, b);
    }
}

void MultiEmitter::setGlobalEnvColor(u8 r, u8 g, u8 b, s32 idx) {
    bool b1 = !MR::isNearZero(_2C);

    if (idx == -1) {
        mCallBack->mEnvColor.set(r, g, b, -1);

        if (!b1) {
            for (s32 i = 0; i < mEmitters.size(); i++) {
                SingleEmitter* pEmitter = getValidEmitter(i, false);
                if (pEmitter != nullptr) {
                    pEmitter->mEmitter->setGlobalEnvColor(r, g, b);
                }
            }
        }

        return;
    }

    SingleEmitter* pEmitter = getValidEmitter(idx, false);
    if (pEmitter != nullptr) {
        pEmitter->mEmitter->setGlobalEnvColor(r, g, b);
    }
}

void MultiEmitter::setLocalScale(const TVec3f& rScale, s32 idx) {
    if (idx == -1) {
        for (s32 i = 0; i < mEmitters.size(); i++) {
            SingleEmitter* pEmitter = getValidEmitter(i, false);
            if (pEmitter != nullptr) {
                pEmitter->mEmitter->mEmitter->mLocalScl.set(rScale);
            }
        }

        return;
    }

    SingleEmitter* pEmitter = getValidEmitter(idx, false);
    if (pEmitter != nullptr) {
        pEmitter->mEmitter->mEmitter->mLocalScl.set(rScale);
    }
}

void MultiEmitter::setDirectionalSpeed(f32 speed, s32 idx) {
    if (idx == -1) {
        for (s32 i = 0; i < mEmitters.size(); i++) {
            SingleEmitter* pEmitter = getValidEmitter(i, false);
            if (pEmitter != nullptr) {
                pEmitter->mEmitter->mEmitter->setDirectionalSpeed(speed);
            }
        }

        return;
    }

    SingleEmitter* pEmitter = getValidEmitter(idx, false);
    if (pEmitter != nullptr) {
        pEmitter->mEmitter->mEmitter->setDirectionalSpeed(speed);
    }
}

void MultiEmitter::setRate(f32 rate, s32 idx) {
    if (idx == -1) {
        for (s32 i = 0; i < mEmitters.size(); i++) {
            SingleEmitter* pEmitter = getValidEmitter(i, false);
            if (pEmitter != nullptr) {
                pEmitter->mEmitter->mEmitter->setRate(rate);
            }
        }

        return;
    }

    SingleEmitter* pEmitter = getValidEmitter(idx, false);
    if (pEmitter != nullptr) {
        pEmitter->mEmitter->mEmitter->setRate(rate);
    }
}

void MultiEmitter::stopCalcEmitter(s32 idx) {
    if (idx == -1) {
        for (s32 i = 0; i < mEmitters.size(); i++) {
            SingleEmitter* pEmitter = getValidEmitter(i, false);
            if (pEmitter != nullptr) {
                pEmitter->mEmitter->mEmitter->stopCalcEmitter();
            }
        }

        return;
    }

    SingleEmitter* pEmitter = getValidEmitter(idx, false);
    if (pEmitter != nullptr) {
        pEmitter->mEmitter->mEmitter->stopCalcEmitter();
    }
}

void MultiEmitter::playCalcEmitter(s32 idx) {
    if (idx == -1) {
        for (s32 i = 0; i < mEmitters.size(); i++) {
            SingleEmitter* pEmitter = getValidEmitter(i, false);
            if (pEmitter != nullptr) {
                pEmitter->mEmitter->mEmitter->playCalcEmitter();
            }
        }

        return;
    }

    SingleEmitter* pEmitter = getValidEmitter(idx, false);
    if (pEmitter != nullptr) {
        pEmitter->mEmitter->mEmitter->playCalcEmitter();
    }
}

void MultiEmitter::stopDrawParticle(s32 idx) {
    if (idx == -1) {
        for (s32 i = 0; i < mEmitters.size(); i++) {
            SingleEmitter* pEmitter = getValidEmitter(i, false);
            if (pEmitter != nullptr) {
                pEmitter->mEmitter->mEmitter->stopDrawParticle();
            }
        }

        return;
    }

    SingleEmitter* pEmitter = getValidEmitter(idx, false);
    if (pEmitter != nullptr) {
        pEmitter->mEmitter->mEmitter->stopDrawParticle();
    }
}

void MultiEmitter::playDrawParticle(s32 idx) {
    if (idx == -1) {
        for (s32 i = 0; i < mEmitters.size(); i++) {
            SingleEmitter* pEmitter = getValidEmitter(i, false);
            if (pEmitter != nullptr) {
                pEmitter->mEmitter->mEmitter->playDrawParticle();
            }
        }

        return;
    }

    SingleEmitter* pEmitter = getValidEmitter(idx, false);
    if (pEmitter != nullptr) {
        pEmitter->mEmitter->mEmitter->playDrawParticle();
    }
}

void MultiEmitter::setParticleCallBackPtr(JPAParticleCallBack* pCallBack, s32 idx) {
    if (idx == -1) {
        for (s32 i = 0; i < mEmitters.size(); i++) {
            SingleEmitter* pEmitter = getValidEmitter(i, false);
            if (pEmitter != nullptr) {
                pEmitter->mEmitter->mEmitter->setParticleCallBackPtr(pCallBack);
            }
        }

        return;
    }

    SingleEmitter* pEmitter = getValidEmitter(idx, false);
    if (pEmitter != nullptr) {
        pEmitter->mEmitter->mEmitter->setParticleCallBackPtr(pCallBack);
    }
}

void MultiEmitter::pauseOn(s32 idx) {
    if (idx == -1) {
        for (s32 i = 0; i < mEmitters.size(); i++) {
            SingleEmitter* pEmitter = getValidEmitter(i, false);
            if (pEmitter != nullptr) {
                pEmitter->mEmitter->pauseOn();
            }
        }

        return;
    }

    SingleEmitter* pEmitter = getValidEmitter(idx, false);
    if (pEmitter != nullptr) {
        pEmitter->mEmitter->pauseOn();
    }
}

void MultiEmitter::pauseOff(s32 idx) {
    if (idx == -1) {
        for (s32 i = 0; i < mEmitters.size(); i++) {
            SingleEmitter* pEmitter = getValidEmitter(i, false);
            if (pEmitter != nullptr) {
                pEmitter->mEmitter->pauseOff();
            }
        }

        return;
    }

    SingleEmitter* pEmitter = getValidEmitter(idx, false);
    if (pEmitter != nullptr) {
        pEmitter->mEmitter->pauseOff();
    }
}

MultiEmitter::MultiEmitter(const char* pName, const TVec3f* pScale, const TVec3f* pRotation, const TVec3f* pTranslation, const TVec3f& rVec)
    : mEmitters(), mCallBack(), mParticleCallBack(), _24(), _28(), _2C(), _30(), mHash(), mFlags() {
    mCallBack = new MultiEmitterCallBack(this, pScale, pRotation, pTranslation, rVec);
    mParticleCallBack = new MultiEmitterParticleCallBack();

    allocateEmitter(pName);
}

MultiEmitter::MultiEmitter(const char* pName, MtxPtr pMtx, const TVec3f& rVec)
    : mEmitters(), mCallBack(), mParticleCallBack(), _24(), _28(), _2C(), _30(), mHash(), mFlags() {
    mCallBack = new MultiEmitterCallBack(this, pMtx, rVec);
    mParticleCallBack = new MultiEmitterParticleCallBack();

    allocateEmitter(pName);
}

MultiEmitter::MultiEmitter(const char* pName, MtxPtr pMtx, const TVec3f* pTranslation, const TVec3f& rVec)
    : mEmitters(), mCallBack(), mParticleCallBack(), _24(), _28(), _2C(), _30(), mHash(), mFlags() {
    mCallBack = new MultiEmitterCallBack(this, pMtx, pTranslation, rVec);
    mParticleCallBack = new MultiEmitterParticleCallBack();

    allocateEmitter(pName);
}

MultiEmitter::MultiEmitter(const char* pName) : mEmitters(), mCallBack(), mParticleCallBack(), _24(), _28(), _2C(), _30(), mHash(), mFlags() {
    mCallBack = new MultiEmitterCallBack(this, TVec3f(0.0f, 0.0f, 0.0f));
    mParticleCallBack = new MultiEmitterParticleCallBack();

    allocateEmitter(pName);
}

void MultiEmitter::createEmitter() {
    create(MR::getEffectSystem());
}

void MultiEmitter::createEmitterWithCallBack(MultiEmitterCallBackBase* pCallBackBase) {
    for (SingleEmitter* pEmitter = mEmitters.begin(); pEmitter != mEmitters.end(); pEmitter++) {
        MR::getEffectSystem()->createSingleEmitter(pEmitter, pCallBackBase, nullptr);
    }

    std::for_each(mChildren.begin(), mChildren.end(),
                  std::binder2nd< std::mem_fun1_t< void, MultiEmitter, MultiEmitterCallBackBase* >, MultiEmitterCallBackBase* >(
                      std::mem_func(&MultiEmitter::createEmitterWithCallBack), pCallBackBase));
}

void MultiEmitter::deleteEmitter() {
    std::for_each_array(mEmitters.begin(), mEmitters.end(), std::mem_func(&SingleEmitter::deleteEmitter));
    std::for_each(mChildren.begin(), mChildren.end(), std::mem_func(&MultiEmitter::deleteEmitter));
}

void MultiEmitter::forceDeleteEmitter() {
    for (SingleEmitter* pEmitter = mEmitters.begin(); pEmitter != mEmitters.end(); pEmitter++) {
        MR::getEffectSystem()->forceDeleteSingleEmitter(pEmitter);
    }

    std::for_each(mChildren.begin(), mChildren.end(), std::mem_func(&MultiEmitter::forceDeleteEmitter));
}

void MultiEmitter::forceDelete(EffectSystem* pSystem) {
    for (SingleEmitter* pEmitter = mEmitters.begin(); pEmitter != mEmitters.end(); pEmitter++) {
        pSystem->forceDeleteSingleEmitter(pEmitter);
    }

    std::for_each(
        mChildren.begin(), mChildren.end(),
        std::binder2nd< std::mem_fun1_t< void, MultiEmitter, EffectSystem* >, EffectSystem* >(std::mem_func(&MultiEmitter::forceDelete), pSystem));
}

void MultiEmitter::deleteForeverEmitter() {
    for (SingleEmitter* pEmitter = mEmitters.begin(); pEmitter != mEmitters.end(); pEmitter++) {
        if (pEmitter->isValid() && !pEmitter->isOneTime()) {
            pEmitter->deleteEmitter();
        }
    }

    std::for_each(mChildren.begin(), mChildren.end(), std::mem_func(&MultiEmitter::deleteForeverEmitter));
}

void MultiEmitter::playCalcAndDeleteForeverEmitter() {
    playCalcEmitter(-1);
    deleteForeverEmitter();
}

bool MultiEmitter::isValid() const {
    for (const SingleEmitter* pEmitter = mEmitters.begin(); pEmitter != mEmitters.end(); pEmitter++) {
        if (pEmitter->isValid()) {
            return true;
        }
    }

    return false;
}

bool MultiEmitter::isExistOneTimeEmitter() const {
    for (const SingleEmitter* pEmitter = mEmitters.begin(); pEmitter != mEmitters.end(); pEmitter++) {
        if (pEmitter->isOneTime()) {
            return true;
        }
    }

    return false;
}

void MultiEmitter::setHostSRT(const TVec3f* pScale, const TVec3f* pRotation, const TVec3f* pTranslation) {
    mCallBack->setHostSRT(pScale, pRotation, pTranslation);
}

void MultiEmitter::setHostMtx(MtxPtr pMtx) {
    mCallBack->setHostMtx(pMtx);
}

void MultiEmitter::setOffset(const TVec3f& rOffset) {
    mCallBack->_18.set(rOffset);
}

void MultiEmitter::setBaseScale(f32 scale) {
    mCallBack->setBaseScale(scale);
}

void MultiEmitter::setName(const char* pName) {
    mHash = MR::getHashCode(pName);
}

bool MultiEmitter::isEqualName(u16 hash) const {
    return mHash == hash;
}

ParticleEmitter* MultiEmitter::getParticleEmitter(int idx) const {
    const SingleEmitter* pEmitter = &mEmitters[idx];
    if (pEmitter->isValid()) {
        return pEmitter->mEmitter;
    }

    return nullptr;
}

void MultiEmitter::create(EffectSystem* pSystem) {
    for (SingleEmitter* pEmitter = mEmitters.begin(); pEmitter != mEmitters.end(); pEmitter++) {
        pSystem->createSingleEmitter(pEmitter, mCallBack, mParticleCallBack);
    }

    mCallBack->resetFollowCurrent();

    for (MultiEmitter** pEmitter = mChildren.begin(); pEmitter != mChildren.end(); pEmitter++) {
        (*pEmitter)->create(pSystem);
    }
}

void MultiEmitter::scanParticleEmitter(EffectSystem* pSystem) {
    std::for_each_array(mEmitters.begin(), mEmitters.end(),
                        std::binder2nd< std::mem_fun1_t< void, SingleEmitter, EffectSystem* >, EffectSystem* >(
                            std::mem_func(&SingleEmitter::scanParticleEmitter), pSystem));
}

void MultiEmitter::forceFollowOn() {
    mCallBack->forceFollowOn();
}

void MultiEmitter::forceFollowOff() {
    mCallBack->forceFollowOff();
}

void MultiEmitter::forceScaleOn() {
    mCallBack->forceScaleOn();
}

void MultiEmitter::initSyncBck(XanimePlayer* pPlayer, const char* pName, s32 capacity, f32 startFrame) {
    _24 = new SyncBckEffectInfo(pPlayer, pName, capacity, 0.0f, -1.0f, false);
    _24->mStartFrame = startFrame;
}

void MultiEmitter::onDeleteSyncBck(bool deleteOnEnd, f32 endFrame) {
    _24->mEndFrame = endFrame;
}

void MultiEmitter::addSyncBck(const XanimePlayer* pPlayer, const char* pName) {
    _24->addBck(pPlayer, pName);
}

void MultiEmitter::setContinueBckEnd(bool continueAnimEnd) {
    _24->mContinueAnimEnd = continueAnimEnd;
}

void MultiEmitter::onCreateSyncClipping() {
    turnFlagOn(JPAEmtrStts_FirstEmit);
}

void MultiEmitter::onForceDeleteSyncClipping() {
    turnFlagOn(JPAEmtrStts_Immortal);
}

void MultiEmitter::stopEmitterOnClipped() {
    if (isFlagOn(JPAEmtrStts_Immortal)) {
        forceDeleteEmitter();
    } else if (isFlagOn(JPAEmtrStts_RateStepEmit)) {
        deleteEmitter();
    } else if (_24 != nullptr) {
        stopCalcEmitter(-1);
        stopDrawParticle(-1);
    }

    for (SingleEmitter* pEmitter = mEmitters.begin(); pEmitter != mEmitters.end(); pEmitter++) {
        if (pEmitter->isValid() && !pEmitter->isOneTime()) {
            pEmitter->mEmitter->mEmitter->setStatus(JPAEmtrStts_StopCalc);
            pEmitter->mEmitter->mEmitter->setStatus(JPAEmtrStts_StopDraw);
        }
    }
}

void MultiEmitter::playEmitterOffClipped() {
    if (isFlagOn(JPAEmtrStts_FirstEmit)) {
        create(MR::getEffectSystem());
    } else if (_24 != nullptr) {
        playCalcEmitter(-1);
        playDrawParticle(-1);
    }

    for (SingleEmitter* pEmitter = mEmitters.begin(); pEmitter != mEmitters.end(); pEmitter++) {
        if (pEmitter->isValid() && !pEmitter->isOneTime()) {
            pEmitter->mEmitter->mEmitter->clearStatus(JPAEmtrStts_StopCalc);
            pEmitter->mEmitter->mEmitter->clearStatus(JPAEmtrStts_StopDraw);
        }
    }
}

void MultiEmitter::setDrawOrder(s32 index) {
    std::for_each_array(
        mEmitters.begin(), mEmitters.end(),
        std::binder2nd< std::mem_fun1_t< void, SingleEmitter, u8 >, u8 >(std::mem_func(&SingleEmitter::setGroupID), static_cast< u8 >(index)));
}

void MultiEmitter::addChildEmitter(MultiEmitter* pChild) {
    mChildren.push_back(pChild);
}

  // namespace

void MultiEmitter::setGlobalRotationDegree(const TVec3f& rRotation, s32 idx) {
    setGlobalRotation(TVec3s(toS16(rRotation.x), toS16(rRotation.y), toS16(rRotation.z)), idx);
}

void MultiEmitter::allocateEmitter(const char* pName) {
    MR::Vector< MR::FixedArray< u16, 32 > > indices;
    bool numbered = !MR::hasStringSpace(pName) && !MR::isDigitStringTail(pName, 2);
    if (numbered) {
        u16 index = 0;
        while (MR::Effect::isExistInResource(&index, pName, indices.size())) {
            indices.push_back(index);
        }
    } else {
        u32 length = strlen(pName);
        s32 nameLength = 0;
        char name[40];
        MR::zeroMemory(name, sizeof(name));
        for (u32 i = 0; i <= length; i++) {
            if (pName[i] == ' ' || pName[i] == '\0') {
                name[nameLength] = '\0';
                u16 index;
                MR::Effect::isExistInResource(&index, name);
                indices.push_back(index);
                nameLength = 0;
                MR::zeroMemory(name, sizeof(name));
            } else {
                name[nameLength] = pName[i];
                nameLength++;
            }
        }
    }

    mEmitters.init(indices.size());
    for (s32 i = 0; i < indices.size(); i++) {
        mEmitters[i].init(indices[i]);
    }

    if (numbered) {
        setName(pName);
    }
}

SingleEmitter* MultiEmitter::getValidEmitter(s32 idx, bool) {
    SingleEmitter* pEmitter = &mEmitters[idx];
    if (pEmitter->isValid()) {
        return pEmitter;
    }

    return nullptr;
}

void MultiEmitter::createOneTimeEmitter() {
    for (SingleEmitter* pEmitter = mEmitters.begin(); pEmitter != mEmitters.end(); pEmitter++) {
        if (pEmitter->isOneTime()) {
            MR::getEffectSystem()->createSingleEmitter(pEmitter, mCallBack, nullptr);
        }
    }
}

void MultiEmitter::createForeverEmitter() {
    for (SingleEmitter* pEmitter = mEmitters.begin(); pEmitter != mEmitters.end(); pEmitter++) {
        if (!pEmitter->isOneTime()) {
            MR::getEffectSystem()->createSingleEmitter(pEmitter, mCallBack, nullptr);
        }
    }
}





void MultiEmitter::setGlobalScale(f32 scale, s32 a2) {
    TVec3f scaleVec(scale, scale, scale);
    setGlobalScale(scaleVec, a2);
}
