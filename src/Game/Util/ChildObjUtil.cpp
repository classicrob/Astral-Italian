#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/NameObj/NameObjFactory.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/SceneUtil.hpp"

namespace MR {
    LiveActorGroup* initChildObjGroup(const NameObj*, const char* pName, const JMapInfoIter& rIter) {
        s32 count = getChildObjNum(rIter);
        if (count <= 0) return nullptr;
        LiveActorGroup* pGroup = new LiveActorGroup(pName, count);
        for (s32 i = 0; i < count; i++) {
            pGroup->registerActor(static_cast<LiveActor*>(NameObjFactory::initChildObj(rIter, i)));
        }
        return pGroup;
    }
}
