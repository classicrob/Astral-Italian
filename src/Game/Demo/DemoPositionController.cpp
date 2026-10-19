#include "Game/Demo/DemoPositionController.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"

DemoPositionController::DemoPositionController(const char* pName, const JMapInfoIter& rIter) : LiveActor("デモアクター位置"), pCameraInfo() {
    MR::initDefaultPos(this, rIter);
    MR::initActor(this, rIter, pName, false);
    pCameraInfo = new ActorCameraInfo(rIter);
    MR::invalidateClipping(this);
    makeActorDead();
}

void DemoPositionController::control() {
    calcAnim();
}

void DemoPositionController::initAnimCamera(const char* pCameraName) {
    MR::initAnimCamera(this, this->pCameraInfo, pCameraName);
}

void DemoPositionController::startDemo(const char* pCameraName) {
    appear();
    MR::startAction(this, pCameraName);
    MR::startAnimCameraTargetSelf(this, pCameraInfo, pCameraName, 0, false, 1.0f);
}

void DemoPositionController::endDemo(const char* pCameraName) {
    MR::endAnimCamera(this, pCameraInfo, pCameraName, -1, true);
    kill();
}

void DemoPositionController::setAnimCameraLastFrame(const char* pCameraName) {
    MR::setAnimCameraLastFrame(this, pCameraInfo, pCameraName);
}

DemoPositionController::~DemoPositionController() {
}
