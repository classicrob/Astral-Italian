#include "Game/AreaObj/SoundEmitterCube.hpp"
#include "Game/System/GameDataFunction.hpp"

void SoundEmitterCubeWorldMapSync::init(const JMapInfoIter& rIter) {
    SoundEmitterCube::init(rIter);
    s32 world = GameDataFunction::getCurrentWorldId();
    if (world != mObjArg7) {
        invalidate();
    }
}

SoundEmitterCubeWorldMapSync::~SoundEmitterCubeWorldMapSync() {}
