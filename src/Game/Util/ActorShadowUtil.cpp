#include "Game/LiveActor/ShadowSurfaceCircle.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/ShadowSurfaceOval.hpp"
#include "Game/LiveActor/ShadowVolumeBox.hpp"
#include "Game/LiveActor/ShadowVolumeCylinder.hpp"
#include "Game/LiveActor/ShadowVolumeFlatModel.hpp"
#include "Game/LiveActor/ShadowVolumeLine.hpp"
#include "Game/LiveActor/ShadowVolumeOval.hpp"
#include "Game/LiveActor/ShadowVolumeSphere.hpp"
#include "Game/Map/CollisionParts.hpp"
#include "Game/Util/ActorShadowLocalUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/CollisionPartsFilter.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/StringUtil.hpp"

namespace ActorShadow {
    // NONMATCHING: regalloc (inlining, scheduling, stack slots or equivalent branch layout).
    void initShadowFromCSV(LiveActor* actor, const char* name, bool useSphere) {
        JMapInfo* csv = nullptr;
        if (MR::getResourceHolder(actor))
            csv = MR::tryCreateCsvParser(MR::getResourceHolder(actor), "%s.bcsv", name);
        if (!csv) {
            if (useSphere)
                MR::initShadowVolumeSphere(actor, 100.0f);
            return;
        }
        MR::initShadowControllerList(actor, csv->getNumEntries());
        JMapInfoIter begin(csv, 0);
        JMapInfoIter iter(begin);
        while (iter.isValid()) {
            addShadowFromCSV(actor, iter);
            ++iter.mIndex;
        }
    }
}

namespace MR {
    void initShadowFromCSVWithInitShadowVolumeSphere(LiveActor* actor, const char* name) {
        ActorShadow::initShadowFromCSV(actor, name, true);
    }
    void initShadowFromCSVWithoutInitShadowVolumeSphere(LiveActor* actor, const char* name) {
        ActorShadow::initShadowFromCSV(actor, name, false);
    }
    void initShadowControllerList(LiveActor* actor, u32 count) {
        actor->initShadowControllerList(count);
    }


    // NONMATCHING: regalloc (inlining, scheduling, stack slots or equivalent branch layout).
    void initShadowSurfaceCircle(LiveActor* pActor, f32 radius) {
        pActor->initShadowControllerList(1);
        ShadowController* ctrl = ActorShadow::createShadowControllerSuefaceParam(pActor, "水面丸影");
        ShadowSurfaceCircle* shadow = new ShadowSurfaceCircle();
        ctrl->setShadowDrawer(shadow);
        shadow->setRadius(radius);
    }

    // NONMATCHING: regalloc (inlining, scheduling, stack slots or equivalent branch layout).
    void initShadowVolumeSphere(LiveActor* pActor, f32 radius) {
        pActor->initShadowControllerList(1);
        ShadowController* ctrl = ActorShadow::createShadowControllerVolumeParam(pActor, "ボリューム影(球)");
        ShadowVolumeSphere* shadow = new ShadowVolumeSphere();
        ctrl->setShadowDrawer(shadow);
        shadow->setRadius(radius);
    }

    void initShadowVolumeOval(LiveActor* pActor, const TVec3f& rSize) {
        pActor->initShadowControllerList(1);
        const char* pName = "ボリューム影(楕球)";
        addShadowVolumeOval(pActor, pName, rSize, pActor->getBaseMtx());
    }

    void initShadowVolumeCylinder(LiveActor* pActor, f32 radius) {
        pActor->initShadowControllerList(1);
        addShadowVolumeCylinder(pActor, "ボリューム影(円柱)", radius);
    }






    void addShadowVolumeOval(LiveActor* pActor, const char* pName, const TVec3f& rSize, MtxPtr pMtx) {
        ShadowController* pController = ActorShadow::createShadowControllerVolumeParam(pActor, pName);
        ShadowVolumeOval* pOval = new ShadowVolumeOval();
        pController->setShadowDrawer(pOval);
        pController->setDropPosMtxPtr(pMtx, TVec3f(0.0f, 0.0f, 0.0f));
        pOval->setSize(rSize);
    }

    void addShadowVolumeCylinder(LiveActor* pActor, const char* pName, f32 radius) {
        ShadowController* pController = ActorShadow::createShadowControllerVolumeParam(pActor, pName);
        ShadowVolumeCylinder* pCylinder = new ShadowVolumeCylinder();
        pController->setShadowDrawer(pCylinder);
        pController->offCalcCollision();
        pCylinder->setRadius(radius);
    }

    void addShadowVolumeBox(LiveActor* pActor, const char* pName, const TVec3f& rSize) {
        addShadowVolumeBox(pActor, pName, rSize, pActor->getBaseMtx());
    }

    void addShadowVolumeBox(LiveActor* pActor, const char* pName, const TVec3f& rSize, MtxPtr pMtx) {
        ShadowController* pController = ActorShadow::createShadowControllerVolumeParam(pActor, pName);
        ShadowVolumeBox* pBox = new ShadowVolumeBox();
        pController->setShadowDrawer(pBox);
        pController->setDropPosMtxPtr(pMtx, TVec3f(0.0f, 0.0f, 0.0f));
        pBox->setSize(rSize);
    }

