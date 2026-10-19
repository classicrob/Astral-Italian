#include "Game/AreaObj/HazeCube.hpp"
#include "Game/Util.hpp"
#include "Game/Scene/SceneObjHolder.hpp"

namespace {
    static const f32 sDefaultDistance = 1000.0f;
};  // namespace

HazeCube::HazeCube(s32 formType, const char* pName) : AreaObj(formType, pName), mDistance(::sDefaultDistance) {
}

void HazeCube::init(const JMapInfoIter& rIter) {
    AreaObj::init(rIter);

    if (mObjArg0 >= 0) {
        mDistance = mObjArg0;
    }

    MR::createSceneObj(SceneObj_HeatHazeDirector);
}

bool HazeCube::isInVolume(const TVec3f& position) const {
    if (isValidSwitchA() && !isOnSwitchA()) {
        return false;
    }
    return AreaObj::isInVolume(position);
}
