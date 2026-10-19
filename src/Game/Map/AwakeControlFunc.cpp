#include "Game/Map/SleepControllerHolder.hpp"
#include "Game/AreaObj/AreaObj.hpp"
#include "Game/Map/AwakeControllerHolder.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util/Functor.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/SwitchEventFunctorListener.hpp"

void AwakeControlFunc::initSyncSleepController() {
    MR::getSceneObj<AwakeControllerHolder>(SceneObj_AwakeControllerHolder)->initSync();
}

bool AwakeControlFunc::addAwakeControl(AreaObj* pArea, const JMapInfoIter& rIter) {
    if (!MR::isExistStageSwitchAwake(rIter)) {
        return false;
    }

    SwitchEventFunctorListener* pListener = new SwitchEventFunctorListener();
    pListener->setOnFunctor(MR::Functor(pArea, &AreaObj::awake));
    pListener->setOffFunctor(MR::Functor(pArea, &AreaObj::sleep));
    MR::getSceneObj<AwakeControllerHolder>(SceneObj_AwakeControllerHolder)->add(pArea, rIter, pListener);
    return true;
}

void AreaObj::awake() {
    mIsValid = true;
}

void AreaObj::sleep() {
    mIsValid = false;
}