    void addShadowVolumeLine(LiveActor* pActor1, const char* pName1, LiveActor* pActor2, const char* pName2, f32 fromWidth, LiveActor* pActor3,
                             const char* pName3, f32 toWidth) {
        ShadowController* pController = ActorShadow::createShadowControllerVolumeParam(pActor1, pName1);
        ShadowVolumeLine* pLine = new ShadowVolumeLine();
        pController->setShadowDrawer(pLine);
        pController->offCalcCollision();
        pLine->setFromWidth(fromWidth);
        pLine->setToWidth(toWidth);
        pLine->setFromShadowController(ActorShadow::getShadowController(pActor2, pName2));
        pLine->setToShadowController(ActorShadow::getShadowController(pActor3, pName3));
    }


    // NONMATCHING: regalloc (inlining, scheduling, stack slots or equivalent branch layout).
    void addShadowVolumeFlatModel(LiveActor* pActor, const char* pName1, const char* pName2, MtxPtr pMtx) {
        ShadowController* pController = ActorShadow::createShadowControllerVolumeParam(pActor, pName1);
        ShadowVolumeFlatModel* pModel = new ShadowVolumeFlatModel(pName2);
        pController->setShadowDrawer(pModel);
        pController->offCalcCollision();
        pController->setDropPosMtxPtr(pMtx, TVec3f(0.0f, 0.0f, 0.0f));
    }


    void initShadowVolumeFlatModel(LiveActor* pActor, const char* pModelName, MtxPtr pMtx) {
        pActor->initShadowControllerList(1);
        addShadowVolumeFlatModel(pActor, "ボリューム影(板モデル)", pModelName, pMtx);
    }

    void setShadowDropPositionPtr(LiveActor* pActor, const char* pName, const TVec3f* pPos) {
        ActorShadow::getShadowController(pActor, pName)->setDropPosPtr(pPos);
    }

    void setShadowDropPositionMtxPtr(LiveActor* pActor, const char* pName, MtxPtr pMtx, const TVec3f& rPos) {
        ActorShadow::getShadowController(pActor, pName)->setDropPosMtxPtr(pMtx, rPos);
    }

    void setShadowDropPositionAtJoint(LiveActor* pActor, const char* pName1, const char* pName2, const TVec3f& rPos) {
        setShadowDropPositionMtxPtr(pActor, pName1, getJointMtx(pActor, pName2), rPos);
    }

    void setShadowDropDirectionPtr(LiveActor* pActor, const char* pName, const TVec3f* pDir) {
        ActorShadow::getShadowController(pActor, pName)->setDropDirPtr(pDir);
    }

    void setShadowProjectionPtr(LiveActor* pActor, const char* pName, const TVec3f* pPosition, const TVec3f* pDirection) {
        ActorShadow::getShadowController(pActor, pName)->setProjectionPtr(pPosition, pDirection);
    }

    void setShadowDropPosition(LiveActor* pActor, const char* pName, const TVec3f& rPos) {
        ActorShadow::getShadowController(pActor, pName)->setDropPosFix(rPos);
    }

    void setShadowDropDirectionPtrAll(LiveActor* actor, const TVec3f* dir) {
        u32 count = ActorShadow::getShadowControllerCount(actor);
        for (u32 i = 0; i < count; ++i)
            ActorShadow::getShadowController(actor, i)->setDropDirPtr(dir);
    }

    void setShadowDropDirection(LiveActor* pActor, const char* pName, const TVec3f& rDir) {
        ActorShadow::getShadowController(pActor, pName)->setDropDirFix(rDir);
    }

    void setShadowDropLength(LiveActor* pActor, const char* pName, f32 length) {
        ActorShadow::getShadowController(pActor, pName)->setDropLength(length);
    }


    void setShadowProjection(LiveActor* pActor, const char* pName, const TVec3f& rPosition, const TVec3f& rDirection, bool a5) {
        ActorShadow::getShadowController(pActor, pName)->setProjectionFix(rPosition, rDirection, a5);
    }

    void setShadowSurfaceOvalColor(LiveActor* pActor, const char* pName, const Color8 color) {
        ShadowSurfaceOval* pOval = ActorShadow::getShadowSurfaceOval(pActor, pName);
        pOval->setColor(color);
    }

    void setShadowSurfaceOvalAlpha(LiveActor* pActor, const char* pName, u8 alpha) {
        ActorShadow::getShadowSurfaceOval(pActor, pName)->setAlpha(alpha);
    }

    void setShadowVolumeSphereRadius(LiveActor* pActor, const char* pName, f32 radius) {
        ActorShadow::getShadowVolumeSphere(pActor, pName)->setRadius(radius);
    }

    void setShadowVolumeCylinderRadius(LiveActor* pActor, const char* pName, f32 radius) {
        ActorShadow::getShadowVolumeCylinder(pActor, pName)->setRadius(radius);
    }

