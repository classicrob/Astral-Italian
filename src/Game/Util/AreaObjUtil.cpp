#include "Game/Util/AreaObjUtil.hpp"
#include "Game/AreaObj/AreaForm.hpp"
#include "Game/AreaObj/AreaObj.hpp"
#include "Game/AreaObj/AreaObjContainer.hpp"
#include "Game/AreaObj/RestartCube.hpp"
#include "Game/AreaObj/WaterArea.hpp"
#include "Game/Map/OceanBowl.hpp"
#include "Game/Map/OceanRing.hpp"
#include "Game/Map/OceanSphere.hpp"
#include "Game/Map/WaterStream.hpp"
#include "Game/Map/WaterAreaHolder.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"



namespace MR {
    bool isAreaFormCube(const AreaObj* pArea) {
        return pArea->mFormType == AreaForm::Type_Cube1 || pArea->mFormType == AreaForm::Type_Cube2;
    }

    bool isAreaFormCylinder(const AreaObj* pArea) {
        return pArea->mFormType == AreaForm::Type_Cylinder;
    }

    inline AreaObj* getAreaIn(const char* pName, const TVec3f& rPos) {
        return getAreaObjContainer()->getAreaObj(pName, rPos);
    }

    AreaObjMgr* getAreaObjManager(const char* pMgrName) {
        return MR::getAreaObjContainer()->getManager2(pMgrName);
    }

    AreaObj* getAreaObj(const char* pAreaName, const TVec3f& rVec) {
        return MR::getAreaObjContainer()->getAreaObj(pAreaName, rVec);
    }

    AreaObj* getAreaObjAtPlayerPos(const char* pAreaName) {
        AreaObjContainer* container = MR::getAreaObjContainer();
        return container->getAreaObj(pAreaName, *MR::getPlayerPos());
    }

    bool isInAreaObj(const char* pAreaName, const TVec3f& rVec) {
        return MR::getAreaObjContainer()->getAreaObj(pAreaName, rVec);
    }

    bool isExistAreaObj(const char* name) {
        return getAreaObjContainer()->isExistAreaObj(name);
    }

