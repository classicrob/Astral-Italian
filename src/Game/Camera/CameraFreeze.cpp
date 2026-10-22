#include "Game/Camera/CameraFreeze.hpp"
#include "Game/Camera/CamTranslatorFreeze.hpp"
#include "Game/Camera/CameraLocalUtil.hpp"
#include "Game/Camera/CameraMan.hpp"

CameraFreeze::CameraFreeze(const char* pName) : Camera(pName) {
}

void CameraFreeze::reset() {
    CameraLocalUtil::setFovy(this, CameraLocalUtil::getFovy(mCameraMan));
    CameraLocalUtil::setWatchPos(this, CameraLocalUtil::getWatchPos(mCameraMan));
    CameraLocalUtil::setPos(this, CameraLocalUtil::getPos(mCameraMan));
    CameraLocalUtil::setUpVec(this, CameraLocalUtil::getUpVec(mCameraMan));
    CameraLocalUtil::setRoll(this, CameraLocalUtil::getRoll(mCameraMan));
}

CameraTargetObj* CameraFreeze::calc() {
    return CameraLocalUtil::getTarget(this);
}

CamTranslatorBase* CameraFreeze::createTranslator() {
    return new CamTranslatorFreeze(this);
}

bool CameraFreeze::isUnknownOff() const {
    return true;
}

bool CameraFreeze::isCorrectingErpPositionOff() const {
    return true;
}
