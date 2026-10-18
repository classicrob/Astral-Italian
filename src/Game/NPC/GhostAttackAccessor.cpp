#include "Game/NPC/GhostAttackAccessor.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util/SceneUtil.hpp"

GhostAttackAccessor::GhostAttackAccessor() : NameObj("ゴーストアタック"), mGhost(nullptr) {}

namespace MR {
GhostAttackAccessor* registerGhostAttackGhost(GhostAttackGhost* ghost) {
    if (!isGhostLuigiOpenInCurrentStage()) {
        return nullptr;
    }
    GhostAttackAccessor* accessor = static_cast<GhostAttackAccessor*>(createSceneObj(SceneObj_GhostAttackAccessor));
    accessor->mGhost = ghost;
    return accessor;
}
}

GhostAttackAccessor::~GhostAttackAccessor() {}
