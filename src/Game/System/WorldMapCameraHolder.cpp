#include "Game/System/GalaxyWorldMapInfo.hpp"
#include "Game/System/MiniRouteLine.hpp"
#include "Game/System/WorldMapCameraHolder.hpp"
#include "Game/System/WorldMapPointParts.hpp"
#include "Game/System/WorldMapPointMarks.hpp"
#include "Game/System/MiniPartsActors.hpp"
#include "Game/System/MiniPlayerRocket.hpp"
#include "Game/System/WorldMapFunction.hpp"
#include "Game/System/MiniStarCheckPoint.hpp"
#include "Game/System/MiniTicoMaster.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/MaterialCtrl.hpp"
#include "Game/Util.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/VectorUtil.hpp"
#include <cmath>

// ==== TU B (0x804EB2F0): WorldMapCameraHolder ====
// ---------------------------------------------------------------------------------------------------------------
// WorldMapCameraHolder (0x68 bytes): rocket, camera target and camera info of the world map
// ---------------------------------------------------------------------------------------------------------------


WorldMapCameraHolder::WorldMapCameraHolder() {
    mCurrent = 0;
    mNext = -1;
    mRocket = nullptr;
    mCameraTarget = nullptr;
    mCameraInfo = nullptr;
    mCameraId = -1;
    mMovingLeft = false;
    mHasNext = false;
    mPrevious = -1;
}

void WorldMapCameraHolder::init(s32 current) {
    mCurrent = current;
    mRocket = new MiniPlayerRocket();
    mRocket->initWithoutIter();
    mCameraTarget = new LiveActor("カメラターゲット");
    mCameraTarget->initWithoutIter();
    mCameraTarget->makeActorAppeared();
    MR::invalidateClipping(mCameraTarget);
    mCameraId = MR::getCameraParamId("WorldMapCamera", "CameraParam");
    mCameraInfo = new ActorCameraInfo(0, 0);
}

void WorldMapCameraHolder::setNext(s32 next) {
    mHasNext = true;
    mPrevious = mNext;
    mNext = next;
}

bool WorldMapCameraHolder::tryApplyNext() {
    s32 previous = mPrevious;
    mHasNext = false;
    if (previous == mCurrent) {
        mPrevious = -1;
        return false;
    }
    mNext = previous;
    mPrevious = -1;
    return true;
}
