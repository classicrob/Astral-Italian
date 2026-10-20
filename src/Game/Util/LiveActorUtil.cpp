#define JGEOMETRY_OUTLINE_MATRIX_SET
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Animation/BckCtrl.hpp"
#include "Game/Animation/XanimeCore.hpp"
#include "Game/AudioLib/AudAnmSoundObject.hpp"
#include "Game/LiveActor/ActorActionKeeper.hpp"
#include "Game/LiveActor/ActorAnimKeeper.hpp"
#include "Game/LiveActor/ActorHitReactionCtrl.hpp"
#include "Game/LiveActor/ActorPoseTQ.hpp"
#include "Game/LiveActor/ActorLightCtrl.hpp"
#include "Game/LiveActor/Binder.hpp"
#include "Game/LiveActor/ClippingActorHolder.hpp"
#include "Game/LiveActor/ClippingDirector.hpp"
#include "Game/LiveActor/AllLiveActorGroup.hpp"
#include "Game/LiveActor/CollisionObj.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/LiveActor/DemoObj.hpp"
#include "Game/LiveActor/RailObj.hpp"
#include "Game/LiveActor/ActionFootPrintCtrl.hpp"
#include "Game/Util/FootPrint.hpp"
#include "Game/Util/JMapLinkInfo.hpp"
#include "Game/LiveActor/DisplayListMaker.hpp"
#include "Game/LiveActor/EffectKeeper.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/LiveActor/LiveActorGroupArray.hpp"
#include "Game/LiveActor/MaterialCtrl.hpp"
#include "Game/LiveActor/LodCtrl.hpp"
#include "Game/LiveActor/MirrorActor.hpp"
#include "Game/LiveActor/MirrorCamera.hpp"
#include "Game/LiveActor/ModelManager.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/Map/CollisionParts.hpp"
#include "Game/Map/Flag.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Map/LightFunction.hpp"
#include "Game/NameObj/NameObjExecuteHolder.hpp"
#include "Game/NameObj/NameObjFinder.hpp"
#include "Game/Player/GroupChecker.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/System/ResourceHolder.hpp"
#include "Game/Util.hpp"
#include "Game/Util/CollisionPartsFilter.hpp"
#include "Game/Util/FurMulti.hpp"
#include <cstdio>



namespace {
    CollisionParts* createCollisionParts(ResourceHolder* pResHolder, const char* pName, HitSensor* pSensor, const TPos3f& rMtx,
                                         MR::CollisionScaleType scaleType, s32 unk) {
        CollisionParts* parts = new CollisionParts();
        char kcl[0x80];
        snprintf(kcl, sizeof(kcl), "%s.kcl", pName);
        char pa[0x80];
        snprintf(pa, sizeof(pa), "%s.pa", pName);
        void* kclRes;
        void* paRes = nullptr;
        kclRes = pResHolder->mFileInfoTable->getRes(kcl);

        if (pResHolder->mFileInfoTable->isExistRes(pa)) {
            paRes = pResHolder->mFileInfoTable->getRes(pa);
        }

        switch (scaleType) {
        case MR::CollisionScaleType_AutoEqualScale:
            parts->initWithAutoEqualScale(rMtx, pSensor, kclRes, paRes, unk, false);
            break;
        case MR::CollisionScaleType_NotUsingScale:
            parts->initWithNotUsingScale(rMtx, pSensor, kclRes, paRes, unk, false);
            break;
        case MR::CollisionScaleType_Unk2:
            parts->init(rMtx, pSensor, kclRes, paRes, unk, false);
            break;
        }

        return parts;
    }

    const char* createSubModelObjName(const LiveActor* pActor, const char* pSubName) {
        u32 subLength = strlen(pSubName);
        u32 actorLength = strlen(pActor->mName);
        u32 length = actorLength + subLength + strlen("（）") + 1;
        char* pBuffer = new char[length];
        snprintf(pBuffer, length, "%s（%s）", pActor->mName, pSubName);
        return pBuffer;
    }

    bool tryMakeSubModelName(const char** pObjName, char* pBuf, u32 bufSize, const LiveActor* pActor, const char* pSubModelName) NO_INLINE {
        const char* modelResName = MR::getModelResourceHolder(pActor)->mName;

        if (!MR::isExistSubModel(modelResName, pSubModelName)) {
            return false;
        }

        snprintf(pBuf, bufSize, "%s%s", modelResName, pSubModelName);
        *pObjName = createSubModelObjName(pActor, pSubModelName);
        return true;
    }

    PartsModel* createSubModel(LiveActor* pActor, const char* pSubModelName, MtxPtr pMtx, int drawBufferType) NO_INLINE {
        const char* objName = nullptr;
        char subModelName[0x100];

        if (!tryMakeSubModelName(&objName, subModelName, sizeof(subModelName), pActor, pSubModelName)) {
            return nullptr;
        }

        PartsModel* parts = new PartsModel(pActor, objName, subModelName, pMtx, drawBufferType, false);
        parts->initWithoutIter();
        return parts;
    }

    void changeBckForEffectKeeper(const LiveActor* pActor) NO_INLINE {
        EffectKeeper* keeper = pActor->mEffectKeeper;

        if (keeper != nullptr) {
            keeper->changeBck();
        }
    }

#pragma dont_inline on
    void calcCollisionMtx(TPos3f* pMtx, const LiveActor* pActor, const CollisionParts* pParts) NO_INLINE {
        pMtx->set(pActor->getBaseMtx());

        f32 scale = pActor->mScale.x;
        pMtx->mMtx[0][0] *= scale;
        pMtx->mMtx[0][1] *= scale;
        pMtx->mMtx[0][2] *= scale;
        pMtx->mMtx[1][0] *= scale;
        pMtx->mMtx[1][1] *= scale;
        pMtx->mMtx[1][2] *= scale;
        pMtx->mMtx[2][0] *= scale;
        pMtx->mMtx[2][1] *= scale;
        pMtx->mMtx[2][2] *= scale;
    }
#pragma dont_inline reset

    bool isShowModel(LiveActor* pActor) NO_INLINE {
        return !pActor->mFlags.mIsNoEntryDrawBuffer;
    }

    void callFuncAllGroupMember(const LiveActor* pActor, void (*pFunc)(LiveActor*)) NO_INLINE {
        LiveActorGroupArray* pGroupArray = MR::getSceneObj< LiveActorGroupArray >(SceneObj_LiveActorGroupArray);
        LiveActorGroup* pGroup = pGroupArray->getLiveActorGroup(pActor);

        if (pGroup == nullptr) {
            return;
        }

        for (s32 i = 0; i < pGroup->getObjNum(); i++) {
            LiveActor* pMember = pGroup->getActor(i);

            if (pMember == pActor) {
                continue;
            }

            pFunc(pMember);
        }
    }

    void callMethodAllGroupMember(const LiveActor* pActor, void (LiveActor::*pMethod)()) NO_INLINE {
        LiveActorGroupArray* pGroupArray = MR::getSceneObj< LiveActorGroupArray >(SceneObj_LiveActorGroupArray);
        LiveActorGroup* pGroup = pGroupArray->getLiveActorGroup(pActor);

        if (pGroup == nullptr) {
            return;
        }

        for (s32 i = 0; i < pGroup->getObjNum(); i++) {
            LiveActor* pMember = pGroup->getActor(i);

            if (pMember == pActor) {
                continue;
            }

            (pMember->*pMethod)();
        }
    }

    template < typename T >
    s32 countGroupMember(const LiveActor* pActor, T pPred) NO_INLINE;

    template <>
    s32 countGroupMember< bool (*)(LiveActor*) >(const LiveActor* pActor, bool (*pPred)(LiveActor*)) NO_INLINE {
        LiveActorGroupArray* pGroupArray = MR::getSceneObj< LiveActorGroupArray >(SceneObj_LiveActorGroupArray);
        LiveActorGroup* pGroup = pGroupArray->getLiveActorGroup(pActor);
        s32 count = 0;

        if (pGroup == nullptr) {
            return 0;
        }

        for (s32 i = 0; i < pGroup->getObjNum(); i++) {
            LiveActor* pMember = pGroup->getActor(i);

            if (pMember == pActor) {
                continue;
            }

            if (pPred(pMember)) {
                count++;
            }
        }

        return count;
    }

    template <>
    s32 countGroupMember< bool (*)(const LiveActor*) >(const LiveActor* pActor, bool (*pPred)(const LiveActor*)) NO_INLINE {
        LiveActorGroupArray* pGroupArray = MR::getSceneObj< LiveActorGroupArray >(SceneObj_LiveActorGroupArray);
        LiveActorGroup* pGroup = pGroupArray->getLiveActorGroup(pActor);
        s32 count = 0;

        if (pGroup == nullptr) {
            return 0;
        }

        for (s32 i = 0; i < pGroup->getObjNum(); i++) {
            LiveActor* pMember = pGroup->getActor(i);

            if (pMember == pActor) {
                continue;
            }

            if (pPred(pMember)) {
                count++;
            }
        }

        return count;
    }
};  // namespace

namespace MR {
    void setPosition(LiveActor* pActor, const TVec3f& rPosition) {
        pActor->mPosition.set(rPosition);
    }

    void setPosition(LiveActor* pActor, f32 x, f32 y, f32 z) {
        pActor->mPosition.x = x;
        pActor->mPosition.y = y;
        pActor->mPosition.z = z;
    }

    void setRotation(LiveActor* pActor, const TVec3f& rRotation) {
        pActor->mRotation.set(rRotation);
    }

    void setRotation(LiveActor* pActor, f32 x, f32 y, f32 z) {
        pActor->mRotation.x = x;
        pActor->mRotation.y = y;
        pActor->mRotation.z = z;
    }

    void setRotation(LiveActor* pActor, f32 xyz) {
        pActor->mRotation.x = xyz;
        pActor->mRotation.y = xyz;
        pActor->mRotation.z = xyz;
    }

    void setRotation(LiveActor* pActor, const TPos3f& rMtx) {
        TPos3f mtx;
        mtx.set(static_cast< const f32 (*)[4] >(rMtx));
        mtx.getEuler(pActor->mRotation);
        pActor->mRotation.mult(57.295776f);
    }

    void setRotation(LiveActor* pActor, const TQuat4f& rQuat) {
        rQuat.getEuler(pActor->mRotation);
        pActor->mRotation.mult(57.295776f);
    }

    void setScale(LiveActor* pActor, const TVec3f& rScale) {
        pActor->mScale.set(rScale);
    }

    void setScale(LiveActor* pActor, f32 x, f32 y, f32 z) {
        pActor->mScale.x = x;
        pActor->mScale.y = y;
        pActor->mScale.z = z;
    }

    void setScale(LiveActor* pActor, f32 xyz) {
        pActor->mScale.x = xyz;
        pActor->mScale.y = xyz;
        pActor->mScale.z = xyz;
    }

