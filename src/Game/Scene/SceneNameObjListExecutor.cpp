#include "Game/Scene/SceneNameObjListExecutor.hpp"
#include "Game/Util.hpp"
#include "Game/Scene/SceneFunction.hpp"

namespace {
    // Tables are in category-id order (SMG2). Ids are raw: the MR::MovementType/DrawType/DrawBufferType enums in SceneFunction.hpp
    // are Petari-derived and their values do not match this build.
    const CategoryListInitialTable cMovementListInitTable[] = {
        {0x00, 1},
        {0x01, 16},
        {0x02, 1},
        {0x03, 16},
        {0x04, 1},
        {0x05, 1},
        {0x06, 32},
        {0x07, 16},
        {0x08, 1},
        {0x09, 1},
        {0x0A, 1},
        {0x0B, 1},
        {0x0C, 32},
        {0x0D, 256},
        {0x0E, 96},
        {0x0F, 32},
        {0x10, 8},
        {0x11, 16},
        {0x12, 1},
        {0x13, 64},
        {0x14, 1},
        {0x15, 4},
        {0x16, 1},
        {0x17, 1},
        {0x18, 1},
        {0x19, 1},
        {0x1A, 1},
        {0x1B, 2048},
        {0x1C, 96},
        {0x1D, 1024},
        {0x1E, 128},
        {0x1F, 1},
        {0x20, 512},
        {0x21, 2048},
        {0x22, 512},
        {0x23, 8},
        {0x24, 16},
        {0x25, 32},
        {0x26, 1},
        {0x27, 96},
        {0x28, 256},
        {0x29, 1024},
        {0x2A, 768},
        {0x2B, 512},
        {0x2C, 1},
        {0x2D, 1},
        {0x2E, 1},
        {0x2F, 256},
        {0x30, 128},
#ifdef GALAXY_NATIVE
        {0xFFFFFFFFu, 0},
#else
        {-1, 0},
#endif
    };
    const CategoryListInitialTable cCalcAnimListInitTable[] = {
        {MR::CalcAnimType_ClippedMapParts, 1280},
        {MR::CalcAnimType_Planet, 96},
        {MR::CalcAnimType_CollisionMapObj, 1024},
        {MR::CalcAnimType_CollisionEnemy, 128},
        {MR::CalcAnimType_Environment, 512},
        {MR::CalcAnimType_MapObj, 1280},
        {MR::CalcAnimType_NPC, 96},
        {MR::CalcAnimType_Ride, 256},
        {MR::CalcAnimType_Enemy, 2048},
        {MR::CalcAnimType_Player, 8},
        {MR::CalcAnimType_PlayerDecoration, 32},
        {MR::CalcAnimType_MapObjDecoration, 768},
        {MR::CalcAnimType_MirrorMapObj, 256},
        {MR::CalcAnimType_Layout, 96},
        {MR::CalcAnimType_LayoutDecoration, 32},
        {MR::CalcAnimType_Item, 512},
        {MR::CalcAnimType_UNK_0x10, 128},
        {MR::CalcAnimType_UNK_0x11, 128},
        {MR::CalcAnimType_AnimParticle, 1},
        {MR::CalcAnimType_AnimParticleIgnorePause, 2},
        {MR::CalcAnimType_WorldMapMini, 256},
        {MR::CalcAnimType_WorldMapIcon, 128},
        {MR::CalcAnimType_UNK_0x16, 1},
#ifdef GALAXY_NATIVE
        {0xFFFFFFFFu, 0},
#else
        {-1, 0},
#endif
    };
    const DrawBufferInitialTable cDrawBufferListInitTable[] = {
        {0x00, 32, 3, 0},
        {0x01, 4, 3, 0},
        {0x02, 4, 3, 0},
        {0x03, 8, 3, 0},
        {0x04, 4, 3, 0},
        {0x05, 64, 3, 0},
        {0x06, 64, 3, 0},
        {0x07, 64, 3, 0},
        {0x08, 64, 3, 0},
        {0x09, 256, 3, 0},
        {0x0A, 32, 2, 0},
        {0x0B, 64, 1, 0},
        {0x0C, 64, 3, 0},
        {0x0D, 64, 1, 0},
        {0x0E, 64, 3, 0},
        {0x0F, 64, 2, 0},
        {0x10, 64, 1, 0},
        {0x11, 80, 1, 0},
        {0x12, 32, 1, 0},
        {0x13, 128, 1, 0},
        {0x14, 64, 1, 0},
        {0x15, 4, 0, 0},
        {0x16, 16, 0, 0},
        {0x17, 8, 0, 0},
        {0x18, 1, 2, 0},
        {0x19, 64, 3, 0},
        {0x1A, 64, 3, 0},
        {0x1B, 64, 1, 0},
        {0x1C, 16, 1, 0},
        {0x1D, 64, 1, 0},
        {0x1E, 64, 3, 0},
        {0x1F, 16, 3, 0},
        {0x20, 64, 3, 0},
        {0x21, 8, 3, 0},
        {0x22, 8, 1, 0},
        {0x23, 8, 3, 0},
        {0x24, 1, 3, 0},
        {0x25, 4, 3, 0},
#ifdef GALAXY_NATIVE
        {0x26, 1, 0xFFFFFFFFu, 0},
#else
        {0x26, 1, -1, 0},
#endif
        {0x27, 64, 3, 3},
        {0x28, 128, 3, 4},
#ifdef GALAXY_NATIVE
        {0x29, 16, 0xFFFFFFFFu, 1},
#else
        {0x29, 16, -1, 1},
#endif
#ifdef GALAXY_NATIVE
        {0x2A, 16, 0xFFFFFFFFu, 1},
#else
        {0x2A, 16, -1, 1},
#endif
#ifdef GALAXY_NATIVE
        {0x2B, 16, 0xFFFFFFFFu, 1},
#else
        {0x2B, 16, -1, 1},
#endif
        {0x2C, 256, 3, 2},
        {0x2D, 64, 3, 0},
#ifdef GALAXY_NATIVE
        {0xFFFFFFFFu, 0, 0xFFFFFFFFu, 0},
#else
        {-1, 0, -1, 0},
#endif
    };
    const CategoryListInitialTable cDrawListInitTable[] = {
        {0x00, 256},
        {0x01, 64},
        {0x02, 64},
        {0x03, 64},
        {0x04, 64},
        {0x05, 64},
        {0x06, 64},
        {0x07, 8},
        {0x08, 16},
        {0x09, 8},
        {0x0A, 8},
        {0x0B, 8},
        {0x0C, 16},
        {0x0D, 64},
        {0x0E, 1},
        {0x0F, 1},
        {0x10, 32},
        {0x11, 64},
        {0x12, 64},
        {0x13, 64},
        {0x14, 64},
        {0x15, 1},
        {0x16, 64},
        {0x17, 32},
        {0x18, 256},
        {0x19, 16},
        {0x1A, 64},
        {0x1B, 256},
        {0x1C, 64},
        {0x1D, 64},
        {0x1E, 1024},
        {0x1F, 1},
        {0x20, 8},
        {0x21, 8},
        {0x22, 8},
        {0x23, 16},
        {0x24, 32},
        {0x25, 4},
        {0x26, 4},
        {0x27, 4},
        {0x28, 1},
        {0x29, 128},
        {0x2A, 256},
        {0x2B, 1024},
        {0x2C, 4},
        {0x2D, 1},
        {0x2E, 256},
        {0x2F, 1},
        {0x30, 1},
        {0x31, 1},
        {0x32, 1},
        {0x33, 1},
        {0x34, 2},
        {0x35, 16},
        {0x36, 1},
        {0x37, 2},
        {0x38, 1},
        {0x39, 1},
        {0x3A, 1},
        {0x3B, 64},
        {0x3C, 4},
        {0x3D, 16},
        {0x3E, 16},
        {0x3F, 1},
        {0x40, 1},
        {0x41, 4},
        {0x42, 1},
        {0x43, 64},
        {0x44, 64},
        {0x45, 8},
        {0x46, 128},
        {0x47, 32},
        {0x48, 32},
        {0x49, 3},
        {0x4A, 8},
        {0x4B, 32},
        {0x4C, 32},
        {0x4D, 1},
        {0x4E, 1},
        {0x4F, 1},
        {0x50, 32},
        {0x51, 32},
        {0x52, 1},
        {0x53, 1},
        {0x54, 1},
        {0x55, 1},
        {0x56, 2},
        {0x57, 1},
        {0x58, 1},
        {0x59, 1},
        {0x5A, 1},
        {0x5B, 1},
        {0x5C, 1},
#ifdef GALAXY_NATIVE
        {0xFFFFFFFFu, 0},
#else
        {-1, 0},
#endif
    };
    static bool sVar = true;
};  // namespace

void SceneNameObjListExecutor::initMovementList() {
    mMovementList =
        new NameObjCategoryList(ARRAY_SIZE(::cMovementListInitTable) - 1, ::cMovementListInitTable, &NameObj::executeMovement, ::sVar, "");
}

void SceneNameObjListExecutor::initCalcAnimList() {
    mCalcAnimList = new NameObjCategoryList(ARRAY_SIZE(::cCalcAnimListInitTable) - 1, ::cCalcAnimListInitTable, &NameObj::calcAnim, ::sVar, "");
}

void SceneNameObjListExecutor::initCalcViewAndEntryList() {
    mBufferHolder = new DrawBufferHolder();
    mBufferHolder->initTable(::cDrawBufferListInitTable, ARRAY_SIZE(::cDrawBufferListInitTable) - 1);
}

void SceneNameObjListExecutor::initDrawList() {
    mDrawList = new NameObjCategoryList(ARRAY_SIZE(::cDrawListInitTable) - 1, ::cDrawListInitTable, &NameObj::draw, ::sVar, "");
}

SceneNameObjListExecutor::~SceneNameObjListExecutor() {
}
