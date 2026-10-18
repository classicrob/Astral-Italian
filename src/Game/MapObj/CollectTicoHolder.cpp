// NONMATCHING: the 0x80290CE0 LiveActorGroup destructor copy retains an extra null check; its canonical name is already owned at 0x8008AD40.
#include "Game/MapObj/CollectTicoHolder.hpp"
#include "Game/NPC/CollectTico.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util.hpp"
CollectTicoHolder::CollectTicoHolder() : LiveActorGroup("チコ集め管理", 16) {}
namespace MR {
void registerCollectTico(CollectTico* actor) {
    MR::createSceneObj(SceneObj_CollectTicoHolder);
    MR::getSceneObj<CollectTicoHolder*>(SceneObj_CollectTicoHolder)->registerActor(actor);
}
void launchRescuedStrayTico(const TPos3f& mtx) {
    if (MR::isExistSceneObj(SceneObj_CollectTicoHolder)) {
        CollectTicoHolder* holder = MR::getSceneObj<CollectTicoHolder*>(SceneObj_CollectTicoHolder);
        s32 count = holder->mObjectCount;
        for (s32 i = 0; i < count; ++i) static_cast<CollectTico*>(holder->getActor(i))->launch(mtx);
    }
}
}
#pragma dont_inline on
CollectTicoHolder::~CollectTicoHolder() {}
#pragma dont_inline reset