    void initMirrorReflection(LiveActor* pActor) {
        initDLMakerChangeTex(pActor, "MirrorTex");
        pActor->mModelManager->newDifferedDLBuffer();
        changeModelDataTexAll(pActor, "MirrorTex", *MR::getScreenResTIMG());
        pActor->mModelManager->mDisplayListMaker->addMirrorReflectionMtxSetter();
    }

    CollisionParts* tryCreateCollisionMoveLimit(LiveActor* pActor, HitSensor* pSensor) {
        char kcl[0x80];
        {
            const char* pKclName = "MoveLimit";
            snprintf(kcl, sizeof(kcl), "%s.kcl", pKclName);
        }

        if (!MR::getResourceHolder(pActor)->mFileInfoTable->isExistRes(kcl)) {
            return nullptr;
        }

        TPos3f mtx;
        const char* pCollisionName = "MoveLimit";
        MR::makeMtxTRS(mtx, pActor);
        CollisionParts* parts = ::createCollisionParts(MR::getResourceHolder(pActor), pCollisionName, pSensor, mtx, MR::CollisionScaleType_Unk2, 3);

        if (parts != nullptr) {
            MR::validateCollisionParts(parts);
        }

        return parts;
    }

    CollisionParts* tryCreateCollisionWaterSurface(LiveActor* pActor, HitSensor* pSensor) {
        char kcl[0x80];
        {
            const char* pKclName = "WaterSurface";
            snprintf(kcl, sizeof(kcl), "%s.kcl", pKclName);
        }

        if (!MR::getResourceHolder(pActor)->mFileInfoTable->isExistRes(kcl)) {
            return nullptr;
        }

        TPos3f mtx;
        const char* pCollisionName = "WaterSurface";
        MR::makeMtxTRS(mtx, pActor);
        CollisionParts* parts = ::createCollisionParts(MR::getResourceHolder(pActor), pCollisionName, pSensor, mtx, MR::CollisionScaleType_Unk2, 2);

        if (parts != nullptr) {
            MR::validateCollisionParts(parts);
        }

        return parts;
    }


    const char* createLowModelObjName(const LiveActor* pActor) {
        return ::createSubModelObjName(pActor, "Low");
    }

    const char* createMiddleModelObjName(const LiveActor* pActor) {
        return ::createSubModelObjName(pActor, "Middle");
    }

    PartsModel* createBloomModel(LiveActor* pActor, MtxPtr pMtx) {
        PartsModel* parts = ::createSubModel(pActor, "Bloom", pMtx, 31);

        if (parts != nullptr) {
            MR::registerDemoSimpleCastAll(parts);
        }

        return parts;
    }

    PartsModel* createWaterModel(LiveActor* pActor, MtxPtr pMtx) {
        return ::createSubModel(pActor, "Water", pMtx, 9);
    }

    CollisionObj* createCollisionObj(LiveActor* pHost, const char* pCollisionName, HitSensor* pSensor, MtxPtr pMtx) {
        CollisionObj* pCollisionObj = new CollisionObj(pHost, "コリジョンオブジェ", pCollisionName, pSensor, pMtx, -2);
        pCollisionObj->initWithoutIter();
        return pCollisionObj;
    }

    BreakModel* createBreakModel(LiveActor* pHost, MtxPtr pMtx) {
        const char* pObjName = createSubModelObjName(pHost, "Break");
        char modelName[0x100];
        snprintf(modelName, sizeof(modelName), "%sBreak", getModelResourceHolder(pHost)->mName);

        BreakModel* pBreakModel = new BreakModel(pHost, pObjName, modelName, pMtx, "Break");
        pBreakModel->initWithoutIter();
        return pBreakModel;
    }

    BreakModel* tryCreateBreakModel(LiveActor* pHost, MtxPtr pMtx) {
        const char* pObjName = nullptr;
        char modelName[0x100];

        if (!tryMakeSubModelName(&pObjName, modelName, sizeof(modelName), pHost, "Break")) {
            return nullptr;
        }

        BreakModel* pBreakModel = new BreakModel(pHost, pObjName, modelName, pMtx, "Break");
        pBreakModel->initWithoutIter();
        return pBreakModel;
    }

    DemoObj* tryCreateBreakDustDemoObj(LiveActor* pHost, MtxPtr pMtx) {
        const char* pObjName = nullptr;
        char modelName[0x100];

        if (!tryMakeSubModelName(&pObjName, modelName, sizeof(modelName), pHost, "BreakDust")) {
            return nullptr;
        }

        DemoObj* pDemoObj = new DemoObj(pObjName, pMtx);
        initActor(pDemoObj, modelName, false);
        pDemoObj->initWithoutIter();
        pDemoObj->makeActorDead();
        return pDemoObj;
    }

    ModelObj* createBloomModel2(LiveActor* pActor, MtxPtr pMtx) {
        return (ModelObj*)createBloomModel(pActor, pMtx);  // SMG2 header types this as ModelObj*
    }

    PartsModel* createIndirectPlanetModel(LiveActor* pActor, MtxPtr pMtx) {
        return ::createSubModel(pActor, "Indirect", pMtx, 0x1E);
    }

    PartsModel* createIceModel(LiveActor* pActor, MtxPtr pMtx, bool isIce) {
        if (isIce) {
            return ::createSubModel(pActor, "Ice", pMtx, 9);
        }

        return ::createSubModel(pActor, "Ice", pMtx, 0x1A);
    }

    PartsModel* createIndirectModel(LiveActor* pActor, MtxPtr pMtx) {
        return ::createSubModel(pActor, "Indirect", pMtx, 0x1A);
    }

    MirrorActor* tryCreateMirrorActor(LiveActor* pActor, const char* pModelName) {
        if (!MR::isInAreaObj("MirrorArea", pActor->mPosition)) {
            return nullptr;
        }

        const char* objName = ::createSubModelObjName(pActor, "鏡内モデル");
        MirrorActor* mirror = new MirrorActor(pActor, objName, pModelName);
        mirror->initWithoutIter();
        return mirror;
    }

    bool isOnGround(const LiveActor* pActor) {
        Binder* binder = pActor->mBinder;

        if (binder == nullptr) {
            return false;
        }

        if (!binder->isBindedGround()) {
            return false;
        }

        const TVec3f* normal = binder->mGroundInfo.mParentTriangle.getNormal(0);
        return !(0.0f < pActor->mVelocity.dot(*normal));
    }

    bool isOnGroundCos(const LiveActor* pActor, f32 cos) {
        bool isGround = false;

        if (isOnGround(pActor)) {
            const TVec3f* normal = pActor->mBinder->mGroundInfo.mParentTriangle.getNormal(0);

            if (MR::isFloorPolygonCos(*normal, pActor->mGravity, cos)) {
                isGround = true;
            }
        }

        return isGround;
    }

    bool isBindedGround(const LiveActor* pActor) {
        Binder* binder = pActor->mBinder;

        if (binder == nullptr) {
            return false;
        }

        return binder->isBindedGround();
    }

    bool isBindedGround(const LiveActor* pActor, HitSensor* pSensor) {
        if (isBindedGround(pActor)) {
            return pSensor == MR::getGroundSensor(pActor);
        }

        return false;
    }

    bool isBindedWall(const LiveActor* pActor) {
        Binder* binder = pActor->mBinder;

        if (binder == nullptr) {
            return false;
        }

        return binder->isBindedWall();
    }

    bool isBindedWall(const LiveActor* pActor, HitSensor* pSensor) {
        if (isBindedWall(pActor)) {
            return pSensor == MR::getWallSensor(pActor);
        }

        return false;
    }

    bool isBindedWallOfMap(const LiveActor* pActor) {
        bool isMap = false;

        if (isBindedWall(pActor)) {
            const CollisionParts* parts = pActor->mBinder->mWallInfo.mParentTriangle.mParts;

            if (parts->mKeeperIndex == 0) {
                isMap = true;
            }
        }

        return isMap;
    }

    bool isBindedWallOfMoveLimit(const LiveActor* pActor) {
        bool isMoveLimit = false;

        if (isBindedWall(pActor)) {
            const CollisionParts* parts = pActor->mBinder->mWallInfo.mParentTriangle.mParts;

            if (parts->mKeeperIndex == 3) {
                isMoveLimit = true;
            }
        }

        return isMoveLimit;
    }


    bool isBindedRoof(const LiveActor* pActor) {
        if (pActor->mBinder == nullptr) {
            return false;
        }

        return pActor->mBinder->isBindedRoof();
    }

    bool isBindedRoof(const LiveActor* pActor, HitSensor* pSensor) {
        if (isBindedRoof(pActor)) {
            return pSensor == MR::getRoofSensor(pActor);
        }

        return false;
    }

    bool isBinded(const LiveActor* pActor) {
        if (isBindedGround(pActor) || isBindedRoof(pActor) || isBindedWall(pActor)) {
            return true;
        }

        return false;
    }

    bool isBinded(const LiveActor* pActor, HitSensor* pSensor) {
        if (isBindedGround(pActor, pSensor) || isBindedRoof(pActor, pSensor) || isBindedWall(pActor, pSensor)) {
            return true;
        }

        return false;
    }

    bool isPressedRoofAndGround(const LiveActor* pActor) {
        if (!isBindedRoof(pActor) || !isBindedGround(pActor)) {
            return false;
        }

        if (MR::isSensorPressObj(MR::getGroundSensor(pActor)) || MR::isSensorPressObj(MR::getRoofSensor(pActor))) {
            const Binder* binder = pActor->mBinder;
            const HitInfo* pRoofInfo = &binder->mRoofInfo;
            const HitInfo* pGroundInfo = &binder->mGroundInfo;
            TVec3f roofPower;
            TVec3f groundPower;
            pRoofInfo->mParentTriangle.calcForceMovePower(&roofPower, pRoofInfo->mHitPos);
            pGroundInfo->mParentTriangle.calcForceMovePower(&groundPower, pGroundInfo->mHitPos);

            TVec3f diff(roofPower);
            diff.sub(groundPower);

            if (0.0f < diff.dot(pActor->mGravity)) {
                return true;
            }
        }

        return false;
    }


