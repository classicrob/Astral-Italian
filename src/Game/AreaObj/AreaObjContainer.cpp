// NONMATCHING: the implicit destructor omits the target's impossible null test on the +0x14 array subobject.
#include "Game/AreaObj/AreaObjContainer.hpp"
#include "Game/Util.hpp"
#include "Game/AreaObj/AreaObj.hpp"
#include "Game/AreaObj/CubeCamera.hpp"
#include "Game/AreaObj/GlaringLightArea.hpp"
#include "Game/AreaObj/ImageEffectArea.hpp"
#include "Game/AreaObj/LightAreaHolder.hpp"
#include "Game/AreaObj/WarpCube.hpp"
#include "Game/AreaObj/WaterArea.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include <algorithm>
#ifdef GALAXY_NATIVE
#include <petari/game_compat.hpp>
#endif
#include <cstring>

namespace {
    struct IsManagerName : std::binary_function< AreaObjMgr*, const char*, bool > {
        bool operator()(const AreaObjMgr* pManager, const char* pName) const {
            const char* pMatch = strstr(pName, pManager->mName);
            return pMatch != nullptr && pMatch == pName;
        }
    };
}  // namespace

template < typename T >
AreaObjMgr* AreaObjContainer::createManager(s32 numMax, const char* pName) {
    return new T(numMax, pName);
}

const Name2CreateManagerFunc AreaObjContainer::cCreateTable[] = {
    {"AreaMoveSphere", 0x10, createManager< AreaObjMgr >},
    {"AudioEffectArea", 0x10, createManager< AreaObjMgr >},
    {"BeeWallShortDistArea", 0x10, createManager< AreaObjMgr >},
    {"BgmChangeArea", 0x10, createManager< AreaObjMgr >},
    {"BgmVolumeFadeArea", 0x4, createManager< AreaObjMgr >},
    {"BindEnd", 0x40, createManager< AreaObjMgr >},
    {"BlueStarGuidance", 0x10, createManager< AreaObjMgr >},
    {"CameraRepulsiveArea", 0x80, createManager< AreaObjMgr >},
    {"CancelBeeMorphArea", 0x10, createManager< AreaObjMgr >},
    {"Celestrial", 0x4, createManager< AreaObjMgr >},
    {"ClipDrawArea", 0x40, createManager< AreaObjMgr >},
    {"CollisionArea", 0x40, createManager< AreaObjMgr >},
    {"CometFireDanceArea", 0x4, createManager< AreaObjMgr >},
    {"CometHopperArea", 0x4, createManager< AreaObjMgr >},
    {"CometNoSpinArea", 0x4, createManager< AreaObjMgr >},
    {"CometRainbowArea", 0x4, createManager< AreaObjMgr >},
    {"CubeCamera", 0xa0, createManager< CubeCameraMgr >},
    {"CumulativeTimerSwitchArea", 0x4, createManager< AreaObjMgr >},
    {"DarkMatterArea", 0x80, createManager< AreaObjMgr >},
    {"DashChargeArea", 0x8, createManager< AreaObjMgr >},
    {"DeathArea", 0x40, createManager< AreaObjMgr >},
    {"DemoPlayerForbidUpdateArea", 0x4, createManager< AreaObjMgr >},
    {"DodoryuClosedArea", 0x8, createManager< AreaObjMgr >},
    {"DreamerDeadCountArea", 0x1, createManager< AreaObjMgr >},
    {"DrillHoleHiddenArea", 0x10, createManager< AreaObjMgr >},
    {"Effect", 0x40, createManager< AreaObjMgr >},
    {"EnforceJumpArea", 0x4, createManager< AreaObjMgr >},
    {"ExtraWallCheckArea", 0x10, createManager< AreaObjMgr >},
    {"ExtraWallCheckCylinder", 0x10, createManager< AreaObjMgr >},
    {"Falls", 0x20, createManager< AreaObjMgr >},
    {"ForbidJump", 0x8, createManager< AreaObjMgr >},
    {"ForbidSupportTicoArea", 0x20, createManager< AreaObjMgr >},
    {"ForbidTriangleJump", 0x10, createManager< AreaObjMgr >},
    {"ForbidWaterSearch", 0x10, createManager< AreaObjMgr >},
    {"ForceDashCube", 0x10, createManager< AreaObjMgr >},
    {"GlaringLightArea", 0x40, createManager< GlaringLightAreaMgr >},
    {"Haze", 0x10, createManager< AreaObjMgr >},
    {"HeavySteering", 0x10, createManager< AreaObjMgr >},
    {"HipDropGuidance", 0x10, createManager< AreaObjMgr >},
    {"ImageEffectArea", 0x20, createManager< ImageEffectAreaMgr >},
    {"LensFlareArea", 0x40, createManager< AreaObjMgr >},
    {"LightArea", 0x80, createManager< LightAreaHolder >},
    {"MarioNoCapArea", 0x4, createManager< AreaObjMgr >},
    {"MercatorCube", 0x1, createManager< AreaObjMgr >},
    {"MessageArea", 0x10, createManager< AreaObjMgr >},
    {"MirrorArea", 0x10, createManager< AreaObjMgr >},
    {"MorphSwitchArea", 0x40, createManager< AreaObjMgr >},
    {"NoMessageArea", 0x1, createManager< AreaObjMgr >},
    {"NonSleep", 0x10, createManager< AreaObjMgr >},
    {"Onimasu", 0x20, createManager< AreaObjMgr >},
    {"PipeMode", 0x4, createManager< AreaObjMgr >},
    {"PlaneCircularMode", 0x10, createManager< AreaObjMgr >},
    {"PlaneCollision", 0x10, createManager< AreaObjMgr >},
    {"PlaneMode", 0x10, createManager< AreaObjMgr >},
    {"PlayerMorphLimitArea", 0x4, createManager< AreaObjMgr >},
    {"PlayerSeArea", 0x8, createManager< AreaObjMgr >},
    {"PlayerSideRetainOffArea", 0x8, createManager< AreaObjMgr >},
    {"PullBack", 0x80, createManager< AreaObjMgr >},
    {"QuakeEffectArea", 0x10, createManager< AreaObjMgr >},
    {"RaceJudge", 0x40, createManager< AreaObjMgr >},
    {"RasterScrollArea", 0x40, createManager< AreaObjMgr >},
    {"RecoveryFloorControlArea", 0x10, createManager< AreaObjMgr >},
    {"RepeatBlockShadowChangeArea", 0x8, createManager< AreaObjMgr >},
    {"RollingWaterBreakArea", 0x4, createManager< AreaObjMgr >},
    {"SceneChangeArea", 0x8, createManager< AreaObjMgr >},
    {"SceneExitArea", 0x8, createManager< AreaObjMgr >},
    {"ShadeCube", 0x40, createManager< AreaObjMgr >},
    {"ShadowColorArea", 0x20, createManager< AreaObjMgr >},
    {"ShadowFarClipCtrlArea", 0x10, createManager< AreaObjMgr >},
    {"SlopeRunningCancelArea", 0x4, createManager< AreaObjMgr >},
    {"SmokeEffectColorArea", 0x10, createManager< AreaObjMgr >},
    {"SoundEmitterCube", 0x8, createManager< AreaObjMgr >},
    {"SoundEmitterSphere", 0x8, createManager< AreaObjMgr >},
    {"SoundPlayArea", 0x10, createManager< AreaObjMgr >},
    {"SpinCloudFrontDefineArea", 0x20, createManager< AreaObjMgr >},
    {"SpinGuidance", 0x10, createManager< AreaObjMgr >},
    {"SpinSwitchArea", 0x40, createManager< AreaObjMgr >},
    {"StoryBookAreaText", 0x40, createManager< AreaObjMgr >},
    {"StoryBookAreaWarp", 0x8, createManager< AreaObjMgr >},
    {"SunLightArea", 0x40, createManager< AreaObjMgr >},
    {"SwitchArea", 0x40, createManager< AreaObjMgr >},
    {"TamakoroJumpGuidance", 0x10, createManager< AreaObjMgr >},
    {"TamakoroMoveGuidance", 0x10, createManager< AreaObjMgr >},
    {"TicoSeedGuidance", 0x10, createManager< AreaObjMgr >},
    {"TogepinAttackFoulArea", 0x10, createManager< AreaObjMgr >},
    {"TowerModeCylinder", 0x4, createManager< AreaObjMgr >},
    {"TripodBossStepStart", 0x40, createManager< AreaObjMgr >},
    {"ViewGroupCtrl", 0x40, createManager< AreaObjMgr >},
    {"WarpCube", 0x40, createManager< WarpCubeMgr >},
    {"Water", 0x40, createManager< WaterAreaMgr >},
    {"WorldMapAccessArea", 0x4, createManager< AreaObjMgr >},
    {"WorldMapSyncSoundEmitterCube", 0x8, createManager< AreaObjMgr >},
    {"YoshiCaptureGuidanceArea", 0x8, createManager< AreaObjMgr >},
    {"YoshiEatGuidanceArea", 0x8, createManager< AreaObjMgr >},
    {"YoshiSendOffArea", 0x4, createManager< AreaObjMgr >},
};