    s32 getAreaObjArg(const AreaObj* pObj, s32 which) {
        switch (which) {
        case 0:
            return pObj->mObjArg0;
        case 1:
            return pObj->mObjArg1;
        case 2:
            return pObj->mObjArg2;
        case 3:
            return pObj->mObjArg3;
        case 4:
            return pObj->mObjArg4;
        case 5:
            return pObj->mObjArg5;
        case 6:
            return pObj->mObjArg6;
        case 7:
            return pObj->mObjArg7;
        default:
            return -1;
        }
    }

#pragma push
#pragma opt_propagation off
    // NONMATCHING: equivalent switch dispatch lowering
    bool getWaterAreaInfo(WaterInfo* pInfo, const TVec3f& rPos, const TVec3f& rGravity, bool skipConnectedArea) {
        if (pInfo->mOceanSphere) return pInfo->mOceanSphere->calcWaterInfo(rPos, rGravity, pInfo);
        if (pInfo->mWaterStream) return pInfo->mWaterStream->calcWaterInfo(rPos, rGravity, pInfo);

        const WaterArea* pArea = pInfo->mWaterArea;
        f32 depth;

        const s32 form = pArea->mFormType;
        if (form == AreaForm::Type_Cylinder) goto waterCylinder;
        if (AreaForm::Type_Cylinder <= form) goto waterUpperForm;
        if (AreaForm::Type_Sphere <= form) goto waterSphere;
        if (AreaForm::Type_Cube1 <= form) goto waterCube;
        goto waterResolved;
waterUpperForm:
        if (AreaForm::Type_Bowl + 1 <= form) goto waterResolved;
        goto waterBowl;
waterCube: {
            TPos3f worldMtx;
            TRot3f rotationMtx;
            TVec3f up;
            TVec3f top;
            TVec3f localPos;
            pArea->getForm< AreaFormCube >()->calcLocalPos(&localPos, rPos);
            up.set< f32 >(0.0f, 1.0f, 0.0f);
            top = TVec3f(0.0f, pArea->getForm< AreaFormCube >()->mBounding.f.y, 0.0f);
            TVec3f fromTop = localPos - top;
            TVec3f fromBottom(localPos);
            pInfo->mCamWaterDepth = MR::abs(MR::vecKillElement(fromTop, up, &fromTop));
            pInfo->mBottomDepth = MR::abs(MR::vecKillElement(fromBottom, up, &fromBottom));
            pInfo->mSurfacePos.set(top + fromTop);
            pArea->getForm< AreaFormCube >()->calcWorldMtx(&worldMtx);
            PSMTXMultVec(worldMtx, &pInfo->mSurfacePos, &pInfo->mSurfacePos);
            TVec3f rotation;
            pArea->getForm< AreaFormCube >()->calcWorldRotate(&rotation);
            MR::makeMtxRotate(rotationMtx, rotation.x, rotation.y, rotation.z);
            rotationMtx.getYDir2(up);
            pInfo->mSurfaceNormal.set(up);

            if (pArea->mObjArg0 > 0) {
                TVec3f streamDir(0.0f, 0.0f, 1.0f);
                MR::calcCubeAxisZ(pArea, &streamDir);
                pInfo->mStreamVec.set(streamDir);
                pInfo->mStreamVec.scale(pArea->mObjArg0);
            }

            goto waterResolved;
        }
waterSphere: {
            TVec3f center(0.0f, 0.0f, 0.0f);
            pArea->getForm< AreaFormSphere >()->calcPos(&center);
            f32 radius = pArea->getForm< AreaFormSphere >()->mRadius;
            TVec3f radial = rPos - center;
            f32 height = MR::vecKillElement(radial, -rGravity, &radial);
            f32 distanceRate = radial.length() / radius;
            f32 angle = distanceRate;
            angle *= PI;
            f32 theta = angle;
            theta *= 0.5f;
            f32 surfaceHeight = radius * MR::cos(theta);
            pInfo->mCamWaterDepth = surfaceHeight - height;
            pInfo->mBottomDepth = surfaceHeight + height;
            TVec3f normal = rPos - center;
            MR::normalizeOrZero(&normal);
            pInfo->mSurfaceNormal.set(normal);
            pInfo->mSurfacePos.set(center + normal * radius);
            goto waterResolved;
        }
waterBowl: {
            depth = 0.0f;
            goto waterResolved;
        }
waterCylinder: {
            TVec3f center(0.0f, 0.0f, 0.0f);
            TVec3f up(0.0f, 0.0f, 1.0f);
            pArea->getForm< AreaFormCylinder >()->calcPos(&center);
            pArea->getForm< AreaFormCylinder >()->calcUpVec(&up);

            if (MR::abs(up.dot(rGravity)) > 0.707f) {
                TVec3f radial;
                f32 height = MR::vecKillElement(rPos - center, up, &radial);
                depth = pArea->getForm< AreaFormCylinder >()->mHeight - height;
                pInfo->mBottomDepth = height;
                pInfo->mCamWaterDepth = depth;
                pInfo->mSurfacePos.set(rPos + up * depth);
            } else {
                TVec3f radial;
                MR::vecKillElement(rPos - center, up, &radial);
                f32 distance = radial.length();
                f32 radius = pArea->getForm< AreaFormCylinder >()->mRadius;
                depth = radius - distance;
                pInfo->mCamWaterDepth = depth;
                pInfo->mBottomDepth = radius + distance;
                pInfo->mSurfacePos.set(rPos - rGravity * depth);
            }

            pInfo->mSurfaceNormal.set(-rGravity);

            if (pArea->mObjArg0 > 0) {
                TVec3f streamDir(0.0f, 0.0f, 0.0f);
                pArea->getForm< AreaFormCylinder >()->calcUpVec(&streamDir);
                pInfo->mStreamVec.set(streamDir);
                pInfo->mStreamVec.scale(pArea->mObjArg0);
            }

            goto waterResolved;
        }
waterResolved:

        if (!skipConnectedArea) {
            TVec3f checkPos(pInfo->getSurfacePos() - rGravity * 5.0f);
            WaterInfo nextInfo;
            MR::getWaterAreaObj(&nextInfo, checkPos);

            if (nextInfo.isInWater()) {
                MR::getWaterAreaInfo(&nextInfo, checkPos, rGravity, true);
                f32 nextDepth = nextInfo.mCamWaterDepth;
                pInfo->mCamWaterDepth = pInfo->mCamWaterDepth + nextDepth;
                pInfo->mSurfacePos -= rGravity * nextInfo.mCamWaterDepth;
            }
        }

        return pArea->isInVolume(rPos);
    }
#pragma pop

