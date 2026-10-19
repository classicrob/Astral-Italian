#include "Game/NPC/MiiFaceParts.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/NPC/MiiFaceRecipe.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util/FixedPosition.hpp"
#include "Game/Util/MtxUtil.hpp"
#include <JSystem/J3DGraphBase/J3DSys.hpp>
#include <JSystem/JKernel/JKRHeap.hpp>
#include <RVLFaceLib.h>

MiiFaceParts::MiiFaceParts(const char* pName, const MiiFaceRecipe& rRecipe)
    : LiveActor(pName), mCharModel(nullptr), mModelBuffer(nullptr), mRecipe(nullptr), mFixedPosition(nullptr), mNeedsModelReinit(false) {
    mRecipe = new MiiFaceRecipe(rRecipe);

    MR::createSceneObj(SceneObj_MiiFacePartsHolder);
}

MiiFaceParts::~MiiFaceParts() {
}

void MiiFaceParts::init(const JMapInfoIter& rIter) {
    mModelBuffer = new (32) u8[mRecipe->getModelBufferSize()];
    mCharModel = new RFLCharModel;
    mIsModelUninitialized = true;
}

void MiiFaceParts::calcAnim() {
    if (mFixedPosition != nullptr) {
        mFixedPosition->calc();
        MR::setPosition(this, mFollowMtx[0][3], mFollowMtx[1][3], mFollowMtx[2][3]);
        PSMTXCopy(mFollowMtx, mModelMtx);
    } else {
        MR::makeMtxTR(mModelMtx, mPosition, mRotation);
    }
    MR::preScaleMtx(mModelMtx, mScale);
}

void MiiFaceParts::calcViewAndEntry() {
    Mtx viewMtx;

    if (mIsModelUninitialized) {
        return;
    }

    MR::multMtx(viewMtx, mModelMtx, j3dSys.mViewMtx);
    RFLSetMtx(mCharModel, viewMtx);
}

void MiiFaceParts::drawOpa(const RFLDrawCoreSetting* pSetting) const {
    if (mIsModelUninitialized) {
        return;
    }

    if (pSetting != nullptr) {
        RFLDrawOpaCore(mCharModel, pSetting);
    } else {
        RFLDrawOpa(mCharModel);
    }
}

void MiiFaceParts::drawXlu(const RFLDrawCoreSetting* pSetting) const {
    if (mIsModelUninitialized) {
        return;
    }

    if (pSetting != nullptr) {
        RFLDrawXluCore(mCharModel, pSetting);
    } else {
        RFLDrawXlu(mCharModel);
    }
}

void MiiFaceParts::initFixedPosition(MtxPtr pParam1, const TVec3f& rParam2, const TVec3f& rParam3) {
    mFixedPosition = new FixedPosition(pParam1, rParam2, rParam3);
    mFollowMtx = mFixedPosition->mMtx;
}

void MiiFaceParts::changeExpressionNormal() {
    tryChangeExpression(RFLExp_Normal);
}

void MiiFaceParts::changeExpressionBlink() {
    tryChangeExpression(RFLExp_Blink);
}

void MiiFaceParts::changeFaceModel(const MiiFaceRecipe& rRecipe) {
    mRecipe->mIndex = rRecipe.mIndex;
    mRecipe->mDataSource = rRecipe.mDataSource;

    if (rRecipe.mDataSource == RFLDataSource_Middle) {
        mRecipe->setDatabase(rRecipe.mMiddleDB);
    }

    mNeedsModelReinit = true;
}

void MiiFaceParts::initFaceModel() {
    mRecipe->initFaceModel(mCharModel, mModelBuffer);

    mNeedsModelReinit = false;
    mIsModelUninitialized = false;
}

bool MiiFaceParts::tryChangeExpression(u32 expression) {
    if (mIsModelUninitialized) {
        return false;
    }

    if (!mRecipe->hasExpression(RFLExpression(expression))) {
        return false;
    }

    RFLSetExpression(mCharModel, RFLExpression(expression));

    return true;
}
