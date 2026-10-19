#define JGEOMETRY_VEC3_MI_NOINLINE
#include <revolution.h>
#include "Game/Util.hpp"

#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/MapObj/MapPartsRailGuideHolder.hpp"
#include "Game/MapObj/MapPartsRailMover.hpp"
#include "Game/MapObj/MapPartsRailPointPassChecker.hpp"
#include "Game/MapObj/MapPartsRailRotator.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapPartsUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/RailUtil.hpp"
#include <cstdio>

template TVec3f TVec3f::operator-(const TVec3f&) const;

void MapPartsUtil_FORCE_MATCH_SDATA2() {
    0.0f;
    0.5f;
}

void MapPartsUtil_DUMMY() {
    TVec3f a(0.0f, 0.0f, 0.0f);
    TVec3f b;
    a.sub(b);
}

namespace {
    const char* cFollowJointName = "Move";

};  // namespace

namespace MR {
    HitSensor* getBodySensor(LiveActor* pActor) {
        return pActor->getSensor("Body");
    }

    void setBodySensorType(LiveActor* pActor, u32 type) {
        pActor->getSensor("Body")->setType(type);
    }






    inline bool tryStartMapPartsRotateAtPoint(LiveActor* pActor, MapPartsRailMover*, MapPartsRailRotator* pRotator) {
        if (!pRotator) {
            return false;
        }

        s32 point = getCurrentRailPointNo(pActor);
        if (!pRotator->hasRotation(point)) {
            return false;
        }

        if (pRotator->hasRotationBetweenPoints(point)) {
            return false;
        }

        pRotator->rotateAtPoint(point);
        return true;
    }

    inline bool tryEndMapPartsRotateAtPoint(LiveActor*, MapPartsRailMover* pMover, MapPartsRailRotator* pRotator) {
        if (!pMover) {
            return false;
        }

        if (!pRotator) {
            return false;
        }

        pMover->endRotateAtPoint();
        return true;
    }

    inline bool tryStartMapPartsRotateBetweenPoints(LiveActor* pActor, MapPartsRailMover* pMover, MapPartsRailRotator* pRotator) {
        if (!pMover) {
            return false;
        }

        if (!pRotator) {
            return false;
        }

        if (!pRotator->hasRotationBetweenPoints(getCurrentRailPointNo(pActor))) {
            return false;
        }

        f32 time = 0.0f;
        pMover->calcTimeToNextRailPoint(&time);
        pRotator->rotateBetweenPoints(getCurrentRailPointNo(pActor), time);
        return true;
    }



    inline void getMapPartsObjectName(char* pBuffer, u32 size, const JMapInfoIter& rIter) {
        const char* pName = "";
        getObjectName(&pName, rIter);
        s32 shapeId = -1;
        getJMapInfoShapeIdWithInit(rIter, &shapeId);
        getMapPartsObjectName(pBuffer, size, pName, shapeId);
    }

    void getMapPartsObjectNameIfExistShapeID(char* pBuffer, u32 size, const JMapInfoIter& rIter) {
        const char* pName = "";
        getObjectName(&pName, rIter);
        s32 shapeId = -1;
        getJMapInfoShapeIdWithInit(rIter, &shapeId);
        if (shapeId >= 0) {
            getMapPartsObjectName(pBuffer, size, pName, shapeId);
        } else {
            snprintf(pBuffer, size, "%s", pName);
        }
    }

    inline void getMapPartsObjectName(char* pBuffer, u32 size, const char* pName, s32 shapeId) {
        snprintf(pBuffer, size, "%s%02d", pName, shapeId);
    }

    void initMapPartsClipping(LiveActor* pActor, const JMapInfoIter& rIter, TVec3f* pCenter, bool ignoreRail) {
        f32 radius = 0.0f;
        if (getJ3DModel(pActor)) {
            calcModelBoundingRadius(&radius, pActor);
            if (isNearZero(radius)) {
                radius = getCollisionBoundingSphereRange(pActor);
            }
        } else {
            radius = getCollisionBoundingSphereRange(pActor);
        }

        if (isConnectedWithRail(rIter) && !ignoreRail) {
            initAndSetRailClipping(pCenter, pActor, 0.5f * radius, radius);
        } else {
            setClippingTypeSphere(pActor, radius);
        }

        setGroupClipping(pActor, rIter, 128);
    }


    MapPartsRailGuideDrawer* createMapPartsRailGuideDrawer(LiveActor* pActor, const char* pName, const JMapInfoIter& rIter) {
        MapPartsRailGuideHolder* pHolder = static_cast< MapPartsRailGuideHolder* >(MR::createSceneObj(SceneObj_MapPartsRailGuideHolder));
        return pHolder->createRailGuide(pActor, pName, rIter);
    }

};  // namespace MR


namespace MR {
    bool getJMapInfoArgNoInit(const JMapInfoIter& rIter, const char* pKey, s32* pValue) {
        s32 value;
        if (!rIter.getValue(pKey, &value)) {
            return false;
        }
        if (value == -1) {
            return false;
        }
        *pValue = value;
        return true;
    }

    bool getJMapInfoArgNoInit(const JMapInfoIter& rIter, const char* pKey, f32* pValue) {
        s32 value;
        if (!getJMapInfoArgNoInit(rIter, pKey, &value)) {
            return false;
        }
        *pValue = value;
        return true;
    }

    bool getMapPartsArgMoveConditionType(s32* pValue, const JMapInfoIter& rIter) {
        return MR::getJMapInfoArgNoInit(rIter, "MoveConditionType", pValue);
    }

    bool getMapPartsArgRotateSpeed(f32* pValue, const JMapInfoIter& rIter) {
        return MR::getJMapInfoArgNoInit(rIter, "RotateSpeed", pValue);
    }