    void setShadowVolumeBoxSize(LiveActor* pActor, const char* pName, const TVec3f& rSize) {
        ActorShadow::getShadowVolumeBox(pActor, pName)->setSize(rSize);
    }

    void setShadowVolumeStartDropOffset(LiveActor* pActor, const char* pName, f32 offset) {
        ActorShadow::getShadowVolumeDrawer(pActor, pName)->setStartDrawShepeOffset(offset);
    }

    void setShadowVolumeEndDropOffset(LiveActor* pActor, const char* pName, f32 offset) {
        ActorShadow::getShadowVolumeDrawer(pActor, pName)->setEndDrawShepeOffset(offset);
    }

    void onShadowVolumeCutDropLength(LiveActor* pActor, const char* pName) {
        ActorShadow::getShadowVolumeDrawer(pActor, pName)->onCutDropShadow();
    }

    void onCalcShadow(LiveActor* pActor, const char* pName) {
        if (pName != nullptr) {
            ActorShadow::getShadowController(pActor, pName)->onCalcCollision();
        } else {
            onCalcShadowAll(pActor);
        }
    }

    void offCalcShadow(LiveActor* pActor, const char* pName) {
        if (pName != nullptr) {
            ActorShadow::getShadowController(pActor, pName)->offCalcCollision();
        } else {
            offCalcShadowAll(pActor);
        }
    }

    void onCalcShadowOneTime(LiveActor* pActor, const char* pName) {
        if (pName != nullptr) {
            ActorShadow::getShadowController(pActor, pName)->onCalcCollisionOneTime();
        } else {
            onCalcShadowOneTimeAll(pActor);
        }
    }

    void onCalcShadowAll(LiveActor* pActor) {
        u32 count = ActorShadow::getShadowControllerCount(pActor);

        for (u32 i = 0; i < count; i++) {
            ActorShadow::getShadowController(pActor, i)->onCalcCollision();
        }
    }

    void offCalcShadowAll(LiveActor* pActor) {
        u32 count = ActorShadow::getShadowControllerCount(pActor);

        for (u32 i = 0; i < count; i++) {
            ActorShadow::getShadowController(pActor, i)->offCalcCollision();
        }
    }

    void onCalcShadowOneTimeAll(LiveActor* pActor) {
        u32 count = ActorShadow::getShadowControllerCount(pActor);

        for (u32 i = 0; i < count; i++) {
            ActorShadow::getShadowController(pActor, i)->onCalcCollisionOneTime();
        }
    }

    void onCalcShadowDropGravity(LiveActor* pActor, const char* pName) {
        ActorShadow::getShadowController(pActor, pName)->onCalcDropGravity();
    }

    void onCalcShadowDropGravityOneTime(LiveActor* pActor, const char* pName) {
        ActorShadow::getShadowController(pActor, pName)->onCalcDropGravityOneTime();
    }

    void onCalcShadowDropPrivateGravity(LiveActor* pActor, const char* pName) {
        ActorShadow::getShadowController(pActor, pName)->onCalcDropPrivateGravity();
    }

    void onCalcShadowDropPrivateGravityOneTime(LiveActor* pActor, const char* pName) {
        ActorShadow::getShadowController(pActor, pName)->onCalcDropPrivateGravityOneTime();
    }

    void offCalcShadowDropPrivateGravity(LiveActor* pActor, const char* pName) {
        ActorShadow::getShadowController(pActor, pName)->offCalcDropPrivateGravity();
    }

    void excludeCalcShadowToMyCollision(LiveActor* pActor, const char* pName) {
        CollisionParts* pCollisionParts = pActor->mCollisionParts;
        if (!pCollisionParts)
            return;

        if (pName != nullptr) {
            excludeCalcShadowToCollision(pActor, pName, pCollisionParts);
        } else {
            excludeCalcShadowToSensorAll(pActor, pCollisionParts->mHitSensor);
        }
    }

    inline void excludeCalcShadowToCollision(LiveActor* pActor, const char* pName, const HitSensor* pSensor) {
        if (pName == nullptr) {
            excludeCalcShadowToSensorAll(pActor, pSensor);
        } else {
            ShadowController* pShadowCtrl = ActorShadow::getShadowController(pActor, pName);
            pShadowCtrl->setCollisionPartsFilter(new CollisionPartsFilterSensor(pSensor));
        }
    }

    void excludeCalcShadowToCollision(LiveActor* pActor, const char* pName, CollisionParts* pCollision) {
        if (pName != nullptr) {
            const HitSensor* pSensor = pCollision->mHitSensor;
            excludeCalcShadowToCollision(pActor, pName, pSensor);
        } else {
            excludeCalcShadowToSensorAll(pActor, pCollision->mHitSensor);
        }
    }

    void excludeCalcShadowToSensorAll(LiveActor* pActor, const HitSensor* pExcluded) {
        u32 count = ActorShadow::getShadowControllerCount(pActor);

        if (count == 0) {
            return;
        }

        CollisionPartsFilterSensor* pFilter = new CollisionPartsFilterSensor(pExcluded);

        for (u32 i = 0; i < count; i++) {
            ActorShadow::getShadowController(pActor, i)->setCollisionPartsFilter(pFilter);
        }
    }