    void requestMovementOnAreas(const char* pMgrName) {
        AreaObjMgr* pMgr = MR::getAreaObjContainer()->getManager(pMgrName);

        if (pMgr != nullptr) {
            pMgr->requestMovementOnAll();
        }
    }

    void requestMovementOnSoundAreas() {
        MR::requestMovementOnAreas("SoundPlayArea");
        MR::requestMovementOnAreas("SoundEmitterCube");
        MR::requestMovementOnAreas("SoundEmitterSphere");
        MR::requestMovementOnAreas("SoundEmitterPoint");
    }

    bool hasNoWaterAreaCube(const WaterInfo* pInfo) {
        if (pInfo->mWaterArea == nullptr) {
            return true;
        }

        return pInfo->mWaterArea->mFormType != AreaForm::Type_Cube1 && pInfo->mWaterArea->mFormType != AreaForm::Type_Cube2;
    }

    bool intersectsWaterAreaInfo(const WaterInfo* a, const WaterInfo* b) {
        if (a->mWaterArea) return a->mWaterArea == b->mWaterArea;
        if (a->mOceanBowl) return a->mOceanBowl == b->mOceanBowl;
        if (a->mOceanRing) return a->mOceanRing == b->mOceanRing;
        if (a->mOceanSphere) return a->mOceanSphere == b->mOceanSphere;
        if (a->mWaterStream) return a->mWaterStream == b->mWaterStream;
        return false;
    }

    bool func_80062020(const WaterInfo* info, TVec3f* a, TVec3f* b) {
        if (hasNoWaterAreaCube(info)) return true;
        const WaterArea* area = info->mWaterArea;
        if (static_cast<u32>(area->mFormType) <= 1) {
            TVec3f start;
            TVec3f end;
            area->getForm<AreaFormCube>()->calcLocalPos(&start, *a);
            area->getForm<AreaFormCube>()->calcLocalPos(&end, *b);
            const AreaFormCube* cube = area->getForm<AreaFormCube>();
            if (end.y == start.y) return false;
            f32 rate = (cube->mBounding.f.y - start.y) / (end.y - start.y);
            if (rate < 0.0f || rate > 1.0f) return false;
            TVec3f hit = (end - start).scaleInline(rate) + start;
            return hit.x <= cube->mBounding.f.x && hit.x >= cube->mBounding.i.x &&
                   hit.z <= cube->mBounding.f.z && hit.z >= cube->mBounding.i.z;
        }
        return true;
    }

    bool func_800621A0(const WaterInfo* info, TVec3f* a, TVec3f* b) {
        if (hasNoWaterAreaCube(info)) return false;
        const WaterArea* area = info->mWaterArea;
        if (static_cast<u32>(area->mFormType) <= 1) {
            TVec3f start;
            TVec3f end;
            area->getForm<AreaFormCube>()->calcLocalPos(&start, *a);
            area->getForm<AreaFormCube>()->calcLocalPos(&end, *b);
            const AreaFormCube* cube = area->getForm<AreaFormCube>();
            if (end.y == start.y) return false;
            f32 rate = (cube->mBounding.i.y - start.y) / (end.y - start.y);
            if (rate < 0.0f || rate > 1.0f) return false;
            TVec3f hit = (end - start).scaleInline(rate) + start;
            return hit.x <= cube->mBounding.f.x && hit.x >= cube->mBounding.i.x &&
                   hit.z <= cube->mBounding.f.z && hit.z >= cube->mBounding.i.z;
        }
        return false;
    }

    inline TVec3f calcWaterRayPosition(const TVec3f& start, const TVec3f& direction, f32 rate) {
        TVec3f scaled(direction);
        scaled.scale(rate);
        TVec3f hit(start);
        hit += scaled;
        return hit;
    }

    bool func_80062320(const WaterArea* area, TVec3f* a, TVec3f* b) {
        if (static_cast<u32>(area->mFormType) <= 1) {
            TVec3f start;
            TVec3f end;
            area->getForm<AreaFormCube>()->calcLocalPos(&start, *a);
            area->getForm<AreaFormCube>()->calcLocalRotate(&end, *b);
            normalize(&end);
            const AreaFormCube* cube = area->getForm<AreaFormCube>();
            TVec3f up(0.0f, 1.0f, 0.0f);
            f32 dot = up.dot(end);
            if (0.0f == dot) return false;
            f32 rate = (cube->mBounding.f.y - start.y) / dot;
            if (rate < 0.0f) return false;
            TVec3f hit = calcWaterRayPosition(start, end, rate);
            return hit.x <= cube->mBounding.f.x && hit.x >= cube->mBounding.i.x &&
                   hit.z <= cube->mBounding.f.z && hit.z >= cube->mBounding.i.z;
        }
        return true;
    }