    bool isPressedBetweenWalls(const LiveActor* pActor) {
        if (!isBindedWall(pActor)) {
            return false;
        }

        s32 planeNum = pActor->mBinder->mPlaneNum;

        for (s32 i = 0; i < planeNum - 1; i++) {
            const HitInfo* pPlane = pActor->mBinder->getPlane(i);
            if (!MR::isWallPolygon(*pPlane->mParentTriangle.getFaceNormal(), pActor->mGravity)) {
                continue;
            }

            TVec3f forcePower;
            pPlane->mParentTriangle.calcForceMovePower(&forcePower, pPlane->mHitPos);

            for (s32 j = i + 1; j < planeNum; j++) {
                const HitInfo* pOther = pActor->mBinder->getPlane(j);
                if (!MR::isWallPolygon(*pOther->mParentTriangle.getFaceNormal(), pActor->mGravity)) {
                    continue;
                }

                if (0.0f <= pPlane->mParentTriangle.getFaceNormal()->dot(*pOther->mParentTriangle.getFaceNormal())) {
                    continue;
                }

                TVec3f otherPower;
                pOther->mParentTriangle.calcForceMovePower(&otherPower, pOther->mHitPos);

                TVec3f posDiff(pPlane->mHitPos);
                posDiff.sub(pOther->mHitPos);

                TVec3f powerDiff(otherPower);
                powerDiff.sub(forcePower);

                if (0.0f < powerDiff.dot(posDiff)) {
                    return true;
                }
            }
        }

        return false;
    }

    void copyTransRotate(const LiveActor* pToActor, LiveActor* pFromActor) {
        pFromActor->mPosition.set(pToActor->mPosition);
        pFromActor->mRotation.set(pToActor->mRotation);
    }

    void rotateAroundActor(TVec3f& rDst, const TVec3f& rSrc, const LiveActor* pActor) {
        TPos3f mtx;
        makeMtxRotate(mtx, pActor->mRotation);
        mtx.mult(rSrc, rDst);
    }

    void copyTransRotateScale(const LiveActor* pSrc, LiveActor* pDst) {
        pDst->mPosition.set(pSrc->mPosition);
        pDst->mRotation.set(pSrc->mRotation);
        pDst->mScale.set(pSrc->mScale);
    }



    bool isValidMovement(const LiveActor* pActor) {
        if (pActor->mFlags.mIsDead || pActor->mFlags.mIsClipped) {
            return false;
        }

        return true;
    }

    bool isValidCalcAnim(const LiveActor* pActor) {
        if (pActor->mFlags.mIsDead || pActor->mFlags.mIsClipped || pActor->mFlags.mIsOnCalcAnim) {
            return false;
        }

        return true;
    }

    bool isValidCalcViewAndEntry(const LiveActor* pActor) {
        if (pActor->mFlags.mIsDead || pActor->mFlags.mIsClipped) {
            return false;
        }

        return true;
    }

    bool isValidDraw(const LiveActor* pActor) {
        if (pActor->mFlags.mIsDead || pActor->mFlags.mIsClipped || pActor->mFlags.mIsNoEntryDrawBuffer) {
            return false;
        }

        return true;
    }

    void calcAnimDirect(LiveActor* pActor) {
        bool isNoCalcAnim = pActor->mFlags.mIsOnCalcAnim;

        pActor->mFlags.mIsOnCalcAnim = false;
        pActor->calcAnim();

        if (isNoCalcAnim) {
            pActor->mFlags.mIsOnCalcAnim = true;
        }
    }

    void setClippingTypeSphere(LiveActor* pActor, f32 radius) {
        MR::getClippingDirector()->mActorHolder->setTypeToSphere(pActor, radius, nullptr);
    }

    void setClippingTypeSphere(LiveActor* pActor, f32 radius, const TVec3f* pTrans) {
        MR::getClippingDirector()->mActorHolder->setTypeToSphere(pActor, radius, pTrans);
    }

    void setClippingTypeSphereContainsModelBoundingBox(LiveActor* pActor, f32 radiusOffset) {
        f32 modelBoundingRadius = 0.0f;
        MR::calcModelBoundingRadius(&modelBoundingRadius, pActor);

        setClippingTypeSphere(pActor, modelBoundingRadius + radiusOffset, nullptr);
    }

    void setClippingFar50m(LiveActor* pActor) {
        MR::getClippingDirector()->mActorHolder->setFarClipLevel(pActor, 7);
    }

    void setClippingFar100m(LiveActor* pActor) {
        MR::getClippingDirector()->mActorHolder->setFarClipLevel(pActor, 6);
    }

    void setClippingFar200m(LiveActor* pActor) {
        MR::getClippingDirector()->mActorHolder->setFarClipLevel(pActor, 5);
    }

    void setClippingFar300m(LiveActor* pActor) {
        MR::getClippingDirector()->mActorHolder->setFarClipLevel(pActor, 4);
    }

    void setClippingFarMax(LiveActor* pActor) {
        MR::getClippingDirector()->mActorHolder->setFarClipLevel(pActor, 0);
    }

    void setClippingFar(LiveActor* pActor, f32 clipping) {
        s32 clip = clipping;

        switch (clip) {
        case 50:
            MR::getClippingDirector()->mActorHolder->setFarClipLevel(pActor, 7);
            break;
        case 100:
            MR::getClippingDirector()->mActorHolder->setFarClipLevel(pActor, 6);
            break;
        case 200:
            MR::getClippingDirector()->mActorHolder->setFarClipLevel(pActor, 5);
            break;
        case 300:
            MR::getClippingDirector()->mActorHolder->setFarClipLevel(pActor, 4);
            break;
        case 400:
            MR::getClippingDirector()->mActorHolder->setFarClipLevel(pActor, 3);
            break;
        case 500:
            MR::getClippingDirector()->mActorHolder->setFarClipLevel(pActor, 2);
            break;
        case 600:
            MR::getClippingDirector()->mActorHolder->setFarClipLevel(pActor, 1);
            break;
        case -1:
            MR::getClippingDirector()->mActorHolder->setFarClipLevel(pActor, 0);
            break;
        }
    }

    void setGroupClipping(LiveActor* pActor, const JMapInfoIter& rIter, int a3) {
        MR::getClippingDirector()->joinToGroupClipping(pActor, rIter, a3);
    }

    void validateClipping(LiveActor* pActor) {
        if (pActor->mFlags.mIsClippingInvalid) {
            MR::getClippingDirector()->mActorHolder->validateClipping(pActor);
        }
    }

    void invalidateClipping(LiveActor* pActor) {
        if (pActor->mFlags.mIsClippingInvalid) {
            if (pActor->mFlags.mIsClipped) {
                pActor->endClipped();
            }
        } else {
            MR::getClippingDirector()->mActorHolder->invalidateClipping(pActor);
        }
    }

    bool changeShowModelFlagSyncNearClipping(LiveActor* pActor, f32 nearClip) {
        if (MR::isJudgedToNearClip(pActor->mPosition, nearClip)) {
            MR::hideModelAndOnCalcAnimIfShown(pActor);
            return false;
        }

        MR::showModelIfHidden(pActor);
        return true;
    }

    bool isClipped(const LiveActor* pActor) {
        return pActor->mFlags.mIsClipped;
    }

    bool isInvalidClipping(const LiveActor* pActor) {
        return pActor->mFlags.mIsClippingInvalid;
    }

    void setBaseTRMtx(LiveActor* pActor, MtxPtr pMtx) {
        PSMTXCopy(pMtx, getJ3DModel(pActor)->mBaseTransformMtx.mMtx);
    }

    void setBaseTRMtx(LiveActor* pActor, const TPos3f& rMtx) {
        PSMTXCopy((MtxPtr)rMtx.mMtx, getJ3DModel(pActor)->mBaseTransformMtx.mMtx);
    }

    void setBaseTRMtx(LiveActor* pActor, const TQuat4f& rQuat) {
        TPos3f mtx;
        mtx.setQT(rQuat, pActor->mPosition);

        PSMTXCopy((MtxPtr)mtx.mMtx, getJ3DModel(pActor)->mBaseTransformMtx.mMtx);
    }

    void setBaseTRMtx(LiveActor* pActor, ActorPoseTQ* pPose) {
        setBaseTRMtx(pActor, *pPose);
    }

    void setBaseScale(LiveActor* pActor, const TVec3f& rScale) {
        J3DModel* pModel = getJ3DModel(pActor);
        pModel->mBaseScale.x = rScale.x;
        pModel->mBaseScale.y = rScale.y;
        pModel->mBaseScale.z = rScale.z;
    }

    ResourceHolder* getResourceHolder(const LiveActor* pActor) {
        if (pActor->mModelManager != nullptr) {
            return pActor->mModelManager->getResourceHolder();
        }

        return nullptr;
    }

    ResourceHolder* getModelResourceHolder(const LiveActor* pActor) {
        if (pActor->mModelManager != nullptr) {
            return pActor->mModelManager->getModelResourceHolder();
        }

        return nullptr;
    }

    ResTIMG* getTexFromModel(const char* pName, const LiveActor* pActor) {
        s32 texIndex = getJ3DModelData(pActor)->mMaterialTable.getTextureName()->getIndex(pName);
        return getJ3DModelData(pActor)->mMaterialTable.getTexture()->getResTIMG(texIndex);
    }

    ResTIMG* getTexFromArc(const char* pName, const LiveActor* pActor) {
        return (ResTIMG*)getResourceHolder(pActor)->mFileInfoTable->getRes(pName);
    }

    const char* getModelResName(const LiveActor* pActor) {
        return getModelResourceHolder(pActor)->mName;
    }








    bool isExistTexture(const LiveActor* pActor, const char* pName) {
        return getJ3DModelData(pActor)->mMaterialTable.getTextureName()->getIndex(pName) != -1;
    }

    bool isExistIndirectTexture(const LiveActor* pActor) {
        return isExistTexture(pActor, "IndDummy") || isExistTexture(pActor, "ScreenDummy");
    }

    void newDifferedDLBuffer(LiveActor* pActor) {
        pActor->mModelManager->newDifferedDLBuffer();
    }

    void initDLMakerFog(LiveActor* pActor, bool enable) {
        pActor->mModelManager->mDisplayListMaker->addFogCtrl(enable);
    }

    void initDLMakerMatColor0(LiveActor* pActor, const char* pMatName, const J3DGXColor* pColor) {
        pActor->mModelManager->mDisplayListMaker->addMatColorCtrl(pMatName, 0, pColor);
    }

    void initDLMakerChangeTex(LiveActor* pActor, const char* pTexName) {
        J3DModelData* pModelData = getJ3DModelData(pActor);
        DisplayListMaker* pDLMaker = pActor->mModelManager->mDisplayListMaker;

        for (u16 texIndex = 0; texIndex < pModelData->mMaterialTable.getTexture()->getNum(); texIndex++) {
            if (!MR::isEqualString(pModelData->mMaterialTable.getTextureName()->getName(texIndex), pTexName)) {
                continue;
            }

            for (u16 matIndex = 0; matIndex < pModelData->mMaterialTable.getMaterialNum(); matIndex++) {
                if (isUseTex(pModelData->mMaterialTable.getMaterialNodePointer(matIndex), texIndex)) {
                    pDLMaker->onPrgFlag(matIndex, 0x4020000);
                }
            }
        }
    }

