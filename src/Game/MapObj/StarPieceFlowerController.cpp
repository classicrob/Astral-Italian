#include "Game/MapObj/StarPieceFlowerController.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util.hpp"
StarPieceFlowerController::StarPieceFlowerController(const char* name) : NameObj(name) { mRotation.x = 0.0f; mRotation.y = 0.0f; mRotation.z = 0.0f; }
StarPieceFlowerController::~StarPieceFlowerController() {}
void StarPieceFlowerController::init(const JMapInfoIter&) { MR::connectToSceneMapObjMovement(this); }
void StarPieceFlowerController::movement() {
    mRotation.y += 3.0f;
    mRotation.y = MR::repeat(mRotation.y, 0.0f, 360.0f);
}
namespace {
    inline void copyRotation(TVec3f* pRotation, const TVec3f& rotation) {
        *pRotation = rotation;
    }
}

namespace MR {
void getStarPieceFlowerRotation(TVec3f* rotation) {
    copyRotation(rotation, getSceneObj<StarPieceFlowerController>(SceneObj_StarPieceFlowerController)->mRotation);
}
}