    bool func_80062480(const WaterArea* area, TVec3f* a, TVec3f* b) {
        if (static_cast<u32>(area->mFormType) <= 1) {
            TVec3f start;
            TVec3f end;
            area->getForm<AreaFormCube>()->calcLocalPos(&start, *a);
            area->getForm<AreaFormCube>()->calcLocalRotate(&end, *b);
            normalize(&end);
            const AreaFormCube* cube = area->getForm<AreaFormCube>();
            TVec3f up(0.0f, 1.0f, 0.0f);
            f32 dot = up.dot(end);
            if (0.0f == dot) return false;
            f32 rate = (cube->mBounding.i.y - start.y) / dot;
            if (rate < 0.0f) return false;
            TVec3f hit = calcWaterRayPosition(start, end, rate);
            return hit.x <= cube->mBounding.f.x && hit.x >= cube->mBounding.i.x &&
                   hit.z <= cube->mBounding.f.z && hit.z >= cube->mBounding.i.z;
        }
        return false;
    }

    bool calcWhirlPoolAccelInfo(const TVec3f& rVec, TVec3f* pVec) {
        return false;
    }

    void calcCubePos(const AreaObj* pArea, TVec3f* pPos) {
        pArea->getForm< AreaFormCube >()->calcWorldPos(pPos);
    }

    void calcCubeRotate(const AreaObj* pArea, TVec3f* pPos) {
        pArea->getForm< AreaFormCube >()->calcWorldRotate(pPos);
    }

    void calcCubeAxisZ(const AreaObj* pArea, TVec3f* pPos) {
        TVec3f rotate;
        pArea->getForm< AreaFormCube >()->calcWorldRotate(&rotate);
        TRot3f rotation;
        MR::makeMtxRotate(rotation, rotate.x, rotate.y, rotate.z);
        rotation.getZDir2(*pPos);
    }

    void calcCubeWorldBox(TDirBox3f* pBox, const AreaObj* pArea) {
        pArea->getForm< AreaFormCube >()->calcWorldBox(pBox);
    }

    TBox3f* getCubeLocalBox(const AreaObj* pArea) {
        return &pArea->getForm< AreaFormCube >()->mBounding;
    }

    void calcCubeLocalPos(TVec3f* pVec, const AreaObj* pArea, const TVec3f& rVec) {
        pArea->getForm< AreaFormCube >()->calcLocalPos(pVec, rVec);
    }

    void calcSpherePos(TVec3f* pVec, const AreaObj* pArea) {
        pArea->getForm< AreaFormSphere >()->calcPos(pVec);
    }

    f32 getSphereRadius(const AreaObj* pArea) {
        return pArea->getForm< AreaFormSphere >()->mRadius;
    }

    void calcSphereFrontVec(TVec3f* pVec, const AreaObj* pArea, const TVec3f& rPos) {
        TVec3f center(0.0f);
        pArea->getForm< AreaFormSphere >()->calcPos(&center);

        TVec3f dir(center);
        dir -= rPos;

        if (MR::normalizeOrZero(pVec, dir)) {
            pArea->getForm< AreaFormSphere >()->calcFrontVec(pVec);
            pVec->negate();
        }
    }

    void calcCylinderPos(TVec3f* pVec, const AreaObj* pArea) {
        pArea->getForm< AreaFormCylinder >()->calcPos(pVec);
    }

    void calcCylinderFrontVec(TVec3f* pVec, const AreaObj* pArea, const TVec3f& rPos) {
        AreaFormCylinder* pForm = pArea->getForm< AreaFormCylinder >();
        TVec3f center(0.0f);
        TVec3f up(0.0f);
        pForm->calcCenterPos(&center);
        pForm->calcUpVec(&up);

        TVec3f dir(center);
        dir -= rPos;

        JMAVECScaleAdd(&up, &dir, pVec, -up.dot(dir));

        if (MR::normalizeOrZero(pVec)) {
            MR::makeAxisVerticalZX(pVec, up);
        }
    }

    void calcCylinderCenterPos(TVec3f* pVec, const AreaObj* pArea) {
        pArea->getForm< AreaFormCylinder >()->calcCenterPos(pVec);
    }