    void initDLMakerCullMode(LiveActor* pActor, const char* pCullMode) {
        pActor->mModelManager->mDisplayListMaker->addTexMtxCtrl(pCullMode);
    }

    ProjmapEffectMtxSetter* initDLMakerProjmapEffectMtxSetter(LiveActor* pActor) {
        return pActor->mModelManager->mDisplayListMaker->addProjmapEffectMtxSetter();
    }


















    void setFogColor(LiveActor* pActor, const GXColor& rColor) {
        pActor->mModelManager->mDisplayListMaker->mFogCtrl->mFogInfo.mColor = rColor;
    }

    void setFogColorWithAlpha(LiveActor* pActor, const GXColor& rColor, bool) {
        J3DFogInfo& rFogInfo = pActor->mModelManager->mDisplayListMaker->mFogCtrl->mFogInfo;
        rFogInfo.mColor.r = rColor.r;
        rFogInfo.mColor.g = rColor.g;
        rFogInfo.mColor.b = rColor.b;
        rFogInfo.mColor.a = rColor.a;
    }

    void setFogType(LiveActor* pActor, u8 type) {
        pActor->mModelManager->mDisplayListMaker->mFogCtrl->mFogInfo.mType = type;
    }

    void setFogStartEndZ(LiveActor* pActor, f32 startZ, f32 endZ) {
        J3DFogInfo& rFogInfo = pActor->mModelManager->mDisplayListMaker->mFogCtrl->mFogInfo;
        rFogInfo.mStartZ = startZ;
        rFogInfo.mEndZ = endZ;
    }

    void setFogNearFarZ(LiveActor* pActor, f32 nearZ, f32 farZ) {
        J3DFogInfo& rFogInfo = pActor->mModelManager->mDisplayListMaker->mFogCtrl->mFogInfo;
        rFogInfo.mNearZ = nearZ;
        rFogInfo.mFarZ = farZ;
    }

    const char* getPlayingBckName(const LiveActor* pActor) {
        return pActor->mModelManager->getPlayingBckName();
    }

    void setBinderIgnoreMovingCollision(LiveActor* pActor) {
#ifdef GALAXY_NATIVE
        pActor->mBinder->_1EC._0 = false;
#else
        s8* pFlags = (s8*)&pActor->mBinder->_1EC;
        s32 flags = *pFlags;
        flags &= 0xFFFFFF7F;
        *pFlags = flags;
#endif
    }

    ActorHitReactionCtrl* getHitReactionCtrl(const LiveActor* pActor) {
        if (pActor->mActionKeeper == nullptr) {
            return nullptr;
        }

        ActorHitReactionCtrl* pCtrl = pActor->mActionKeeper->mHitReactionCtrl;
        if (pCtrl != nullptr) {
            return pCtrl;
        }

        return nullptr;
    }

    void tryHitReaction(const LiveActor* pActor, const char* pName) {
        ActorHitReactionCtrl* pCtrl = getHitReactionCtrl(pActor);
        if (pCtrl != nullptr) {
            pCtrl->reactToHit(pName, nullptr, nullptr, nullptr);
        }
    }

    void tryHitReactionBetweenSensors(const LiveActor* pActor, const char* pName, HitSensor* pSender, HitSensor* pReceiver) {
        ActorHitReactionCtrl* pCtrl = getHitReactionCtrl(pActor);
        if (pCtrl != nullptr) {
            pCtrl->reactToHit(pName, nullptr, pSender, pReceiver);
        }
    }

    void tryHitReactionAtPos(const LiveActor* pActor, const char* pName, const TVec3f* pPos) {
        ActorHitReactionCtrl* pCtrl = getHitReactionCtrl(pActor);
        if (pCtrl != nullptr) {
            pCtrl->reactToHit(pName, pPos, nullptr, nullptr);
        }
    }

    void tryHitReactionAtMtx(const LiveActor* pActor, const char* pName, const TPos3f* pMtx) {
        ActorHitReactionCtrl* pCtrl = getHitReactionCtrl(pActor);
        if (pCtrl != nullptr) {
            TVec3f pos(pMtx->mMtx[0][3], pMtx->mMtx[1][3], pMtx->mMtx[2][3]);
            pCtrl->reactToHit(pName, &pos, nullptr, nullptr);
        }
    }

    void tryHitReactionBlowBetweenSensors(LiveActor* pActor, HitSensor* pSender, HitSensor* pReceiver) {
        const char* pName = "吹き飛びヒット";
        ActorHitReactionCtrl* pCtrl = getHitReactionCtrl(pActor);
        if (pCtrl != nullptr) {
            pCtrl->reactToHit(pName, nullptr, pSender, pReceiver);
        }
    }

    void tryHitReactionBlow(const LiveActor* pActor, const TVec3f* pPos) {
        const char* pName = "吹き飛びヒット";
        ActorHitReactionCtrl* pCtrl = getHitReactionCtrl(pActor);
        if (pCtrl != nullptr) {
            pCtrl->reactToHit(pName, pPos, nullptr, nullptr);
        }
    }

    void startBlowHitReaction(LiveActor* pActor) {
        const char* pName = "吹き飛びヒット";
        ActorHitReactionCtrl* pCtrl = getHitReactionCtrl(pActor);
        if (pCtrl != nullptr) {
            pCtrl->reactToHit(pName, nullptr, nullptr, nullptr);
        }
    }

    void tryHitReactionAppear(const LiveActor* pActor) {
        const char* pName = "出現";
        ActorHitReactionCtrl* pCtrl = getHitReactionCtrl(pActor);
        if (pCtrl != nullptr) {
            pCtrl->reactToHit(pName, nullptr, nullptr, nullptr);
        }
    }

    void tryHitReactionBreak(const LiveActor* pActor) {
        const char* pName = "破壊";
        ActorHitReactionCtrl* pCtrl = getHitReactionCtrl(pActor);
        if (pCtrl != nullptr) {
            pCtrl->reactToHit(pName, nullptr, nullptr, nullptr);
        }
    }

    void tryHitReactionDead(const LiveActor* pActor) {
        const char* pName = "死亡";
        ActorHitReactionCtrl* pCtrl = getHitReactionCtrl(pActor);
        if (pCtrl != nullptr) {
            pCtrl->reactToHit(pName, nullptr, nullptr, nullptr);
        }
    }

    LiveActorGroup* joinToGroup(LiveActor* pActor, const char* pGroupName) {
        LiveActorGroup* pGroup = (LiveActorGroup*)NameObjFinder::find(pGroupName);
        pGroup->registerActor(pActor);
        return pGroup;
    }

    LiveActorGroup* joinToGroupArray(LiveActor* pActor, const JMapInfoIter& rIter, const char* pName, s32 maxCount) {
        if (!rIter.isValid()) {
            return nullptr;
        }

        s32 groupId;

        if (getJMapInfoGroupID(rIter, &groupId)) {
            LiveActorGroupArray* pGroupArray = MR::getSceneObj< LiveActorGroupArray >(SceneObj_LiveActorGroupArray);
            return pGroupArray->tryCreateGroupForLiveActor(pActor, rIter, pName, maxCount);
        }

        return nullptr;
    }

    LiveActorGroup* getGroupFromArray(const LiveActor* pActor) {
        LiveActorGroupArray* pGroupArray = MR::getSceneObj< LiveActorGroupArray >(SceneObj_LiveActorGroupArray);
        return pGroupArray->getLiveActorGroup(pActor);
    }


    void callMakeActorDeadAllGroupMember(const LiveActor* pActor) {
        ::callMethodAllGroupMember(pActor, &LiveActor::makeActorDead);
    }

    void callKillAllGroupMember(const LiveActor* pActor) {
        ::callMethodAllGroupMember(pActor, &LiveActor::kill);
    }

    void callMakeActorAppearedAllGroupMember(const LiveActor* pActor) {
        ::callMethodAllGroupMember(pActor, &LiveActor::makeActorAppeared);
    }

    void callAppearAllGroupMember(const LiveActor* pActor) {
        ::callMethodAllGroupMember(pActor, &LiveActor::appear);
    }

    void callRequestMovementOnAllGroupMember(const LiveActor* pActor) {
        ::callFuncAllGroupMember(pActor, requestMovementOn);
    }

    void callInvalidateClippingAllGroupMember(const LiveActor* pActor) {
        ::callFuncAllGroupMember(pActor, invalidateClipping);
    }

    void callValidateClippingAllGroupMember(const LiveActor* pActor) {
        ::callFuncAllGroupMember(pActor, validateClipping);
    }



    void addToAttributeGroupSearchTurtle(const LiveActor* pActor) {
        GroupCheckManager* pGroup = MR::getSceneObj< GroupCheckManager >(SceneObj_GroupCheckManager);
        pGroup->add(pActor, 0);
    }


    bool isExistInAttributeGroupSearchTurtle(const LiveActor* pActor) {
        GroupCheckManager* pGroup = MR::getSceneObj< GroupCheckManager >(SceneObj_GroupCheckManager);
        return pGroup->isExist(pActor, 0);
    }





































































    void setMirrorReflectionInfoFromMtxYUp(const TPos3f& rMtx) {
        TVec3f up;
        TVec3f pos;
        rMtx.getYDir(up);
        rMtx.getTrans(pos);
        MR::getMirrorCamera()->setMirrorMapInfo(up, pos);
    }

    void setMirrorReflectionInfoFromModel(LiveActor* pActor) {
        J3DModelData* pModelData = MR::getJ3DModelData(pActor);
        MR::getMirrorCamera()->setMirrorMapInfo(pModelData);
    }

    void changeModelDataTexAll(LiveActor* pActor, const char* pTexName, const ResTIMG& rTimg) {
        J3DModelData* pModelData = MR::getJ3DModelData(pActor);
        DisplayListMaker* pDLMaker = pActor->mModelManager->mDisplayListMaker;

        for (u16 texIndex = 0; texIndex < pModelData->mMaterialTable.getTexture()->getNum(); texIndex++) {
            const char* name = pModelData->mMaterialTable.getTextureName()->getName(texIndex);

            if (strcmp(name, pTexName) != 0) {
                continue;
            }

            pModelData->mMaterialTable.getTexture()->setResTIMG(texIndex, rTimg);

            for (u16 matIndex = 0; matIndex < pModelData->mMaterialTable.getMaterialNum(); matIndex++) {
                J3DMaterial* material = pModelData->mMaterialTable.getMaterialNodePointer(matIndex);

                if (MR::isUseTex(material, texIndex)) {
                    pDLMaker->onCurFlag(matIndex, 0x4020000);
                }
            }
        }
    }

    void initJointTransform(const LiveActor* pActor) {
        pActor->mModelManager->initJointTransform();
    }

    XjointTransform* getJointTransform(const LiveActor* pActor, const char* pName) {
        return pActor->mModelManager->getJointTransform(pName);
    }


