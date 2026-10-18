#pragma once

#include "Game/Map/LightZoneDataHolder.hpp"
#include <JSystem/JGeometry/TBox.hpp>

class AreaObj;
class WaterArea;
class AreaObjContainer;
class AreaObjMgr;
class AreaCubeBoundingBox;
class WaterInfo;

namespace MR {
    bool isInTowerModeCylinder(const TVec3f&, TVec3f*, TVec3f*, f32*);
    bool isExistAreaObj(const char*);
    void calcSpherePos(TVec3f*, const AreaObj*);
    f32 getSphereRadius(const AreaObj*);
    AreaObjMgr* getAreaObjManager(const char*);
    AreaObj* getAreaObj(const char*, const TVec3f&);
    bool isInAreaObj(const char*, const TVec3f&);
    s32 getAreaObjArg(const AreaObj*, s32);
    void tryToUpdatePlayerRestartIdInfo(const TVec3f&);

    bool calcAreaMoveVelocity(TVec3f* pVelocity, const TVec3f& rPos);

    void calcCylinderPos(TVec3f*, const AreaObj*);
    void calcCylinderCenterPos(TVec3f*, const AreaObj*);

    void calcCylinderUpVec(TVec3f*, const AreaObj*);
    f32 getCylinderRadius(const AreaObj*);

    void calcCubeWorldBox(TDirBox3f* pBox, const AreaObj* pArea);
    void calcCubePos(const AreaObj*, TVec3f*);
    void getAreaCenterPos(TVec3f*, const AreaObj*);
    void calcCubeAxisZ(const AreaObj*, TVec3f*);
    void calcCubeLocalPos(TVec3f*, const AreaObj*, const TVec3f&);

    void calcCubeRotate(const AreaObj*, TVec3f*);

    bool calcWhirlPoolAccelInfo(const TVec3f&, TVec3f*);

    bool isAreaFormCube(const AreaObj*);
    bool isAreaFormCylinder(const AreaObj*);
    AreaObj* getAreaObjAtPlayerPos(const char*);
    void requestMovementOnAreas(const char*);
    void requestMovementOnSoundAreas();
    bool hasNoWaterAreaCube(const WaterInfo*);
    bool intersectsWaterAreaInfo(const WaterInfo*, const WaterInfo*);
    bool func_80062320(const WaterArea*, TVec3f*, TVec3f*);
    bool func_80062480(const WaterArea*, TVec3f*, TVec3f*);
    bool func_80062020(const WaterInfo*, TVec3f*, TVec3f*);
    bool func_800621A0(const WaterInfo*, TVec3f*, TVec3f*);
    void calcSphereFrontVec(TVec3f*, const AreaObj*, const TVec3f&);
    void calcCylinderFrontVec(TVec3f*, const AreaObj*, const TVec3f&);

    TBox3f* getCubeLocalBox(const AreaObj*);

    AreaObj* getCurrentAstroOverlookAreaObj();

    AreaObj* getAreaIn(const char*, const TVec3f&);

    bool getWaterAreaObj(WaterInfo*, const TVec3f&);
    bool getWaterAreaInfo(WaterInfo* pInfo, const TVec3f& rPos, const TVec3f& rGravity, bool skipConnectedArea);
};  // namespace MR

namespace MR {
    bool isInTowerModeCylinder(const TVec3f&, TVec3f*, TVec3f*, f32*);
}