AreaObjContainer::AreaObjContainer(const char* pName) : NameObj(pName) {
    mNumManagers = 0;
}

void AreaObjContainer::init(const JMapInfoIter& rIter) {
    mManagerArray.init(ARRAY_SIZE(cCreateTable));
    for (s32 i = 0; i < ARRAY_SIZE(cCreateTable); i++) {
        const Name2CreateManagerFunc* pName2CreateManagerFunc = &cCreateTable[i];
        AreaObjMgr* mgr = pName2CreateManagerFunc->mCreateFunc(pName2CreateManagerFunc->mNumMax, pName2CreateManagerFunc->mName);
        mgr->init(JMapInfoIter());
        s32 num = mNumManagers;
        mNumManagers++;
        mManagerArray[num] = mgr;
    }
}

AreaObjMgr* AreaObjContainer::getManager(const char* pName) const {
    AreaObjMgr* const* pIter = std::find_if(mManagerArray.begin(), mManagerArray.begin() + mNumManagers, std::binder2nd< IsManagerName, const char* >(IsManagerName(), pName));
    if (pIter != mManagerArray.begin() + mNumManagers) {
        return *pIter;
    }
    return nullptr;
}

AreaObjMgr* AreaObjContainer::getManager2(const char* pName) const {
    return getManager(pName);
}

bool AreaObjContainer::isExistAreaObj(const char* pName) const {
    return getManager(pName)->getNumAreaObj() > 0;
}

AreaObj* AreaObjContainer::getAreaObj(const char* pName, const TVec3f& rVec) const {
    return getManager(pName)->find_in(rVec);
}

namespace MR {
    AreaObjContainer* getAreaObjContainer() {
        return MR::getSceneObj< AreaObjContainer >(SceneObj_AreaObjContainer);
    }
};  // namespace MR