    f32 getBckFrame(const LiveActor* pActor) {
        return getBckCtrl(pActor)->getFrame();
    }

    f32 getBrkFrame(const LiveActor* pActor) {
        return getBrkCtrl(pActor)->getFrame();
    }

    f32 getBtpFrame(const LiveActor* pActor) {
        return getBtpCtrl(pActor)->getFrame();
    }

    f32 getBtkFrame(const LiveActor* pActor) {
        return getBtkCtrl(pActor)->getFrame();
    }

    f32 getBvaFrame(const LiveActor* pActor) {
        return getBvaCtrl(pActor)->getFrame();
    }

    f32 getBckRate(const LiveActor* pActor) {
        return getBckCtrl(pActor)->getRate();
    }

    f32 getBtkRate(const LiveActor* pActor) {
        return getBtkCtrl(pActor)->getRate();
    }

    f32 getBrkRate(const LiveActor* pActor) {
        return getBrkCtrl(pActor)->getRate();
    }

    f32 getBtpRate(const LiveActor* pActor) {
        return getBtpCtrl(pActor)->getRate();
    }

    f32 getBpkRate(const LiveActor* pActor) {
        return getBpkCtrl(pActor)->getRate();
    }

    f32 getBvaRate(const LiveActor* pActor) {
        return getBvaCtrl(pActor)->getRate();
    }

    f32 getBtpFrameMax(const LiveActor* pActor) {
        return getBtpCtrl(pActor)->getEnd();
    }

    f32 getBvaFrameMax(const LiveActor* pActor) {
        return getBvaCtrl(pActor)->getEnd();
    }

    f32 getBckFrameMax(const LiveActor* pActor) {
        return getBckCtrl(pActor)->getEnd();
    }

    f32 getBtkFrameMax(const LiveActor* pActor) {
        return getBtkCtrl(pActor)->getEnd();
    }

    f32 getBrkFrameMax(const LiveActor* pActor) {
        return getBrkCtrl(pActor)->getEnd();
    }

    void reflectBckCtrlData(LiveActor* pActor, const BckCtrlData& rBck) {
        BckCtrlFunction::reflectBckCtrlData(rBck, pActor->mModelManager->mXanimePlayer);

        if (pActor->mSoundHolder != nullptr) {
            AudAnmSoundObject* pSoundObj = pActor->mSoundHolder->getSoundObj();

            if (pSoundObj->hasAnim()) {
                if (rBck.mRepeatFrame < 0) {
                    pSoundObj->setLoopFrame((f32)rBck.mStartFrame, (f32)rBck.mEndFrame);
                } else {
                    pSoundObj->setLoopFrame((f32)rBck.mRepeatFrame, (f32)rBck.mEndFrame);
                }

                pSoundObj->setStartPos((f32)rBck.mStartFrame);
            }
        }
    }

    void reflectBckCtrlDataAllAnim(LiveActor* pActor, const char* pName, const BckCtrlData& rBck) {
        reflectBckCtrlData(pActor, rBck);

        J3DFrameCtrl* pFrameCtrls[5];
        pFrameCtrls[0] = isExistBtk(pActor, pName) ? getBtkCtrl(pActor) : nullptr;
        pFrameCtrls[1] = isExistBpk(pActor, pName) ? getBpkCtrl(pActor) : nullptr;
        pFrameCtrls[2] = isExistBtp(pActor, pName) ? getBtpCtrl(pActor) : nullptr;
        pFrameCtrls[3] = isExistBrk(pActor, pName) ? getBrkCtrl(pActor) : nullptr;
        pFrameCtrls[4] = isExistBva(pActor, pName) ? getBvaCtrl(pActor) : nullptr;

        for (s32 i = 0; i < 5; i++) {
            if (pFrameCtrls[i] != nullptr) {
                BckCtrlFunction::reflectBckCtrlDataToFrameCtrl(rBck, pFrameCtrls[i]);
            }
        }
    }

    void reflectBckCtrlDataToAllAnim(LiveActor* pActor, const char* pName, const BckCtrlData& rBck) {
        reflectBckCtrlData(pActor, rBck);

        J3DFrameCtrl* pCtrls[5];
        pCtrls[0] = isExistBtk(pActor, pName) ? getBtkCtrl(pActor) : nullptr;
        pCtrls[1] = isExistBpk(pActor, pName) ? getBpkCtrl(pActor) : nullptr;
        pCtrls[2] = isExistBtp(pActor, pName) ? getBtpCtrl(pActor) : nullptr;
        pCtrls[3] = isExistBrk(pActor, pName) ? getBrkCtrl(pActor) : nullptr;
        pCtrls[4] = isExistBva(pActor, pName) ? getBvaCtrl(pActor) : nullptr;

        for (u32 i = 0; i < 5; i++) {
            if (pCtrls[i] != nullptr) {
                BckCtrlFunction::reflectBckCtrlDataToFrameCtrl(rBck, pCtrls[i]);
            }
        }
    }

    void initLightCtrl(LiveActor* pActor) {
        pActor->initActorLightCtrl();
        pActor->mActorLightCtrl->init(-1, false);
    }

    void initLightCtrlForPlayer(LiveActor* pActor) {
        pActor->initActorLightCtrl();
        pActor->mActorLightCtrl->init(-1, false);
        LightFunction::registerPlayerLightCtrl(pActor->mActorLightCtrl);
    }



    void updateLightCtrl(LiveActor* pActor) {
        pActor->mActorLightCtrl->update(false);
    }

    void updateLightCtrlDirect(LiveActor* pActor) {
        pActor->mActorLightCtrl->update(true);
    }

    void loadActorLight(const LiveActor* pActor) {
        pActor->mActorLightCtrl->loadLight();
    }



    const GXColor* getLightAmbientColor(const LiveActor* pActor) {
        return &pActor->mActorLightCtrl->getActorLight()->mColor;
    }

    ActorLightCtrl* getLightCtrl(const LiveActor* pActor) {
        return pActor->mActorLightCtrl;
    }

    bool isStep(const LiveActor* pActor, s32 step) {
        return pActor->getNerveStep() == step;
    }

    bool isFirstStep(const LiveActor* pActor) {
        return isStep(pActor, 0);
    }

    bool isLessStep(const LiveActor* pActor, s32 step) {
        return pActor->getNerveStep() < step;
    }

    bool isLessEqualStep(const LiveActor* pActor, s32 step) {
        return pActor->getNerveStep() <= step;
    }

    bool isGreaterStep(const LiveActor* pActor, s32 step) {
        return pActor->getNerveStep() > step;
    }

    bool isGreaterEqualStep(const LiveActor* pActor, s32 step) {
        return pActor->getNerveStep() >= step;
    }

    bool isIntervalStep(const LiveActor* pActor, s32 step) {
        return pActor->getNerveStep() % step == 0;
    }

    bool isNewNerve(const LiveActor* pActor) {
        return pActor->getNerveStep() < 0;
    }

    f32 calcNerveRate(const LiveActor* pActor, s32 stepMax) {
        return stepMax <= 0 ? 1.0f : clamp(static_cast< f32 >(pActor->getNerveStep()) / stepMax, 0.0f, 1.0f);
    }

    f32 calcNerveRate(const LiveActor* pActor, s32 stepMin, s32 stepMax) {
        return clamp(normalize(pActor->getNerveStep(), stepMin, stepMax), 0.0f, 1.0f);
    }

    f32 calcNerveEaseInRate(const LiveActor* pActor, s32 stepMax) {
        return getEaseInValue(calcNerveRate(pActor, stepMax), 0.0f, 1.0f, 1.0f);
    }

    f32 calcNerveEaseOutRate(const LiveActor* pActor, s32 stepMax) {
        return getEaseOutValue(calcNerveRate(pActor, stepMax), 0.0f, 1.0f, 1.0f);
    }

    f32 calcNerveEaseOutRate(const LiveActor* pActor, s32 stepMin, s32 stepMax) {
        return getEaseOutValue(calcNerveRate(pActor, stepMin, stepMax), 0.0f, 1.0f, 1.0f);
    }

    f32 calcNerveEaseInOutRate(const LiveActor* pActor, s32 stepMax) {
        return getEaseInOutValue(calcNerveRate(pActor, stepMax), 0.0f, 1.0f, 1.0f);
    }

    f32 calcNerveEaseInOutRate(const LiveActor* pActor, s32 stepMin, s32 stepMax) {
        return getEaseInOutValue(calcNerveRate(pActor, stepMin, stepMax), 0.0f, 1.0f, 1.0f);
    }

    f32 calcNerveValue(const LiveActor* pActor, s32 stepMax, f32 valueStart, f32 valueEnd) {
        return getLinerValue(calcNerveRate(pActor, stepMax), valueStart, valueEnd, 1.0f);
    }

    f32 calcNerveValue(const LiveActor* pActor, s32 stepMin, s32 stepMax, f32 valueStart, f32 valueEnd) {
        return getLinerValue(calcNerveRate(pActor, stepMin, stepMax), valueStart, valueEnd, 1.0f);
    }

    f32 calcNerveEaseInValue(const LiveActor* pActor, s32 stepMax, f32 valueStart, f32 valueEnd) {
        return getEaseInValue(calcNerveRate(pActor, stepMax), valueStart, valueEnd, 1.0f);
    }

    f32 calcNerveEaseInValue(const LiveActor* pActor, s32 stepMin, s32 stepMax, f32 valueStart, f32 valueEnd) {
        return getEaseInValue(calcNerveRate(pActor, stepMin, stepMax), valueStart, valueEnd, 1.0f);
    }

    f32 calcNerveEaseOutValue(const LiveActor* pActor, s32 stepMax, f32 valueStart, f32 valueEnd) {
        return getEaseOutValue(calcNerveRate(pActor, stepMax), valueStart, valueEnd, 1.0f);
    }

    f32 calcNerveEaseOutValue(const LiveActor* pActor, s32 stepMin, s32 stepMax, f32 valueStart, f32 valueEnd) {
        return getEaseOutValue(calcNerveRate(pActor, stepMin, stepMax), valueStart, valueEnd, 1.0f);
    }

    f32 calcNerveEaseInOutValue(const LiveActor* pActor, s32 stepMax, f32 valueStart, f32 valueEnd) {
        return getEaseInOutValue(calcNerveRate(pActor, stepMax), valueStart, valueEnd, 1.0f);
    }

    f32 calcNerveEaseInOutValue(const LiveActor* pActor, s32 stepMin, s32 stepMax, f32 valueStart, f32 valueEnd) {
        return getEaseInOutValue(calcNerveRate(pActor, stepMin, stepMax), valueStart, valueEnd, 1.0f);
    }

    f32 calcNerveParabolaValue(const LiveActor* pActor, s32 step) {
        return clamp(getParabolaValue(pActor->getNerveStep(), step), 0.0f, 1.0f);
    }

