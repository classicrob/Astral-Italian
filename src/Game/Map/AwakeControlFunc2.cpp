#include "Game/Map/SleepControllerHolder.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Map/AwakeControllerHolder.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util/Functor.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/SwitchEventFunctorListener.hpp"

bool AwakeControlFunc::addAwakeControl(LiveActor* pActor, const JMapInfoIter& rIter) {
    if (!MR::isExistStageSwitchAwake(rIter)) {
        return false;
    }

    MR::isExistStageSwitchAppear(rIter);
    SwitchEventFunctorListener* pListener = new SwitchEventFunctorListener();
    pListener->setOnFunctor(MR::Functor(pActor, &LiveActor::makeActorAppeared));
    pListener->setOffFunctor(MR::Functor(pActor, &LiveActor::makeActorDead));
    MR::getSceneObj<AwakeControllerHolder>(SceneObj_AwakeControllerHolder)->add(pActor, rIter, pListener);
    return true;
}
