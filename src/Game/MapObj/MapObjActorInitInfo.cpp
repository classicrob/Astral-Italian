#include "Game/MapObj/MapObjActorInitInfo.hpp"
#include "Game/MapObj/MapObjActor.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/MapPartsUtil.hpp"
#include "Game/Util/StringUtil.hpp"

MapObjActorInitInfo::MapObjActorInitInfo()
    : mSetDefaultPosition(), mConnectToScene(), mInitBinder(), mHasEffect(), mHasSensors(), mHasShadows(), mCalcGravity(), mUseProjectMapMtx(),
      _8(), mHasRailMover(), mHasRotator(), mHasRailRotator(), _C(), _D(), _E(), mUseRailPosture(), mUseBaseMtxFollowTarget(),
      mIsAffectedByScale(), mUseMirrorReflection(), mModelName(), mHioNode(), _1C(), mSensorRadius(), mSensorPosition(gZeroVec),
      mHasHitSensorCB(), mBinderRadius(), mBinderCenterY(), mEffectName(), mEffectNum(), mSound(), mSoundName(), mSoundPos(), mProjmapArg1(),
      mNerve(), mState(), mGroupClipping(), mClippingRadius(), mFarClipping(), mMultiFur(-1), mColorChangeArg(-1), mTextureChangeArg(-1),
      mIsAppearRiddleSE(), mSeesaw1AxisRotatorName(), mSeesaw1AxisRotatorRadius(), mMirrorReflectionArg(), mShadowName(), mShadowLength(-1.0f),
      mBreakModelName(), mDummyChangeTexture(), _94(-1), mUseLightCtrl(), mNoUseLOD() {
}

void MapObjActorInitInfo::setupHioNode(const char* pName) {
    mHioNode = pName;
}

void MapObjActorInitInfo::setupDefaultPos() {
    mSetDefaultPosition = true;
}

void MapObjActorInitInfo::setupModelName(const char* pName) {
    mModelName = pName;
}

void MapObjActorInitInfo::setupConnectToScene() {
    mConnectToScene = true;
}

void MapObjActorInitInfo::setupBinder(f32 radius, f32 centerY) {
    mInitBinder = true;
    mBinderRadius = radius;
    mBinderCenterY = centerY;
}

void MapObjActorInitInfo::setupEffect(const char* pName, s32 num) {
    mHasEffect = true;
    mEffectName = pName;
    mEffectNum = num;
}

void MapObjActorInitInfo::setupSound(s32 num, const char* pName) {
    mSound = num;
    mSoundName = pName;
}

void MapObjActorInitInfo::setupSoundPos(TVec3f* pPos) {
    mSoundPos = pPos;
}

void MapObjActorInitInfo::setupHitSensor() {
    mHasSensors = true;
}

void MapObjActorInitInfo::setupHitSensorParam(u16 a1, f32 radius, const TVec3f& rPosition) {
    _1C = a1;
    mSensorRadius = radius;
    mSensorPosition.set(rPosition);
}

void MapObjActorInitInfo::setupNerve(const Nerve* pNerve) {
    mNerve = pNerve;
}

void MapObjActorInitInfo::setupState(s32 state) {
    mState = state;
}

void MapObjActorInitInfo::setupShadow(const char* pName) {
    mHasShadows = true;
    mShadowName = pName;
}

void MapObjActorInitInfo::setupGroupClipping(s32 id) {
    mGroupClipping = id;
}

void MapObjActorInitInfo::setupClippingRadius(f32 radius) {
    mClippingRadius = radius;
}

void MapObjActorInitInfo::setupFarClipping(f32 distance) {
    mFarClipping = distance;
}

void MapObjActorInitInfo::setupProjmapMtx(bool a1, bool a2) {
    mUseProjectMapMtx = true;
    mProjmapArg1 = a1;
    mProjmapArg2 = a2;
}

void MapObjActorInitInfo::setupMultiFur(s32 num) {
    mMultiFur = num;
}

void MapObjActorInitInfo::setupRailMover() {
    mHasRailMover = true;
}

void MapObjActorInitInfo::setupRotator() {
    mHasRotator = true;
}

void MapObjActorInitInfo::setupRailRotator() {
    mHasRailRotator = true;
}

void MapObjActorInitInfo::setupRailPosture() {
    mUseRailPosture = true;
}

void MapObjActorInitInfo::setupBaseMtxFollowTarget() {
    mUseBaseMtxFollowTarget = true;
}

void MapObjActorInitInfo::setupAffectedScale() {
    mIsAffectedByScale = true;
}

void MapObjActorInitInfo::setupSeAppear() {
    mIsAppearRiddleSE = true;
}

void MapObjActorInitInfo::setupLightCtrl() {
    mUseLightCtrl = true;
}

void MapObjActorInitInfo::setupNoUseLodCtrl() {
    mNoUseLOD = true;
}