    void setNerveAtStep(LiveActor* pActor, const Nerve* pNerve, s32 step) {
        if (pActor->getNerveStep() == step) {
            pActor->setNerve(pNerve);
        }
    }

    void setNerveAtBckStopped(LiveActor* pActor, const Nerve* pNerve) {
        if (isBckStopped(pActor)) {
            pActor->setNerve(pNerve);
        }
    }

    bool trySetNerve(LiveActor* pActor, const Nerve* pNerve) {
        if (pActor->isNerve(pNerve)) {
            return false;
        }

        pActor->setNerve(pNerve);

        return true;
    }

    const TVec3f& getGroundNormal(const LiveActor* pActor) {
        return *pActor->mBinder->mGroundInfo.mParentTriangle.getNormal(0);
    }

    const TVec3f& getWallNormal(const LiveActor* pActor) {
        return *pActor->mBinder->mWallInfo.mParentTriangle.getNormal(0);
    }

    const TVec3f& getRoofNormal(const LiveActor* pActor) {
        return *pActor->mBinder->mRoofInfo.mParentTriangle.getNormal(0);
    }

    const TVec3f& getBindedNormal(const LiveActor* pActor) {
        if (isBindedGround(pActor)) {
            return getGroundNormal(pActor);
        }

        if (isBindedWall(pActor)) {
            return getWallNormal(pActor);
        }

        if (isBindedRoof(pActor)) {
            return getRoofNormal(pActor);
        }

        return getGroundNormal(pActor);
    }

    const TVec3f& getBindedHitPos(const LiveActor* pActor) {
        if (isBindedGround(pActor)) {
            return getGroundHitPos(pActor);
        }

        if (isBindedWall(pActor)) {
            return getWallHitPos(pActor);
        }

        if (isBindedRoof(pActor)) {
            return pActor->mBinder->mRoofInfo.mHitPos;
        }

        return getGroundHitPos(pActor);
    }

    const TVec3f& getGroundHitPos(const LiveActor* pActor) {
        return pActor->mBinder->mGroundInfo.mHitPos;
    }

    const TVec3f& getWallHitPos(const LiveActor* pActor) {
        return pActor->mBinder->mWallInfo.mHitPos;
    }



    f32 calcHitPowerToGround(const LiveActor* pActor) {
        if (!isBindedGround(pActor)) {
            return 0.0f;
        }

        f32 dot = pActor->mVelocity.dot(getGroundNormal(pActor));

        if (-dot > 0.0f) {
            return -dot;
        }

        return 0.0f;
    }

    f32 calcHitPowerToWall(const LiveActor* pActor) {
        if (!isBindedWall(pActor)) {
            return 0.0f;
        }

        f32 dot = pActor->mVelocity.dot(getWallNormal(pActor));

        if (-dot > 0.0f) {
            return -dot;
        }

        return 0.0f;
    }

    u32 getBindedPlaneNum(const LiveActor* pActor) {
        return pActor->mBinder->mPlaneNum;
    }

    const TVec3f* getBindedPlaneNormal(const LiveActor* pActor, int planeIndex) {
        return pActor->mBinder->getPlane(planeIndex)->mParentTriangle.getNormal(0);
    }

    HitSensor* getBindedPlaneSensor(const LiveActor* pActor, int planeIndex) {
        return pActor->mBinder->getPlane(planeIndex)->mParentTriangle.mSensor;
    }

    const TVec3f& getBindedFixReactionVector(const LiveActor* pActor) {
        return pActor->mBinder->mFixReactionVector;
    }

    void setBinderOffsetVec(LiveActor* pActor, const TVec3f* pVec, bool b) {
        Binder* binder = pActor->mBinder;
        binder->mOffsetVec = pVec;
        binder->_1EC._4 = b;
    }

    void setBinderRadius(LiveActor* pActor, f32 radius) {
        pActor->mBinder->mRadius = radius;
    }

    void scaleBinderRadius(LiveActor* pActor, f32 scale) {
        pActor->mBinder->mRadius = scale * pActor->mBinder->mRadius;
    }

    f32 getBinderRadius(const LiveActor* pActor) {
        return pActor->mBinder->mRadius;
    }

    void setBinderCollisionPartsFilter(LiveActor* pActor, CollisionPartsFilterBase* pFilter) {
        pActor->mBinder->setCollisionPartsFilter(pFilter);
    }

    void setBinderExceptSensorType(LiveActor* pActor, u32 exceptSensorType) {
        CollisionPartsFilterSensorType* pFilter = new CollisionPartsFilterSensorType(exceptSensorType);
        pActor->mBinder->setCollisionPartsFilter(pFilter);
    }

    void setBinderExceptActor(LiveActor* pActor, const LiveActor* pExcept) {
        CollisionPartsFilterActor* pFilter = new CollisionPartsFilterActor(pExcept);
        pActor->mBinder->setCollisionPartsFilter(pFilter);
    }

    void setBindTriangleFilter(LiveActor* pActor, TriangleFilterBase* pTriangleFilter) {
        pActor->mBinder->setTriangleFilter(pTriangleFilter);
    }

    bool isExistBinder(const LiveActor* pActor) {
        return pActor->mBinder != nullptr;
    }

    void onEntryDrawBuffer(LiveActor* pActor) {
        if (!isNoEntryDrawBuffer(pActor)) {
            return;
        }

        if (!isDead(pActor) && !pActor->mFlags.mIsClipped) {
            connectToDrawTemporarily(pActor);
        }

        pActor->mFlags.mIsNoEntryDrawBuffer = false;
    }

    void offEntryDrawBuffer(LiveActor* pActor) {
        if (isNoEntryDrawBuffer(pActor)) {
            return;
        }

        if (!isDead(pActor) && !pActor->mFlags.mIsClipped) {
            disconnectToDrawTemporarily(pActor);
        }

        pActor->mFlags.mIsNoEntryDrawBuffer = true;
    }

    bool isDead(const LiveActor* pActor) {
        return pActor->mFlags.mIsDead;
    }

    bool isHiddenModel(const LiveActor* pActor) {
        return pActor->mFlags.mIsNoEntryDrawBuffer;
    }

    void showModel(LiveActor* pActor) {
        if (isNoCalcAnim(pActor)) {
            onCalcAnim(pActor);
        }

        if (isNoCalcView(pActor)) {
            pActor->mFlags.mIsNoCalcView = false;
        }

        if (isNoEntryDrawBuffer(pActor)) {
            onEntryDrawBuffer(pActor);
        }
    }

    void hideModel(LiveActor* pActor) {
        if (!isNoCalcAnim(pActor)) {
            offCalcAnim(pActor);
        }

        if (!isNoCalcView(pActor)) {
            pActor->mFlags.mIsNoCalcView = true;
        }

        if (!isNoEntryDrawBuffer(pActor)) {
            offEntryDrawBuffer(pActor);
        }
    }

    void showModelAndEffects(LiveActor* pActor) {
        if (isHiddenModel(pActor)) {
            showModel(pActor);
            onCalcEmitter(pActor);
            onDrawEffect(pActor);
        }
    }

    void hideModelAndEffects(LiveActor* pActor) {
        if (!isHiddenModel(pActor)) {
            hideModel(pActor);
            offCalcEmitter(pActor);
            offDrawEffect(pActor);
        }
    }

    void hideModelAndOnCalcAnim(LiveActor* pActor) {
        hideModel(pActor);
        onCalcAnim(pActor);
    }

    void showModelIfHidden(LiveActor* pActor) {
        if (isHiddenModel(pActor)) {
            showModel(pActor);
        }
    }

    void hideModelIfShown(LiveActor* pActor) {
        if (isHiddenModel(pActor)) {
            return;
        }

        hideModel(pActor);
    }

    void hideModelAndOnCalcAnimIfShown(LiveActor* pActor) {
        if (isHiddenModel(pActor)) {
            return;
        }

        hideModel(pActor);
        onCalcAnim(pActor);
    }

    void stopAnimFrame(LiveActor* pActor) {
        pActor->mFlags.mIsNotReleasedAnimFrame = true;
    }

    void releaseAnimFrame(LiveActor* pActor) {
        pActor->mFlags.mIsNotReleasedAnimFrame = false;
    }

    bool isStopAnimFrame(const LiveActor* pActor) {
        return pActor->mFlags.mIsNotReleasedAnimFrame;
    }

    bool isNoCalcAnim(const LiveActor* pActor) {
        return pActor->mFlags.mIsOnCalcAnim;
    }

    void onCalcAnim(LiveActor* pActor) {
        pActor->mFlags.mIsOnCalcAnim = false;
    }

    void offCalcAnim(LiveActor* pActor) {
        pActor->mFlags.mIsOnCalcAnim = true;
    }

    bool isNoCalcView(const LiveActor* pActor) {
        return pActor->mFlags.mIsNoCalcView;
    }

    bool isNoEntryDrawBuffer(const LiveActor* pActor) {
        return pActor->mFlags.mIsNoEntryDrawBuffer;
    }

    bool isNoBind(const LiveActor* pActor) {
        return pActor->mFlags.mIsOnBind;
    }

    void onBind(LiveActor* pActor) {
        pActor->mFlags.mIsOnBind = false;
    }

    void offBind(LiveActor* pActor) {
        pActor->mFlags.mIsOnBind = true;
    }

    void calcGravity(LiveActor* pActor) {
        TVec3f gravity;
        calcGravityVector(pActor, pActor->mPosition, &gravity, nullptr, 0);

        if (!isNearZero(gravity)) {
            pActor->mGravity.set(gravity);
        }
    }

    void calcGravity(LiveActor* pActor, const TVec3f& rPos) {
        TVec3f gravity;
        calcGravityVector(pActor, rPos, &gravity, nullptr, 0);

        if (!isNearZero(gravity)) {
            pActor->mGravity.set(gravity);
        }
    }

    void calcGravityOrZero(LiveActor* pActor) {
        calcGravityOrZero(pActor, pActor->mPosition);
    }

    void calcGravityOrZero(LiveActor* pActor, const TVec3f& rPos) {
        TVec3f gravity;
        calcGravityVectorOrZero(pActor, rPos, &gravity, nullptr, 0);

        if (!isNearZero(gravity)) {
            pActor->mGravity.set(gravity);
            return;
        }

        if (isBindedGround(pActor)) {
            const TVec3f* normal = pActor->mBinder->mGroundInfo.mParentTriangle.getNormal(0);
            pActor->mGravity.set(-*normal);
        }
    }

    FurMulti* initFur(LiveActor* pActor) {
        return initMultiFur(pActor, -1);
    }

    FurMulti* initFurPlanet(LiveActor* pActor) {
        return initMultiFur(pActor, 3);
    }

