#include "Game/LiveActor/PartsModel.hpp"
#include "Game/Util.hpp"
#include "Game/NameObj/NameObjExecuteHolder.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/FixedPosition.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/ActorInitUtil.hpp"

PartsModel::PartsModel(LiveActor* pActor, const char* pName, const char* pModelName, MtxPtr pMtx, int drawBufferType, bool a6)
    : LiveActor(pName), mHost(pActor), mFixedPos(), mMtx(pMtx), mCalcOwnMatrix(true), mCheckHiddenHost(), mIsDead() {
    if (pMtx != nullptr) {
        mPosition.set< f32 >(pMtx[0][3], pMtx[1][3], pMtx[2][3]);
    } else {
        mPosition.set(pActor->mPosition);
    }

    mRotation.set(pActor->mRotation);
    mScale.set(pActor->mScale);

    if (pModelName == nullptr) {
        return;
    }

    if (!MR::initActor(this, pModelName, a6)) {
        if (drawBufferType < 0) {
            drawBufferType = MR::DrawBufferType_MapObj;
        }

        u32 movementType = MR::MovementType_EnemyDecoration;
        u32 calcAnimType = MR::CalcAnimType_MapObjDecoration;

        if (drawBufferType == MR::DrawBufferType_PlayerDecoration || drawBufferType == MR::DrawBufferType_CrystalBox ||
            drawBufferType == MR::DrawBufferType_UNK_0x17) {
            movementType = MR::MovementType_PlayerDecoration;
            calcAnimType = MR::CalcAnimType_PlayerDecoration;
        }

        if (drawBufferType == MR::DrawBufferType_NPC || drawBufferType == MR::DrawBufferType_IndirectNpc) {
            movementType = MR::MovementType_NPC;
            calcAnimType = MR::CalcAnimType_NPC;
        }

        MR::connectToScene(this, movementType, calcAnimType, drawBufferType, MR::DrawType_None);
    }
}

void PartsModel::makeActorAppeared() {
    LiveActor::makeActorAppeared();

    mIsDead = false;
}

void PartsModel::makeActorDead() {
    LiveActor::makeActorDead();

    mIsDead = true;
}

void PartsModel::init(const JMapInfoIter& rIter) {
    if (!mEffectKeeper) {
        initEffectKeeper(8, nullptr, false);
    }

    if (!mSoundHolder) {
        initSound(8, nullptr, nullptr, TVec3f(0.0f));
    }

    MR::invalidateClipping(this);
    makeActorAppeared();
}

void PartsModel::movement() {
    if (MR::isDead(this) || MR::isDead(mHost) || MR::isClipped(mHost) || !mCheckHiddenHost && MR::isHiddenModel(mHost)) {
        if (!mIsDead) {
            mIsDead = true;

            MR::invalidateHitSensors(this);

            if (!MR::isHiddenModel(this)) {
                MR::disconnectToDrawTemporarily(this);
            }
        }
    } else {
        if (mIsDead) {
            mIsDead = false;

            MR::validateHitSensors(this);

            if (!MR::isHiddenModel(this)) {
                MR::connectToDrawTemporarily(this);
            }
        }

        LiveActor::movement();
    }
}

inline bool hasFixedPosition(const PartsModel* model) { return model->mFixedPos != nullptr; }

void PartsModel::calcAnim() {
    if (mIsDead || MR::isDead(this) || MR::isNoCalcAnim(this) || MR::isClipped(this)) {
        return;
    }

    if (hasFixedPosition(this)) {
        mFixedPos->calc();
    }

    LiveActor::calcAnim();
}

void PartsModel::calcViewAndEntry() {
    if (mIsDead) {
        return;
    }

    LiveActor::calcViewAndEntry();
}

void PartsModel::initFixedPosition(const TVec3f& rLocalTrans, const TVec3f& rLocalRotate, const char* pJointName) {
    if (pJointName != nullptr) {
        mFixedPos = new FixedPosition(mHost, pJointName, rLocalTrans, rLocalRotate);
    } else {
        mFixedPos = new FixedPosition(mHost, rLocalTrans, rLocalRotate);
    }

    mMtx = mFixedPos->mMtx;
}

void PartsModel::initFixedPosition(const char* pJointName) {
    initFixedPosition(TVec3f(0.0f, 0.0f, 0.0f), TVec3f(0.0f, 0.0f, 0.0f), pJointName);
}

void PartsModel::initFixedPosition(MtxPtr pMtx, const TVec3f& rLocalTrans, const TVec3f& rLocalRotate) {
    mFixedPos = new FixedPosition(pMtx, rLocalTrans, rLocalRotate);
    mMtx = mFixedPos->mMtx;
}

void PartsModel::loadFixedPosition(const char* pJointName, const LiveActor* pActor) {
    mFixedPos = new FixedPosition(mHost, pJointName, pActor);
    mMtx = mFixedPos->mMtx;
}

void PartsModel::offFixedPosNormalizeScale() {
    mFixedPos->mNormalizeScale = false;
}

void PartsModel::calcAndSetBaseMtx() {
    if (mMtx != nullptr && mCalcOwnMatrix) {
        mPosition.set< f32 >(mMtx[0][3], mMtx[1][3], mMtx[2][3]);
        MR::setBaseTRMtx(this, mMtx);
    } else {
        LiveActor::calcAndSetBaseMtx();
    }
}
