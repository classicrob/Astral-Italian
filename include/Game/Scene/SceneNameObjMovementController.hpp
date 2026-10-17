#pragma once

namespace MR {
    void notifyRequestNameObjMovementOnOff();
};

// Merged from Petari (core lane): declarations missing here
#include "Game/NameObj/NameObj.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/System/NerveExecutor.hpp"
class SceneNameObjMovementController;

namespace MR {
    SceneNameObjMovementController* getSceneNameObjMovementController();
};  // namespace MR