    void excludeCalcShadowToActorAll(LiveActor* pActor, const LiveActor* pExcluded) {
        u32 count = ActorShadow::getShadowControllerCount(pActor);

        if (count == 0) {
            return;
        }

        CollisionPartsFilterActor* pFilter = new CollisionPartsFilterActor(pExcluded);

        for (u32 i = 0; i < count; i++) {
            ActorShadow::getShadowController(pActor, i)->setCollisionPartsFilter(pFilter);
        }
    }

    bool isExistShadow(const LiveActor* pActor, const char* pName) {
        return ActorShadow::isExistShadowController(pActor, pName);
    }

    void invalidateShadow(LiveActor* pActor, const char* pName) {
        if (pName != nullptr) {
            ActorShadow::getShadowController(pActor, pName)->invalidate();
        } else {
            invalidateShadowAll(pActor);
        }
    }

    void validateShadow(LiveActor* pActor, const char* pName) {
        if (pName != nullptr) {
            ActorShadow::getShadowController(pActor, pName)->validate();
        } else {
            validateShadowAll(pActor);
        }
    }


    void validateShadowGroup(LiveActor* pActor, const char* pName) {
        u32 count = ActorShadow::getShadowControllerCount(pActor);

        for (u32 i = 0; i < count; i++) {
            if (isEqualString(ActorShadow::getShadowController(pActor, i)->mGroupName, pName)) {
                ActorShadow::getShadowController(pActor, i)->validate();
            }
        }
    }

    void invalidateShadowAll(LiveActor* pActor) {
        u32 count = ActorShadow::getShadowControllerCount(pActor);

        for (u32 i = 0; i < count; i++) {
            ActorShadow::getShadowController(pActor, i)->invalidate();
        }
    }

    void validateShadowAll(LiveActor* pActor) {
        u32 count = ActorShadow::getShadowControllerCount(pActor);

        for (u32 i = 0; i < count; i++) {
            ActorShadow::getShadowController(pActor, i)->validate();
        }
    }

    void offShadowVisibleSyncHost(LiveActor* pActor, const char* pName) {
        if (pName != nullptr) {
            ActorShadow::getShadowController(pActor, pName)->offVisibleSyncHost();
        } else {
            offShadowVisibleSyncHostAll(pActor);
        }
    }

    void onShadowVisibleSyncHostAll(LiveActor* pActor) {
        u32 count = ActorShadow::getShadowControllerCount(pActor);

        for (u32 i = 0; i < count; i++) {
            ActorShadow::getShadowController(pActor, i)->onVisibleSyncHost();
        }
    }

    void offShadowVisibleSyncHostAll(LiveActor* pActor) {
        u32 count = ActorShadow::getShadowControllerCount(pActor);

        for (u32 i = 0; i < count; i++) {
            ActorShadow::getShadowController(pActor, i)->offVisibleSyncHost();
        }
    }

    void onShadowFollowHostScale(LiveActor* pActor, const char* pName) {
        if (pName != nullptr) {
            ActorShadow::getShadowController(pActor, pName)->onFollowHostScale();
        } else {
            onShadowFollowHostScaleAll(pActor);
        }
    }

    void onShadowFollowHostScaleAll(LiveActor* pActor) {
        u32 count = ActorShadow::getShadowControllerCount(pActor);

        for (u32 i = 0; i < count; i++) {
            ActorShadow::getShadowController(pActor, i)->onFollowHostScale();
        }
    }

    // NONMATCHING: regalloc (inlining, scheduling, stack slots or equivalent branch layout).
    bool calcClippingRangeIncludeShadow(TVec3f* pVecOutput, f32* pF32Output, const LiveActor* pActor, f32 a4) {
        TVec3f projectionPos;

        if (ActorShadow::getShadowController(pActor, static_cast< const char* >(nullptr))->isProjected()) {
            getShadowProjectionPos(pActor, nullptr, &projectionPos);
            pVecOutput->set((pActor->mPosition + projectionPos) * 0.5f);
            *pF32Output = projectionPos.distance(pActor->mPosition) * 0.5f + a4;
            return true;
        } else {
            pVecOutput->set(pActor->mPosition);
            *pF32Output = a4;
            return false;
        }
    }

    void setClippingRangeIncludeShadow(LiveActor* pActor, TVec3f* pCenter, f32 a3) {
        f32 stack_8 = a3;

        if (calcClippingRangeIncludeShadow(pCenter, &stack_8, pActor, a3)) {
            setClippingTypeSphere(pActor, stack_8, pCenter);
        } else {
            setClippingTypeSphere(pActor, a3);
        }
    }

    bool isExistShadows(const LiveActor* actor) {
        return ActorShadow::getShadowControllerCount(actor) != 0;
    }

    bool isShadowProjected(const LiveActor* pActor, const char* pName) {
        return ActorShadow::getShadowController(pActor, pName)->isProjected();
    }