void MapObjActorUtil::setupInitInfoSimpleMapObj(MapObjActorInitInfo* pInitInfo) {
    pInitInfo->setupHioNode("地形オブジェ");
    pInitInfo->setupDefaultPos();
    pInitInfo->setupConnectToScene();
    pInitInfo->setupEffect();
    pInitInfo->setupSound(4);
    pInitInfo->setupShadow();
    pInitInfo->setupGroupClipping(64);
    pInitInfo->setupSeAppear();
    pInitInfo->_8 = true;
}

void MapObjActorUtil::setupInitInfoPlanet(MapObjActorInitInfo* pInitInfo) {
    pInitInfo->setupHioNode("惑星");
    pInitInfo->setupDefaultPos();
    pInitInfo->setupConnectToScene();
    pInitInfo->setupEffect();
    pInitInfo->setupFarClipping();
    pInitInfo->setupNoAppearRiddleSE();
}

void MapObjActorUtil::setupInitInfoTypical(MapObjActorInitInfo* pInitInfo, const char* pName) {
    const char* const cNoAppearRiddleSENameList[] = {"dummy"};
    if (MR::isExistString(pName, cNoAppearRiddleSENameList, ARRAY_SIZE(cNoAppearRiddleSENameList))) {
        pInitInfo->setupNoAppearRiddleSE();
    }

    const char* const cProjmapNameList[] = {"BombZoneGravityDisplayPanel"};
    if (MR::isEqualString(cProjmapNameList[0], pName)) {
        pInitInfo->setupProjmapMtx(false, false);
    }

    const char* const cUfoSandObstaclesList[] = {"UFOSandObstacleA", "UFOSandObstacleB", "UFOSandObstacleC"};
    if (MR::isExistString(pName, cUfoSandObstaclesList, ARRAY_SIZE(cUfoSandObstaclesList))) {
        pInitInfo->mShadowName = "Shadow";
    }

    const char* const cKoopaVs2PartsList[] = {"KoopaVS2PartsJoinedMoveStep", "KoopaVS2PartsSquareMoveStepA", "KoopaVS2PartsSquareMoveStepB",
                                              "KoopaVS2PartsNarrowRoad", "KoopaVS2PartsClipAreaDisplayA"};
    if (MR::isExistString(pName, cKoopaVs2PartsList, ARRAY_SIZE(cKoopaVs2PartsList))) {
        pInitInfo->setupHitSensor();
    }

    if (MR::isEqualString("SandUpDownTowerBreakableWallA", pName) || MR::isEqualString("SandUpDownTowerBreakableWallB", pName)) {
        pInitInfo->mBreakModelName = "SandUpDownTowerBreakableWallBreak";
    }

    if (MR::isEqualString("KoopaJrNormalShipA", pName)) {
        pInitInfo->_94 = 33;
    }

    if (MR::isEqualString("KoopaStatue", pName)) {
        pInitInfo->mUseLightCtrl = true;
        pInitInfo->mMultiFur = 2;
    }

    if (MR::isEqualString("DangerSignBoard", pName)) {
        pInitInfo->setupHitSensor();
        pInitInfo->setupHitSensorParam(4, 200.0f, TVec3f(0.0f, 200.0f, 0.0f));
    }

    if (MR::isEqualString("ProminenceSetA", pName) || MR::isEqualString("FoominLavaCrossPlanetProminence", pName) ||
        MR::isEqualString("RepeatBlockLavaBoxPlanetProminence", pName)) {
        pInitInfo->setupFarClipping();
    }
}

void MapObjActorUtil::setupInitInfoColorChangeArg0(MapObjActorInitInfo* pInitInfo, const JMapInfoIter& rIter) {
    s32 arg = -1;
    MR::getJMapInfoArg0NoInit(rIter, &arg);
    pInitInfo->mColorChangeArg = arg;
}

void MapObjActorUtil::setupInitInfoTextureChangeArg1(MapObjActorInitInfo* pInitInfo, const JMapInfoIter& rIter) {
    s32 arg = -1;
    MR::getJMapInfoArg1NoInit(rIter, &arg);
    pInitInfo->mTextureChangeArg = arg;
}

void MapObjActorUtil::setupInitInfoShadowLengthArg2(MapObjActorInitInfo* pInitInfo, const JMapInfoIter& rIter) {
    f32 arg = -1.0f;
    MR::getJMapInfoArg2NoInit(rIter, &arg);
    pInitInfo->mShadowLength = arg;
}

void MapObjActorUtil::setupInitInfoSeesaw(MapObjActorInitInfo* pInitInfo, const JMapInfoIter& rIter, const char* pName, f32 radius) {
    s32 arg = -1;
    MR::getMapPartsArgRotateAxis(&arg, rIter);

    if (arg == 0) {
        pInitInfo->setupSeesaw1AxisRotator(pName, radius);
    } else if (arg == 2) {
        pInitInfo->_E = true;
        pInitInfo->mSeesaw1AxisRotatorName = pName;
        pInitInfo->mSeesaw1AxisRotatorRadius = radius;
    } else {
        pInitInfo->_D = true;
        pInitInfo->mSeesaw1AxisRotatorName = pName;
        pInitInfo->mSeesaw1AxisRotatorRadius = radius;
    }
}
