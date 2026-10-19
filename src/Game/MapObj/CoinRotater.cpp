#include "Game/MapObj/CoinRotater.hpp"
#include "Game/Util.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/ObjUtil.hpp"

CoinRotater::CoinRotater(const char* pName) : NameObj(pName), mAngle(), mWaterAngle(), mHiSpeedAngle() {
    mRotateYMtx.identity();
    mHiSpeedRotateYMtx.identity();
    mWaterRotateMtx.identity();
    
    MR::connectToSceneMapObjMovement(this);
}

void CoinRotater::movement() {
    mAngle += 8.0f;
    mAngle = MR::repeat(mAngle, 0.0f, 360.0f);

    mWaterAngle += 4.0f;
    mWaterAngle = MR::repeat(mWaterAngle, 0.0f, 360.0f);

    mHiSpeedAngle += 16.0f;
    mHiSpeedAngle = MR::repeat(mHiSpeedAngle, 0.0f, 360.0f);

    MR::makeMtxRotateY(mRotateYMtx, mAngle);
    MR::makeMtxRotateY(mHiSpeedRotateYMtx, mHiSpeedAngle);
    MR::makeMtxRotateY(mWaterRotateMtx, mWaterAngle);
}

namespace MR {
    void createCoinRotater() {
        MR::createSceneObj(SceneObj_CoinRotater);
    }

    TMtx34f& getCoinRotateYMatrix() {
        return MR::getSceneObj< CoinRotater >(SceneObj_CoinRotater)->mRotateYMtx;
    }

    TMtx34f& getCoinHiSpeedRotateYMatrix() {
        return MR::getSceneObj< CoinRotater >(SceneObj_CoinRotater)->mHiSpeedRotateYMtx;
    }

    TMtx34f& getCoinInWaterRotateYMatrix() {
        return MR::getSceneObj< CoinRotater >(SceneObj_CoinRotater)->mWaterRotateMtx;
    }
};  // namespace MR
