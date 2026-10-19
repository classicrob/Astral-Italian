#include "Game/AreaObj/SoundEmitterCube.hpp"
#include "Game/System/GameDataFunction.hpp"

void SoundEmitterCubeWorldMapSync::init(const JMapInfoIter& rIter) {
    SoundEmitterCube::init(rIter);
    if (GameDataFunction::getCurrentWorldId() != mObjArg7) {
        invalidate();
    }
}

SoundEmitterCubeWorldMapSync::~SoundEmitterCubeWorldMapSync() {}