    FurMulti* initFurEnemy(LiveActor* pActor) {
        return initMultiFur(pActor, 1);
    }

    FurMulti* initFurPlayer(LiveActor* pActor) {
        return initMultiFur(pActor, 0);
    }

    void initCollisionParts(LiveActor* pActor, const char* pName, HitSensor* pSensor, MtxPtr pMtx) {
        pActor->initActorCollisionParts(pName, pSensor, nullptr, pMtx, false, false);
    }

    void initCollisionPartsAutoEqualScale(LiveActor* pActor, const char* pName, HitSensor* pSensor, MtxPtr pMtx) {
        pActor->initActorCollisionParts(pName, pSensor, nullptr, pMtx, true, false);
    }

    void initCollisionPartsAutoEqualScaleOne(LiveActor* pActor, const char* pName, HitSensor* pSensor, MtxPtr pMtx) {
        pActor->initActorCollisionParts(pName, pSensor, nullptr, pMtx, true, true);
    }

    void initCollisionPartsFromResourceHolder(LiveActor* pActor, const char* pName, HitSensor* pSensor, ResourceHolder* pResHolder, MtxPtr pMtx) {
        pActor->initActorCollisionParts(pName, pSensor, pResHolder, pMtx, false, false);
    }

    CollisionParts* createCollisionPartsFromLiveActor(LiveActor* pActor, const char* pName, HitSensor* pSensor, CollisionScaleType scaleType) {
        TPos3f mtx;
        makeMtxTRS(mtx, pActor);
        return ::createCollisionParts(getResourceHolder(pActor), pName, pSensor, mtx, scaleType, 0);
    }

    CollisionParts* createCollisionPartsFromLiveActor(LiveActor* pActor, const char* pName, HitSensor* pSensor, MtxPtr pMtx,
                                                      CollisionScaleType scaleType) {
        TPos3f mtx;
        mtx.set(pMtx);
        CollisionParts* pParts = ::createCollisionParts(getResourceHolder(pActor), pName, pSensor, mtx, scaleType, 0);
        pParts->mFollowMtx = reinterpret_cast< TPos3f* >(pMtx);
        return pParts;
    }

    CollisionParts* createCollisionPartsFromResourceHolder(ResourceHolder* pResHolder, const char* pName, HitSensor* pSensor, const TPos3f& rMtx,
                                                           MtxPtr pBaseMtx, CollisionScaleType scaleType) {
        CollisionParts* parts = ::createCollisionParts(pResHolder, pName, pSensor, rMtx, scaleType, 0);
        if (pBaseMtx != nullptr) {
            parts->mFollowMtx = reinterpret_cast<TPos3f*>(pBaseMtx);
        }
        return parts;
    }

    CollisionParts* createDashYoshiStepCollisionPartsFromResourceHolder(ResourceHolder* pResourceHolder, const char* pCollisionName,
                                                                        HitSensor* pBodySensor, MtxPtr pMtx, CollisionScaleType collisionScaleType) {
        TPos3f mtx;
        mtx.set(pMtx);

        CollisionParts* parts = createCollisionParts(pResourceHolder, pCollisionName, pBodySensor, mtx, collisionScaleType, 4);
        parts->mFollowMtx = (TPos3f*)pMtx;
        return parts;
    }

    CollisionParts* tryCreateCollisionMoveLimit(LiveActor* pActor, MtxPtr pMtx, HitSensor* pSensor) {
        char kcl[0x80];
        {
            const char* pKclName = "MoveLimit";
            snprintf(kcl, sizeof(kcl), "%s.kcl", pKclName);
        }

        if (!getResourceHolder(pActor)->mFileInfoTable->isExistRes(kcl)) {
            return nullptr;
        }

        TPos3f mtx;
        const char* pCollisionName = "MoveLimit";
        mtx.set(pMtx);
        CollisionParts* pParts = ::createCollisionParts(getResourceHolder(pActor), pCollisionName, pSensor, mtx, CollisionScaleType_Unk2, 3);
        pParts->mFollowMtx = reinterpret_cast< TPos3f* >(pMtx);

        if (pParts != nullptr) {
            validateCollisionParts(pParts);
        }

        return pParts;
    }

    CollisionParts* tryCreateCollisionWaterSurface(LiveActor* pActor, MtxPtr pMtx, HitSensor* pSensor) {
        char kcl[0x80];
        {
            const char* pKclName = "WaterSurface";
            snprintf(kcl, sizeof(kcl), "%s.kcl", pKclName);
        }

        if (!getResourceHolder(pActor)->mFileInfoTable->isExistRes(kcl)) {
            return nullptr;
        }

        TPos3f mtx;
        const char* pCollisionName = "WaterSurface";
        mtx.set(pMtx);
        CollisionParts* pParts = ::createCollisionParts(getResourceHolder(pActor), pCollisionName, pSensor, mtx, CollisionScaleType_Unk2, 2);
        pParts->mFollowMtx = reinterpret_cast< TPos3f* >(pMtx);

        if (pParts != nullptr) {
            validateCollisionParts(pParts);
        }

        return pParts;
    }





    f32 getCollisionBoundingSphereRange(const LiveActor* pActor) {
        return pActor->mCollisionParts->mBoundingSphereRadius;
    }

    bool isValidCollisionParts(LiveActor* pActor) {
        return pActor->mCollisionParts->mIsRegistered;
    }

    void validateCollisionParts(LiveActor* pActor) {
        validateCollisionParts(pActor->mCollisionParts);
        CollisionParts* pParts = pActor->mCollisionParts;

        if (pParts->mFollowMtx != nullptr) {
            pParts->updateBoundingSphereRange();
        } else {
            pParts->updateBoundingSphereRange(pActor->mScale);
        }

        resetAllCollisionMtx(pActor);
    }

    void validateCollisionParts(CollisionParts* pParts) {
        if (!pParts->mIsRegistered) {
            pParts->addToBelongZone();
        }

        pParts->mIsRegistered = true;
        pParts->mNeedsZoneUpdate = true;
    }

    void invalidateCollisionParts(LiveActor* pActor) {
        invalidateCollisionParts(pActor->mCollisionParts);
    }

    void invalidateCollisionParts(CollisionParts* pParts) {
        if (pParts->mIsRegistered) {
            pParts->removeFromBelongZone();
        }

        pParts->mIsRegistered = false;
    }

    void validateExCollisionParts(LiveActor* pActor) {
#ifdef GALAXY_NATIVE
        pActor->mBinder->_1EC._2 = true;
#else
        *(u8*)&pActor->mBinder->_1EC |= 0x20;
#endif
    }

    void invalidateExCollisionParts(LiveActor* pActor) {
#ifdef GALAXY_NATIVE
        pActor->mBinder->_1EC._2 = false;
#else
        *(u8*)&pActor->mBinder->_1EC &= ~0x20;
#endif
    }

    void onUpdateCollisionParts(LiveActor* pActor) {
        CollisionParts* pParts = pActor->mCollisionParts;

        if (!pParts->mIsRegistered) {
            validateCollisionParts(pActor);
        }

        pActor->mCollisionParts->mIsUpdateEnabled = true;
    }


    void onUpdateCollisionPartsOneTimeImmediately(LiveActor* pActor) {
        if (!pActor->mCollisionParts->mIsRegistered) {
            validateCollisionParts(pActor);
        }

        CollisionParts* pParts = pActor->mCollisionParts;
        pParts->mForceUpdateOnce = true;
        makeMtxTR(const_cast< MtxPtr >(pActor->getBaseMtx()), pActor);
        setCollisionMtx(pActor, pActor->mCollisionParts);
        pParts->updateMtx();
    }

    void offUpdateCollisionParts(LiveActor* pActor) {
        CollisionParts* pParts = pActor->mCollisionParts;

        if (!pParts->mIsRegistered) {
            validateCollisionParts(pActor);
        }

        pActor->mCollisionParts->mIsUpdateEnabled = false;
    }

    void resetCollisionPartsBelongZone(LiveActor* pActor) {
        pActor->mCollisionParts->resetBelongZone();
    }

    void resetAllCollisionMtx(LiveActor* pActor) {
        CollisionParts* pParts = pActor->mCollisionParts;

        if (pParts->mFollowMtx != nullptr) {
            pParts->resetAllMtx();
            return;
        }

        TPos3f mtx;
        ::calcCollisionMtx(&mtx, pActor, nullptr);
        pParts->resetAllMtx(mtx);
    }

    u32 getCollisionSensorType(const CollisionParts* pParts) {
        return pParts->mHitSensor->mType;
    }

    DemoObj* createDemoObj(const char* pName, const char* pModelName, MtxPtr pMtx) {
        DemoObj* pDemoObj = new DemoObj(pName, pMtx);
        initActor(pDemoObj, pModelName, false);
        pDemoObj->initWithoutIter();
        return pDemoObj;
    }

    DemoObj* createDemoObjWithAnimArc(const char* pName, const char* pModelName, const char* pAnimArcName, MtxPtr pMtx) {
        DemoObj* pDemoObj = new DemoObj(pName, pMtx);
        initActorNoIter(pDemoObj, pModelName, pAnimArcName, false);
        pDemoObj->initWithoutIter();
        return pDemoObj;
    }

    DemoObj* createDemoObj(const char* pName, const char* pModelName, const char* pFuncName, MtxPtr pMtx) {
        DemoObj* pDemoObj = new DemoObj(pName, pMtx);
        initActor(pDemoObj, pModelName, pFuncName, false);
        pDemoObj->initWithoutIter();
        return pDemoObj;
    }

    DemoObj* createDemoObjWithAnimArc(const char* pName, const char* pModelName, const char* pAnimArcName, const char* pFuncName, MtxPtr pMtx) {
        DemoObj* pDemoObj = new DemoObj(pName, pMtx);
        initActorNoIter(pDemoObj, pModelName, pAnimArcName, pFuncName, false);
        pDemoObj->initWithoutIter();
        return pDemoObj;
    }

    ModelObj* createModelObjMapObjDecoration(const char* pName, const char* pModelName, MtxPtr pMtx) {
        ModelObj* pObj = new ModelObj(pName, pModelName, pMtx, -2, -2, -2, false);
        pObj->initWithoutIter();
        return pObj;
    }

    ModelObj* createModelObjMapObj(const char* pName, const char* pModelName, MtxPtr pMtx) {
        ModelObj* pObj = new ModelObj(pName, pModelName, pMtx, MR::DrawBufferType_MapObj, -2, -2, false);
        pObj->initWithoutIter();
        return pObj;
    }

    ModelObj* createModelObjMapObjStrongLight(const char* pName, const char* pModelName, MtxPtr pMtx) {
        ModelObj* pObj = new ModelObj(pName, pModelName, pMtx, MR::DrawBufferType_MapObjStrongLight, -2, -2, false);
        pObj->initWithoutIter();
        return pObj;
    }