    void getShadowProjectionPos(const LiveActor* pActor, const char* pName, TVec3f* pResult) {
        ActorShadow::getShadowController(pActor, pName)->getProjectionPos(pResult);
    }

    void getShadowProjectionNormal(const LiveActor* pActor, const char* pName, TVec3f* pResult) {
        ActorShadow::getShadowController(pActor, pName)->getProjectionNormal(pResult);
    }

    f32 getShadowProjectionLength(const LiveActor* pActor, const char* pName) {
        ShadowController* pController = ActorShadow::getShadowController(pActor, pName);

        if (pController->isProjected()) {
            return pController->getProjectionLength();
        }

        return FLOAT_MAX;
    }

    HitSensor* getShadowProjectedSensor(const LiveActor* pActor, const char* pName) {
        return ActorShadow::getShadowController(pActor, pName)->mProjectedSensor;
    }

    bool isShadowProjectedAny(const LiveActor* pActor) {
        u32 count = ActorShadow::getShadowControllerCount(pActor);

        for (u32 i = 0; i < count; i++) {
            if (ActorShadow::getShadowController(pActor, i)->isProjected()) {
                return true;
            }
        }

        return false;
    }

    f32 getShadowNearProjectionLength(const LiveActor* pActor) {
        u32 count = ActorShadow::getShadowControllerCount(pActor);
        f32 result = FLOAT_MAX;

        for (u32 i = 0; i < count; i++) {
            if (ActorShadow::getShadowController(pActor, i)->isProjected()) {
                f32 length = ActorShadow::getShadowController(pActor, i)->getProjectionLength();

                if (length < result) {
                    result = length;
                }
            }
        }

        return result;
    }
};  // namespace MR
#include "Game/Util/ActorShadowLocalUtil.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/ShadowController.hpp"
#include "Game/LiveActor/ShadowSurfaceBox.hpp"
#include "Game/LiveActor/ShadowSurfaceCircle.hpp"
#include "Game/LiveActor/ShadowSurfaceOval.hpp"
#include "Game/LiveActor/ShadowVolumeBox.hpp"
#include "Game/LiveActor/ShadowVolumeCylinder.hpp"
#include "Game/LiveActor/ShadowVolumeDrawer.hpp"
#include "Game/LiveActor/ShadowVolumeFlatModel.hpp"
#include "Game/LiveActor/ShadowVolumeLine.hpp"
#include "Game/LiveActor/ShadowVolumeOval.hpp"
#include "Game/LiveActor/ShadowVolumeOvalPole.hpp"
#include "Game/LiveActor/ShadowVolumeSphere.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/StringUtil.hpp"

namespace {
    static const char* sShadowDefineFileName[] = {"SurfaceCircle",  "SurfaceOval",    "SurfaceBox", "VolumeSphere",    "VolumeOval",
                                                  "VolumeOvalPole", "VolumeCylinder", "VolumeBox",  "VolumeFlatModel", "VolumeLine"};
    template < typename T >
    T* createShadowDrawer(LiveActor* pActor, const JMapInfoIter& rIter, bool surface) {
        ShadowController* pController = ActorShadow::createShadowControlFromCSV(pActor, rIter);
        if (surface) {
            pController->setDropTypeSurface();
        } else {
            pController->setDropTypeNormal();
        }

        T* pDrawer = new T;
        pController->setShadowDrawer(pDrawer);
        return pDrawer;
    }
};  // namespace

namespace ActorShadow {
    // NONMATCHING: regalloc (inlining, scheduling, stack slots or equivalent branch layout).
    u32 getShadowControllerCount(const LiveActor* pActor) {
        if (!pActor->mShadowControllerList)
            return 0;
        return pActor->mShadowControllerList->getControllerCount();
    }

    ShadowController* getShadowController(const LiveActor* pActor, u32 index) {
        return pActor->mShadowControllerList->getController(index);
    }

    ShadowController* getShadowController(const LiveActor* pActor, const char* pName) {
        return pActor->mShadowControllerList->getController(pName);
    }

    bool isExistShadowController(const LiveActor* pActor, const char* pName) {
        if (pActor->mShadowControllerList == nullptr) {
            return false;
        }

        return pActor->mShadowControllerList->getController(pName) != nullptr;
    }

    ShadowController* createShadowControllerVolumeParam(LiveActor* pActor, const char* pName) {
        ShadowController* pController = new ShadowController(pActor, pName);
        pController->setDropPosPtr(&pActor->mPosition);
        pController->setDropDirPtr(&pActor->mGravity);
        pController->setDropLength(1000.0f);
        pController->setDropTypeNormal();
        pActor->mShadowControllerList->addController(pController);

        return pController;
    }

    ShadowController* createShadowControllerSuefaceParam(LiveActor* pActor, const char* pName) {
        ShadowController* pController = new ShadowController(pActor, pName);
        pController->setDropPosPtr(&pActor->mPosition);
        pController->setDropDirPtr(&pActor->mGravity);
        pController->setDropLength(1000.0f);
        pController->setDropTypeSurface();
        pActor->mShadowControllerList->addController(pController);

        return pController;
    }

