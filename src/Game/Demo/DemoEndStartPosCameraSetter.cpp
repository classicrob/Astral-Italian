#include "Game/Demo/DemoEndStartPosCameraSetter.hpp"

#include "Game/Camera/CameraManGame.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/Functor.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ObjUtil.hpp"

DemoEndStartPosCameraSetter::DemoEndStartPosCameraSetter(const char* pName) : LiveActor(pName) {
}

void DemoEndStartPosCameraSetter::init(const JMapInfoIter& rIter) {
    MR::connectToSceneMapObjMovement(this);
    MR::invalidateClipping(this);
    makeActorDead();
    registerDemo("ファイルセレクトからの開始", rIter);
    registerDemo("ノーマルエンディング後デモ", rIter);
    registerDemo("スター120個エンディング後デモ", rIter);
    registerDemo("ワールドマップからの開始", rIter);
    registerDemo("スター１個目直後のゲームスタート", rIter);
    registerDemo("ゲームオーバー後のデモ", rIter);
}

void DemoEndStartPosCameraSetter::registerDemo(const char* pName, const JMapInfoIter& rIter) {
    if (MR::tryRegisterDemoCast(this, pName, rIter)) {
        MR::registerDemoActionFunctorDirect(this, MR::FunctorV0M< DemoEndStartPosCameraSetter*, void (DemoEndStartPosCameraSetter::*)() >(this, &DemoEndStartPosCameraSetter::endStartPosCamera), pName, NULL);
    }
}

void DemoEndStartPosCameraSetter::endStartPosCamera() {
    MR::endStartPosCamera();
}

DemoEndStartPosCameraSetter::~DemoEndStartPosCameraSetter() {
}