    ModelObj* createModelObjNoSilhouettedMapObj(const char* pName, const char* pModelName, MtxPtr pMtx) {
        ModelObj* pObj = new ModelObj(pName, pModelName, pMtx, MR::DrawBufferType_NoSilhouettedMapObj, -2, -2, false);
        pObj->initWithoutIter();
        return pObj;
    }

    ModelObj* createModelObjNoSilhouettedMapObjStrongLight(const char* pName, const char* pModelName, MtxPtr pMtx) {
        ModelObj* pObj = new ModelObj(pName, pModelName, pMtx, MR::DrawBufferType_NoSilhouettedMapObjStrongLight, -2, -2, false);
        pObj->initWithoutIter();
        return pObj;
    }

    ModelObj* createModelObjIndirectMapObj(const char* pName, const char* pModelName, MtxPtr pMtx) {
        ModelObj* pObj = new ModelObj(pName, pModelName, pMtx, MR::DrawBufferType_IndirectMapObj, -2, -2, false);
        pObj->initWithoutIter();
        return pObj;
    }

    ModelObj* createModelObjPlayerDecoration(const char* pName, const char* pModelName, MtxPtr pMtx) {
        ModelObj* pObj = new ModelObj(pName, pModelName, pMtx, MR::DrawBufferType_PlayerDecoration, -2, -2, false);
        pObj->initWithoutIter();
        return pObj;
    }

    ModelObj* createModelObjEnemy(const char* pName, const char* pModelName, MtxPtr pMtx) {
        ModelObj* pObj = new ModelObj(pName, pModelName, pMtx, MR::DrawBufferType_Enemy, MR::MovementType_EnemyDecoration, -2, false);
        pObj->initWithoutIter();
        return pObj;
    }

    ModelObj* createModelObjNpc(const char* pName, const char* pModelName, MtxPtr pMtx) {
        ModelObj* pObj = new ModelObj(pName, pModelName, pMtx, MR::DrawBufferType_NPC, -2, -2, false);
        pObj->initWithoutIter();
        return pObj;
    }

    ModelObj* createModelObjBloomModel(const char* pName, const char* pModelName, MtxPtr pMtx, bool registerDemo) {
        ModelObj* pObj = new ModelObj(pName, pModelName, pMtx, 0x1F, -2, -2, false);
        pObj->initWithoutIter();

        if (registerDemo) {
            registerDemoSimpleCastAll(pObj);
        }

        return pObj;
    }


    ModelObj* createModelObjBloomModel(const char* pName, const char* pModelName, MtxPtr pMtx) {
        ModelObj* pObj = new ModelObj(pName, pModelName, pMtx, MR::DrawBufferType_BloomModel, -2, -2, false);
        pObj->initWithoutIter();
        registerDemoSimpleCastAll(pObj);
        return pObj;
    }

    PartsModel* createPartsModelMapObj2(LiveActor* pHost, const char* pName, const char* pModelName, MtxPtr pMtx) {
        PartsModel* pModel = new PartsModel(pHost, pName, pModelName, pMtx, -1, false);
        pModel->initWithoutIter();
        return pModel;
    }

    PartsModel* createPartsModelMapObj(LiveActor* pHost, const char* pName, const char* pModelName, MtxPtr pMtx) {
        PartsModel* pModel = new PartsModel(pHost, pName, pModelName, pMtx, MR::DrawBufferType_MapObj, false);
        pModel->initWithoutIter();
        return pModel;
    }

    PartsModel* createPartsModelMapObjStrongLight(LiveActor* pHost, const char* pName, const char* pModelName, MtxPtr pMtx) {
        PartsModel* pModel = new PartsModel(pHost, pName, pModelName, pMtx, MR::DrawBufferType_MapObjStrongLight, false);
        pModel->initWithoutIter();
        return pModel;
    }

    PartsModel* createPartsModelNoSilhouettedMapObj(LiveActor* pHost, const char* pName, const char* pModelName, MtxPtr pMtx) {
        PartsModel* pModel = new PartsModel(pHost, pName, pModelName, pMtx, MR::DrawBufferType_NoSilhouettedMapObj, false);
        pModel->initWithoutIter();
        return pModel;
    }

    PartsModel* createPartsModelEnemy(LiveActor* pHost, const char* pName, const char* pModelName, MtxPtr pMtx) {
        PartsModel* pModel = new PartsModel(pHost, pName, pModelName, pMtx, MR::DrawBufferType_Enemy, false);
        pModel->initWithoutIter();
        return pModel;
    }

    PartsModel* createPartsModelNpc(LiveActor* pHost, const char* pName, const char* pModelName, MtxPtr pMtx) {
        PartsModel* pModel = new PartsModel(pHost, pName, pModelName, pMtx, MR::DrawBufferType_NPC, false);
        pModel->initWithoutIter();
        pModel->mCheckHiddenHost = true;
        return pModel;
    }

    PartsModel* createPartsModelIndirectNpc(LiveActor* pHost, const char* pName, const char* pModelName, MtxPtr pMtx) {
        PartsModel* pModel = new PartsModel(pHost, pName, pModelName, pMtx, MR::DrawBufferType_IndirectNpc, false);
        pModel->initWithoutIter();
        pModel->mCheckHiddenHost = true;
        return pModel;
    }

    PartsModel* createPartsModelEnemyAndFix(LiveActor* pHost, const char* pName, const char* pModelName, MtxPtr pMtx, const TVec3f& rPos,
                                            const TVec3f& rRot, const char* pJointName) {
        PartsModel* pModel = new PartsModel(pHost, pName, pModelName, pMtx, MR::DrawBufferType_Enemy, false);
        pModel->initFixedPosition(rPos, rRot, pJointName);
        pModel->initWithoutIter();
        return pModel;
    }

    PartsModel* createPartsModelNpcAndFix(LiveActor* pHost, const char* pName, const char* pModelName, const char* pJointName) {
        PartsModel* pModel = new PartsModel(pHost, pName, pModelName, nullptr, MR::DrawBufferType_NPC, false);
        pModel->initFixedPosition(pJointName);
        pModel->initWithoutIter();
        pModel->mCheckHiddenHost = true;
        return pModel;
    }

    RailObj* createRailObj(const char* pName, LiveActor* pHost, const JMapInfoIter& rIter, int railIndex) {
        RailObj* pRailObj = new RailObj(pName);
        initChildObj(pRailObj, rIter, railIndex);
        return pRailObj;
    }

    LodCtrl* createLodCtrlNPC(LiveActor* pActor, const JMapInfoIter& rIter) {
        LodCtrl* ctrl = new LodCtrl(pActor, rIter);
        ctrl->createLodModel(0x11, 0x27, -1);
        ctrl->syncMaterialAnimation();
        ctrl->syncJointAnimation();
        ctrl->initLightCtrl();
        ctrl->offSyncShadowHost();
        ctrl->mDistCameraCalc = true;
        return ctrl;
    }

    LodCtrl* createLodCtrlPlanet(LiveActor* pActor, const JMapInfoIter& rIter, f32 farClip, s32 calcAnim) {
        LodCtrl* ctrl = new LodCtrl(pActor, rIter);
        ctrl->createLodModel(6, calcAnim, 1);
        ctrl->setDistanceToMiddleAndLow(5000.0f, 10000.0f);
        ctrl->setFarClipping(farClip);
        return ctrl;
    }




    bool tryKillAndForceDeleteEffectAll(LiveActor* pActor) {
        if (!isDead(pActor)) {
            pActor->makeActorDead();
        }

        return tryForceDeleteEffectAll(pActor);
    }

    void stopSceneAtStep(const LiveActor* pActor, s32 step, s32 sceneStep) {
        if (isStep(pActor, step)) {
            stopScene(sceneStep);
        }
    }


    void tryRumblePadAndCameraDistanceStrong(const LiveActor* pActor, f32 near, f32 middle, f32 far) {
        if (isNearPlayerAnyTime(pActor, near)) {
            tryRumblePadStrong(pActor, WPAD_CHAN0);
            shakeCameraNormal();
        } else if (isNearPlayerAnyTime(pActor, middle)) {
            tryRumblePadMiddle(pActor, WPAD_CHAN0);
            shakeCameraNormalWeak();
        } else if (isNearPlayerAnyTime(pActor, far)) {
            tryRumblePadWeak(pActor, WPAD_CHAN0);
            shakeCameraWeak();
        }
    }

    void tryRumblePadAndCameraDistanceMiddle(const LiveActor* pActor, f32 near, f32 middle, f32 far) {
        if (isNearPlayerAnyTime(pActor, near)) {
            tryRumblePadMiddle(pActor, WPAD_CHAN0);
            shakeCameraNormalWeak();
        } else if (isNearPlayerAnyTime(pActor, middle)) {
            tryRumblePadWeak(pActor, WPAD_CHAN0);
            shakeCameraWeak();
        } else if (isNearPlayerAnyTime(pActor, far)) {
            tryRumblePadVeryWeak(pActor, WPAD_CHAN0);
            shakeCameraVeryWeak();
        }
    }

    void forceClearFootPrint(LiveActor* pActor) {
        pActor->mActionKeeper->mFootPrintCtrl->mHandle->clearForce();
    }

    LiveActor* getLinkedActor(const JMapLinkInfo* pLink) {
        AllLiveActorGroup* pGroup = getAllLiveActorGroup();

        for (s32 i = 0; i < pGroup->getObjNum(); i++) {
            LiveActor* pLinked = pGroup->getActor(i);

            if (pLinked->mLinkInfo == *pLink) {
                return pLinked;
            }
        }

        return nullptr;
    }

    bool isCalcGravity(const LiveActor* pActor) {
        return pActor->mFlags.mIsCalcGravity;
    }

    void onCalcGravity(LiveActor* pActor) {
        if (!isDead(pActor)) {
            calcGravity(pActor);
        }

        pActor->mFlags.mIsCalcGravity = true;
    }

    void offCalcGravity(LiveActor* pActor) {
        pActor->mFlags.mIsCalcGravity = false;
    }

    CollisionParts* getCollisionParts(LiveActor* pActor) {
        return pActor->mCollisionParts;
    }

    void setCollisionMtx(LiveActor* pActor) {
        setCollisionMtx(pActor, getCollisionParts(pActor));
    }

    void setCollisionMtx(LiveActor* pActor, CollisionParts* pParts) {
        if (!pParts->mIsRegistered) {
            return;
        }

        if (pParts->mFollowMtx != nullptr) {
            pParts->setMtx();
            return;
        }

        TPos3f mtx;
        ::calcCollisionMtx(&mtx, pActor, pParts);
        pParts->setMtx(mtx);
    }

    bool isExistCollisionParts(const LiveActor* pActor) {
        return pActor->mCollisionParts != nullptr;
    }
};  // namespace MR