    ShadowSurfaceOval* getShadowSurfaceOval(const LiveActor* pActor, const char* pName) {
        return static_cast< ShadowSurfaceOval* >(pActor->mShadowControllerList->getController(pName)->getShadowDrawer());
    }

    ShadowVolumeSphere* getShadowVolumeSphere(const LiveActor* pActor, const char* pName) {
        return static_cast< ShadowVolumeSphere* >(pActor->mShadowControllerList->getController(pName)->getShadowDrawer());
    }

    ShadowVolumeCylinder* getShadowVolumeCylinder(const LiveActor* pActor, const char* pName) {
        return static_cast< ShadowVolumeCylinder* >(pActor->mShadowControllerList->getController(pName)->getShadowDrawer());
    }

    ShadowVolumeBox* getShadowVolumeBox(const LiveActor* pActor, const char* pName) {
        return static_cast< ShadowVolumeBox* >(pActor->mShadowControllerList->getController(pName)->getShadowDrawer());
    }

    ShadowVolumeDrawer* getShadowVolumeDrawer(const LiveActor* pActor, const char* pName) {
        return static_cast< ShadowVolumeDrawer* >(pActor->mShadowControllerList->getController(pName)->getShadowDrawer());
    }

    ShadowController* createShadowControlFromCSV(LiveActor* pActor, const JMapInfoIter& rIter) {
        const char* pName = nullptr;
        rIter.getValue("Name", &pName);
        ShadowController* pController = new ShadowController(pActor, pName);

        const char* pGroupName = "";
        rIter.getValue("GroupName", &pGroupName);
        pController->setGroupName(pGroupName);

        pController->setDropDirPtr(&pActor->mGravity);
        setUpShadowControlFromCSV(pController, pActor, rIter);
        pActor->mShadowControllerList->addController(pController);

        return pController;
    }

    bool getJointNameFromCSV(const char** pJointName, const JMapInfoIter& rIter) {
        *pJointName = "";

        if (rIter.getValue< const char* >("Joint", pJointName)) {
            return *pJointName[0] != '\0';
        }

        return false;
    }

    s32 getShadowTypeFromCSV(const JMapInfoIter& rIter) {
        const char* pType = nullptr;

        if (!rIter.getValue< const char* >("Type", &pType)) {
            return -1;
        }

        for (s32 i = 0; i < ARRAY_SIZE(::sShadowDefineFileName); i++) {
            if (MR::isEqualString(pType, ::sShadowDefineFileName[i])) {
                return i;
            }
        }

        return -1;
    }

    ShadowSurfaceCircle* createShadowSurfaceCircleFromCSV(LiveActor* pActor, const JMapInfoIter& rIter) {
        ShadowSurfaceCircle* pShadow = ::createShadowDrawer< ShadowSurfaceCircle >(pActor, rIter, true);

        f32 radius = 100.0f;
        rIter.getValue< f32 >("Radius", &radius);
        pShadow->setRadius(radius);

        return pShadow;
    }

    ShadowSurfaceOval* createShadowSurfaceOvalFromCSV(LiveActor* pActor, const JMapInfoIter& rIter) {
        ShadowSurfaceOval* pShadow = ::createShadowDrawer< ShadowSurfaceOval >(pActor, rIter, true);

        TVec3f size;
        size.set< f32 >(100.0f, 100.0f, 100.0f);
        MR::getJMapInfoV3f(rIter, "Size", &size);
        pShadow->setSize(size);

        return pShadow;
    }

    ShadowSurfaceBox* createShadowSurfaceBoxFromCSV(LiveActor* pActor, const JMapInfoIter& rIter) {
        ShadowSurfaceBox* pShadow = ::createShadowDrawer< ShadowSurfaceBox >(pActor, rIter, true);

        TVec3f size;
        size.set< f32 >(100.0f, 100.0f, 100.0f);
        MR::getJMapInfoV3f(rIter, "Size", &size);
        pShadow->setSize(size);

        return pShadow;
    }

    ShadowVolumeSphere* createShadowVolumeSphereFromCSV(LiveActor* pActor, const JMapInfoIter& rIter) {
        ShadowVolumeSphere* pShadow = ::createShadowDrawer< ShadowVolumeSphere >(pActor, rIter, false);
        setUpShadowVolumeFromCSV(pShadow, rIter);

        f32 radius = 100.0f;
        rIter.getValue< f32 >("Radius", &radius);
        pShadow->setRadius(radius);

        return pShadow;
    }

    ShadowVolumeOval* createShadowVolumeOvalFromCSV(LiveActor* pActor, const JMapInfoIter& rIter) {
        ShadowVolumeOval* pShadow = ::createShadowDrawer< ShadowVolumeOval >(pActor, rIter, false);
        setUpShadowVolumeFromCSV(pShadow, rIter);

        TVec3f size;
        size.set< f32 >(100.0f, 100.0f, 100.0f);
        MR::getJMapInfoV3f(rIter, "Size", &size);
        pShadow->setSize(size);

        return pShadow;
    }

