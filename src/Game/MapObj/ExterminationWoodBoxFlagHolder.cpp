#include "Game/MapObj/ExterminationWoodBoxFlagHolder.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util.hpp"
ExterminationWoodBoxFlagHolder::ExterminationWoodBoxFlagHolder(const char* name) : NameObj(name),mFlag(false) {}
ExterminationWoodBoxFlagHolder::~ExterminationWoodBoxFlagHolder() {}
void ExterminationWoodBoxFlagHolder::init(const JMapInfoIter&) {}
namespace MR {
    void setExterminationWoodBoxFlag() { getSceneObj<ExterminationWoodBoxFlagHolder*>(SceneObj_ExterminationWoodBoxFlagHolder)->mFlag=true; }
    void clearExterminationWoodBoxFlag() { getSceneObj<ExterminationWoodBoxFlagHolder*>(SceneObj_ExterminationWoodBoxFlagHolder)->mFlag=false; }
    bool getExterminationWoodBoxFlagHolder() { return getSceneObj<ExterminationWoodBoxFlagHolder*>(SceneObj_ExterminationWoodBoxFlagHolder)->mFlag; }
}
