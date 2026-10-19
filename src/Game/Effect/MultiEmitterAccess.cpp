#define JGEOMETRY_VEC3_SET_NOINLINE
#define JGEOMETRY_VEC2_SET2_NOINLINE
#include "Game/Effect/MultiEmitter.hpp"
#include "Game/Effect/MultiEmitterCallBack.hpp"
#include "Game/Effect/SingleEmitter.hpp"
#include "Game/Effect/ParticleEmitter.hpp"
#include "Game/Util/MathUtil.hpp"
#include <JSystem/JParticle/JPAEmitter.hpp>

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

void ParticleEmitter::setGlobalRotation(const TVec3s& rotation) {
    mEmitter->setGlobalRotation(rotation);
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

void ParticleEmitter::setGlobalScale(const TVec3f& scale) {
    mEmitter->setGlobalScale(scale);
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

void ParticleEmitter::setGlobalSRTMatrix(const Mtx matrix) {
    mEmitter->setGlobalSRTMatrix(matrix);
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

void ParticleEmitter::setGlobalPrmColor(u8 r, u8 g, u8 b) {
    mEmitter->mGlobalPrmClr.r = r;
    mEmitter->mGlobalPrmClr.g = g;
    mEmitter->mGlobalPrmClr.b = b;
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

void ParticleEmitter::setGlobalEnvColor(u8 r, u8 g, u8 b) {
    mEmitter->mGlobalEnvClr.r = r;
    mEmitter->mGlobalEnvClr.g = g;
    mEmitter->mGlobalEnvClr.b = b;
}


void MultiEmitter::setGlobalAlpha(u8 alpha, s32 idx) {
    if (idx == -1) {
        for (s32 i = 0; i < mEmitters.size(); i++) {
            SingleEmitter* pEmitter = getValidEmitter(i, false);
            if (pEmitter != nullptr) {
                pEmitter->mEmitter->mEmitter->mGlobalPrmClr.a = alpha;
            }
        }

        return;
    }

    SingleEmitter* pEmitter = getValidEmitter(idx, false);
    if (pEmitter != nullptr) {
        pEmitter->mEmitter->mEmitter->mGlobalPrmClr.a = alpha;
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