    ShadowVolumeOvalPole* createShadowVolumeOvalPoleFromCSV(LiveActor* pActor, const JMapInfoIter& rIter) {
        ShadowVolumeOvalPole* pShadow = ::createShadowDrawer< ShadowVolumeOvalPole >(pActor, rIter, false);
        setUpShadowVolumeFromCSV(pShadow, rIter);

        TVec3f size;
        size.set< f32 >(100.0f, 100.0f, 100.0f);
        MR::getJMapInfoV3f(rIter, "Size", &size);
        pShadow->setSize(size);

        return pShadow;
    }

    ShadowVolumeCylinder* createShadowVolumeCylinderFromCSV(LiveActor* pActor, const JMapInfoIter& rIter) {
        ShadowVolumeCylinder* pShadow = ::createShadowDrawer< ShadowVolumeCylinder >(pActor, rIter, false);
        setUpShadowVolumeFromCSV(pShadow, rIter);

        f32 radius = 100.0f;
        rIter.getValue< f32 >("Radius", &radius);
        pShadow->setRadius(radius);

        return pShadow;
    }

    ShadowVolumeBox* createShadowVolumeBoxFromCSV(LiveActor* pActor, const JMapInfoIter& rIter) {
        ShadowVolumeBox* pShadow = ::createShadowDrawer< ShadowVolumeBox >(pActor, rIter, false);
        setUpShadowVolumeFromCSV(pShadow, rIter);

        TVec3f size;
        size.set< f32 >(100.0f, 100.0f, 100.0f);
        MR::getJMapInfoV3f(rIter, "Size", &size);
        pShadow->setSize(size);

        return pShadow;
    }

    // NONMATCHING: regalloc (inlining, scheduling, stack slots or equivalent branch layout).
    ShadowVolumeFlatModel* createShadowVolumeFlatModelFromCSV(LiveActor* pActor, const JMapInfoIter& rIter) {
        ShadowVolumeFlatModel* pShadow = ::createShadowDrawer< ShadowVolumeFlatModel >(pActor, rIter, false);
        setUpShadowVolumeFromCSV(pShadow, rIter);

        const char* pModel = nullptr;
        rIter.getValue< const char* >("Model", &pModel);
        pShadow->initModel(pModel);

        return pShadow;
    }

    // NONMATCHING: regalloc (inlining, scheduling, stack slots or equivalent branch layout).
    ShadowVolumeLine* createShadowVolumeLineFromCSV(LiveActor* pActor, const JMapInfoIter& rIter) {
        ShadowVolumeLine* pShadow = ::createShadowDrawer< ShadowVolumeLine >(pActor, rIter, false);
        setUpShadowVolumeFromCSV(pShadow, rIter);

        const char* pLineStart = nullptr;
        rIter.getValue< const char* >("LineStart", &pLineStart);

        const char* pLineEnd = nullptr;
        rIter.getValue< const char* >("LineEnd", &pLineEnd);

        f32 lineStartRadius = 100.0f;
        rIter.getValue< f32 >("LineStartRadius", &lineStartRadius);

        f32 lineEndRadius = 100.0f;
        rIter.getValue< f32 >("LineEndRadius", &lineEndRadius);

        pShadow->setFromWidth(lineStartRadius);
        pShadow->setToWidth(lineEndRadius);

        pShadow->setFromShadowController(pActor->mShadowControllerList->getController(pLineStart));
        pShadow->setToShadowController(pActor->mShadowControllerList->getController(pLineEnd));

        return pShadow;
    }

    // NONMATCHING: regalloc (inlining, scheduling, stack slots or equivalent branch layout).
    void setUpShadowControlBaseMtxFromCSV(ShadowController* pCtrl, LiveActor* pActor, const JMapInfoIter& rIter) {
        TVec3f dropOffset;

        if (!MR::getJMapInfoV3f(rIter, "DropOffset", &dropOffset)) {
            dropOffset.zero();
        }

        const char* pJointName;

        if (getJointNameFromCSV(&pJointName, rIter)) {
            if (MR::isEqualString(pJointName, "::ACTOR_TRANS")) {
                pCtrl->setDropPosPtr(&pActor->mPosition);
                goto rotate;
            }

            if (MR::isEqualString(pJointName, "::BASE_MATRIX")) {
                pCtrl->setDropPosMtxPtr(pActor->getBaseMtx(), dropOffset);
                goto rotate;
            }

            if (MR::isEqualString(pJointName, "::FIX_POSITION")) {
                pCtrl->setDropPosFix(pActor->mPosition);
                goto rotate;
            }

            if (MR::isEqualString(pJointName, "::OTHER_TRANS")) {
                pCtrl->setDropPosPtr(&pActor->mPosition);
                goto rotate;
            }

            if (MR::isEqualString(pJointName, "::OTHER_MATRIX")) {
                pCtrl->setDropPosMtxPtr(pActor->getBaseMtx(), dropOffset);
            } else {
                pCtrl->setDropPosMtxPtr(MR::getJointMtx(pActor, pJointName), dropOffset);
            }
        } else {
            pCtrl->setDropPosPtr(&pActor->mPosition);
        }
    rotate:
        TVec3f localRotate;
        if (!MR::getJMapInfoV3f(rIter, "LocalRotate", &localRotate))
            localRotate.zero();
        pCtrl->setLocalRotate(localRotate);
    }