    void calcCylinderUpVec(TVec3f* pVec, const AreaObj* pArea) {
        pArea->getForm< AreaFormCylinder >()->calcUpVec(pVec);
    }

    f32 getCylinderRadius(const AreaObj* pArea) {
        return pArea->getForm< AreaFormCylinder >()->mRadius;
    }

    void tryToUpdatePlayerRestartIdInfo(const TVec3f& rVec) {
    }

    bool getWaterAreaObj(WaterInfo* pInfo, const TVec3f& rPos) {
        pInfo->clear();

        if (WaterAreaFunction::tryInOceanArea(rPos, pInfo)) {
            return true;
        }

        WaterArea* pArea = getAreaObj< WaterArea >("Water", rPos);

        if (pArea != nullptr) {
            pInfo->mWaterArea = pArea;
            return true;
        }

        return false;
    }

    bool calcAreaMoveVelocity(TVec3f* pVelocity, const TVec3f& rPos) {
        AreaObj* pArea = MR::getAreaObj("AreaMoveSphere", rPos);

        if (pArea == nullptr) {
            pVelocity->zero();
            return false;
        }

        TVec3f center;
        pArea->getForm< AreaFormSphere >()->calcPos(&center);
        TVec3f up;
        pArea->getForm< AreaFormSphere >()->calcUpVec(&up);
        TVec3f radial = center - rPos;
        MR::normalizeOrZero(&radial);
        MR::vecKillElement(up, radial, &up);
        MR::normalizeOrZero(&up);
        s32 speed = MR::getAreaObjArg(pArea, 0);

        if (speed == -1) {
            speed = 10;
        }

        pVelocity->set(up * speed);
        return true;
    }

    AreaObj* getCurrentAstroOverlookAreaObj() {
        return getAreaIn("AstroOverlookArea", *MR::getPlayerPos());
    }

    void calcHemisphereFrontVec(TVec3f* pOut, const AreaObj* pArea, const TVec3f& rPos) {
        TVec3f center(0.0f);
        center.set(pArea->getForm<AreaFormBowl>()->mTranslation);
        TVec3f dir(center);
        dir -= rPos;
        normalizeOrZero(pOut, dir);
    }

    bool isInPlaneModeCube(const TVec3f& rPos) {
        return getAreaIn("PlaneModeCube", rPos) != NULL;
    }

    bool isInTowerModeCylinder(const TVec3f& rPos, TVec3f* pCenter, TVec3f* pUp, f32* pRadius) {
        AreaObj* area = getAreaIn("TowerModeCylinder", rPos);
        if (!area)
            return false;
        if (pCenter)
            area->getForm<AreaFormCylinder>()->calcCenterPos(pCenter);
        if (pUp)
            area->getForm<AreaFormCylinder>()->calcUpVec(pUp);
        if (pRadius)
            *pRadius = area->getForm<AreaFormCylinder>()->mRadius;
        return true;
    }

    void getAreaFrontVec(TVec3f* pOut, const AreaObj* pArea, const TVec3f& rPos) {
        if (isAreaFormCube(pArea))
            calcCubeAxisZ(pArea, pOut);
        else if (pArea->mFormType == AreaForm::Type_Sphere) {
            calcSphereFrontVec(pOut, pArea, rPos);
            pOut->negate();
        } else if (pArea->mFormType == AreaForm::Type_Cylinder) {
            calcCylinderFrontVec(pOut, pArea, rPos);
            pOut->negate();
        } else if (pArea->mFormType == AreaForm::Type_Bowl) {
            calcHemisphereFrontVec(pOut, pArea, rPos);
            pOut->negate();
        }
    }

    void getAreaCenterPos(TVec3f* pOut, const AreaObj* pArea) {
        if (isAreaFormCube(pArea))
            pArea->getForm<AreaFormCube>()->calcWorldPos(pOut);
        else if (pArea->mFormType == AreaForm::Type_Sphere)
            pArea->getForm<AreaFormSphere>()->calcPos(pOut);
        else if (pArea->mFormType == AreaForm::Type_Cylinder)
            pArea->getForm<AreaFormCylinder>()->calcCenterPos(pOut);
        else if (pArea->mFormType == AreaForm::Type_Bowl)
            pOut->set(pArea->getForm<AreaFormBowl>()->mTranslation);
    }
};  // namespace MR
