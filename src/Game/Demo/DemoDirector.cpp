#include "Game/Demo/DemoDirector.hpp"
#include "Game/Util.hpp"
#include "Game/Demo/DemoCastGroupHolder.hpp"
#include "Game/Demo/DemoExecutor.hpp"
#include "Game/Demo/DemoFunction.hpp"
#include "Game/Demo/DemoSimpleCastHolder.hpp"
#include "Game/Demo/DemoStartRequestHolder.hpp"
#include "Game/Demo/DemoStartRequestUtil.hpp"
#include "Game/Player/MarioAccess.hpp"
#include "Game/Scene/SceneNameObjMovementController.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/System/ResourceHolder.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/JMapIdInfo.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"

DemoDirector::DemoDirector(const char* pName)
    : NameObj(pName), mIsActive(), _13(), mExecutor(), _14(), _18(), _20(new DemoSimpleCastHolder(512, 64, 128)), mResourceHolder(),
      mStartRequestHolder(new DemoStartRequestHolder()), _2C(), _30(), _34(-1), _38(true) {
    MR::connectToScene(this, MR::MovementType_DemoDirector, MR::CalcAnimType_None, MR::DrawBufferType_None, MR::DrawType_None);

    _18 = new DemoCastGroupHolder();
    _18->initWithoutIter();

    mCastSubGroupHolder = new DemoCastGroupHolder();
    mCastSubGroupHolder->initWithoutIter();

    mResourceHolder = DemoFunction::loadDemoArchive();
}

void DemoDirector::movement() {
    if (mExecutor != nullptr && !_14) {
        mExecutor->movement();
    }

    if (tryStartDemoRequested()) {
        return;
    }

    if (!_14) {
        return;
    }

    if (!MR::isCameraInterpolatingNearlyEnd()) {
        return;
    }

    _14 = false;

    doDemoEndRequest();
}




void DemoDirector::endDemo(NameObj* pParam1, const char* pParam2, bool param3) {
    if (DemoStartRequestUtil::isExistStartDemoRequest(mStartRequestHolder)) {
        MR::sendMsgToAllLiveActor(ACTMES_END_DEMO, nullptr);

        mExecutor = nullptr;

        doDemoEndRequest();
        startDemoRequested();
    } else if (param3 && !MR::isCameraInterpolatingNearlyEnd() && _34 != 3) {
        _14 = true;

        MR::sendMsgToAllLiveActor(ACTMES_END_DEMO, nullptr);

        mExecutor = nullptr;

        MR::getSceneObj< SceneNameObjMovementController >(SceneObj_SceneNameObjMovementController)->requestStopSceneOverwrite(pParam1);
    } else {
        MR::sendMsgToAllLiveActor(ACTMES_END_DEMO, nullptr);

        mExecutor = nullptr;

        doDemoEndRequest();
    }
}

const char* DemoDirector::getCurrentDemoName() const {
    if (mIsActive) {
        return _30;
    }

    return nullptr;
}


bool DemoDirector::isExistTimeKeepDemo(const char* pParam1) const {
    for (int i = 0; i < _18->getObjNum(); i++) {
        if (MR::isName(_18->getCastGroup(i), pParam1)) {
            return true;
        }
    }

    return false;
}

bool DemoDirector::registerDemoCast(LiveActor* pActor, const JMapInfoIter& rIter) {
    if (!MR::isValidInfo(rIter)) {
        return false;
    }

    s32 demoGroupId = -1;

    if (!MR::getJMapInfoDemoGroupID(rIter, &demoGroupId)) {
        return false;
    }

    JMapIdInfo idInfo = JMapIdInfo(MR::getDemoGroupID(rIter), rIter);

    if (_18->tryRegisterDemoActor(pActor, rIter, idInfo)) {
        return true;
    }

    return mCastSubGroupHolder->tryRegisterDemoActor(pActor, rIter, idInfo);
}

bool DemoDirector::registerDemoCast(LiveActor* pActor, const char* pParam2, const JMapInfoIter& rIter) {
    if (_18->tryRegisterDemoActor(pActor, pParam2, rIter)) {
        return true;
    }

    return mCastSubGroupHolder->tryRegisterDemoActor(pActor, pParam2, rIter);
}

void DemoDirector::registerDemoSimpleCast(LiveActor* pActor) {
    _20->registerActor(pActor);
}

void DemoDirector::registerDemoSimpleCast(LayoutActor* pActor) {
    _20->registerActor(pActor);
}

void DemoDirector::registerDemoSimpleCast(NameObj* pObj) {
    _20->registerNameObj(pObj);
}