    void setUpShadowControlFromCSV(ShadowController* pController, LiveActor* pActor, const JMapInfoIter& rIter) {
        f32 dropLength = 1000.0f;
        rIter.getValue("DropLength", &dropLength);
        pController->setDropLength(dropLength);

        f32 dropOffset = 0.0f;
        rIter.getValue("DropStart", &dropOffset);
        pController->setDropStartOffset(dropOffset);

        setUpShadowControlBaseMtxFromCSV(pController, pActor, rIter);
        setUpShadowControlIsFollowScaleFromCSV(pController, rIter);
        setUpShadowControlIsSyncShowFromCSV(pController, rIter);
        setUpShadowControlCalcCollisionFromCSV(pController, rIter);
        setUpShadowControlCalcGravityFromCSV(pController, rIter);
    }

    void setUpShadowControlIsFollowScaleFromCSV(ShadowController* pController, const JMapInfoIter& rIter) {
        bool followScale = true;
        rIter.getValue("FollowScale", &followScale);

        if (followScale) {
            pController->onFollowHostScale();
        } else {
            pController->offFollowHostScale();
        }
    }

    void setUpShadowControlIsSyncShowFromCSV(ShadowController* pController, const JMapInfoIter& rIter) {
        bool syncShow = true;
        rIter.getValue("SyncShow", &syncShow);

        if (syncShow) {
            pController->onVisibleSyncHost();
        } else {
            pController->offVisibleSyncHost();
        }
    }

    void setUpShadowControlCalcCollisionFromCSV(ShadowController* pCtrl, const JMapInfoIter& rIter) {
        s32 collision = 0;
        rIter.getValue< s32 >("Collision", &collision);

        switch (collision) {
        case 0:
            pCtrl->offCalcCollision();
            break;
        case 1:
            pCtrl->onCalcCollision();
            break;
        case 2:
            pCtrl->onCalcCollisionOneTime();
            break;
        }
    }

    void setUpShadowControlCalcGravityFromCSV(ShadowController* pCtrl, const JMapInfoIter& rIter) {
        s32 gravity = 0;
        rIter.getValue< s32 >("Gravity", &gravity);

        switch (gravity) {
        case 0:
            pCtrl->offCalcDropGravity();
            break;
        case 1:
            pCtrl->onCalcDropGravity();
            break;
        case 2:
            pCtrl->onCalcDropGravityOneTime();
            break;
        case 3:
            pCtrl->offCalcDropPrivateGravity();
            break;
        case 4:
            pCtrl->onCalcDropPrivateGravity();
            break;
        case 5:
            pCtrl->onCalcDropPrivateGravityOneTime();
            break;
        }
    }

    // NONMATCHING: regalloc (inlining, scheduling, stack slots or equivalent branch layout).
    void setUpShadowVolumeFromCSV(ShadowVolumeDrawer* pDrawer, const JMapInfoIter& rIter) {
        f32 volumeStart = 100.0f;
        f32 volumeEnd = 100.0f;
        s32 volumeCut = 0;
        rIter.getValue< f32 >("VolumeStart", &volumeStart);
        rIter.getValue< f32 >("VolumeEnd", &volumeEnd);
        rIter.getValue< s32 >("VolumeCut", &volumeCut);
        pDrawer->setStartDrawShepeOffset(volumeStart);
        pDrawer->setEndDrawShepeOffset(volumeEnd);

        if (volumeCut != 0) {
            pDrawer->onCutDropShadow();
        } else {
            pDrawer->offCutDropShadow();
        }
    }

    void addShadowFromCSV(LiveActor* pActor, const JMapInfoIter& rIter) {
        s32 type = getShadowTypeFromCSV(rIter);

        switch (type) {
        case 0:
            createShadowSurfaceCircleFromCSV(pActor, rIter);
            break;
        case 1:
            createShadowSurfaceOvalFromCSV(pActor, rIter);
            break;
        case 2:
            createShadowSurfaceBoxFromCSV(pActor, rIter);
            break;
        case 3:
            createShadowVolumeSphereFromCSV(pActor, rIter);
            break;
        case 4:
            createShadowVolumeOvalFromCSV(pActor, rIter);
            break;
        case 5:
            createShadowVolumeOvalPoleFromCSV(pActor, rIter);
            break;
        case 6:
            createShadowVolumeCylinderFromCSV(pActor, rIter);
            break;
        case 7:
            createShadowVolumeBoxFromCSV(pActor, rIter);
            break;
        case 8:
            createShadowVolumeFlatModelFromCSV(pActor, rIter);
            break;
        case 9:
            createShadowVolumeLineFromCSV(pActor, rIter);
            break;
        }
    }
};  // namespace ActorShadow