    bool getMapPartsArgRotateTime(s32* pValue, const JMapInfoIter& rIter) {
        return MR::getJMapInfoArgNoInit(rIter, "RotateSpeed", pValue);
    }

    bool getMapPartsArgRotateAngle(f32* pValue, const JMapInfoIter& rIter) {
        return MR::getJMapInfoArgNoInit(rIter, "RotateAngle", pValue);
    }

    bool getMapPartsArgRotateAxis(s32* pValue, const JMapInfoIter& rIter) {
        return MR::getJMapInfoArgNoInit(rIter, "RotateAxis", pValue);
    }

    bool getMapPartsArgRotateAccelType(s32* pValue, const JMapInfoIter& rIter) {
        return MR::getJMapInfoArgNoInit(rIter, "RotateAccelType", pValue);
    }

    bool getMapPartsArgRotateStopTime(s32* pValue, const JMapInfoIter& rIter) {
        return MR::getJMapInfoArgNoInit(rIter, "RotateStopTime", pValue);
    }

    bool getMapPartsArgRotateType(s32* pValue, const JMapInfoIter& rIter) {
        return MR::getJMapInfoArgNoInit(rIter, "RotateType", pValue);
    }

    s32 getMapPartsArgShadowType(s32* pValue, const JMapInfoIter& rIter) {
        return MR::getJMapInfoArgNoInit(rIter, "ShadowType", pValue);
    }

    bool getMapPartsArgSignMotionType(s32* pValue, const JMapInfoIter& rIter) {
        return MR::getJMapInfoArgNoInit(rIter, "SignMotionType", pValue);
    }

    bool getMapPartsArgMovePosture(s32* pValue, const LiveActor* pActor) {
        return getRailArg0NoInit(pActor, pValue);
    }

    bool getMapPartsArgMoveStopType(s32* pValue, const LiveActor* pActor) {
        return getRailArg1NoInit(pActor, pValue);
    }

    bool getMapPartsArgRailGuideType(s32* pValue, const LiveActor* pActor) {
        return getRailArg2NoInit(pActor, pValue);
    }

    bool getMapPartsArgRailInitPosType(s32* pValue, const LiveActor* pActor) {
        return getRailArg4NoInit(pActor, pValue);
    }

    bool getMapPartsArgMoveSnap(s32* pValue, const LiveActor* pActor) {
        return getRailArg5NoInit(pActor, pValue);
    }

    bool getMapPartsArgMoveSpeed(f32* pValue, const LiveActor* pActor) {
        return getCurrentRailPointArg0NoInit(pActor, pValue);
    }

    bool getMapPartsArgStopTime(s32* pValue, const LiveActor* pActor) {
        return getCurrentRailPointArg5NoInit(pActor, pValue);
    }

    bool getMapPartsArgAccelTime(s32* pValue, const LiveActor* pActor) {
        return getCurrentRailPointArg1NoInit(pActor, pValue);
    }

    bool getMapPartsArgSpeedCalcType(s32* pValue, const LiveActor* pActor) {
        return getCurrentRailPointArg7NoInit(pActor, pValue);
    }


    bool getMapPartsArgRailRotateSpeed(f32* pValue, const LiveActor* pActor, s32 point) {
        return getRailPointArg2WithInit(pActor, point, pValue);
    }

    bool getMapPartsArgRailRotateTime(s32* pValue, const LiveActor* pActor, s32 point) {
        return getRailPointArg2WithInit(pActor, point, pValue);
    }

    bool getMapPartsArgRailRotateAngle(f32* pValue, const LiveActor* pActor, s32 point) {
        return getRailPointArg3WithInit(pActor, point, pValue);
    }

    bool getMapPartsArgRailRotateAxis(s32* pValue, const LiveActor* pActor, s32 point) {
        return getRailPointArg4NoInit(pActor, point, pValue);
    }

    bool getMapPartsArgRailRotateType(s32* pValue, const LiveActor* pActor, s32 point) {
        return getRailPointArg6NoInit(pActor, point, pValue);
    }

    bool getMapPartsArgSpeedCalcType(s32* pValue, const LiveActor* pActor, s32 point) {
        return getRailPointArg7NoInit(pActor, point, pValue);
    }

    bool getMapPartsArgMoveTimeToNextPoint(s32* pValue, const LiveActor* pActor) {
        return getCurrentRailPointArg0NoInit(pActor, pValue);
    }

    bool isMapPartsSignMotionTypeMoveStart(s32 type) {
        if (type == 1) {
            return true;
        }

        return type == 4;
    }

    bool isMapPartsSignMotionTypeMoveWait(s32 type) {
        if (type == 2) {
            return true;
        }

        return type == 5;
    }

    bool hasMapPartsMoveStartSignMotion(s32 type) {
        if (isMapPartsSignMotionTypeMoveStart(type)) {
            return true;
        }

        return isMapPartsSignMotionTypeMoveWait(type);
    }


    bool isMapPartsShadowTypeNone(s32 shadowType) {
        return shadowType == 0;
    }

    bool hasMapPartsShadow(s32 flag) {
        return flag != 0;
    }

    bool isMoveStartTypeUnconditional(s32 startType) {
        return startType == 0;
    }

    bool isMoveStartTypePlayerOnStopEnd(s32 startType) {
        return startType == 1;
    }

    bool isMapPartsRailInitPosTypeRailPos(s32 posType) {
        return posType == 0;
    }

    bool isMapPartsRailInitPosTypeRailPoint(s32 posType) {
        return posType == 1;
    }

    bool isMapPartsRailInitPosTypePoint0(s32 posType) {
        return posType == 2;
    }

    bool isMapPartsRailSpeedCalcTypeTime(s32 calcType) {
        return calcType == 1;
    }
};  // namespace MR
