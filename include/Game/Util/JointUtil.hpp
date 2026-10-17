#pragma once

#include <revolution.h>

class LiveActor;

namespace MR {
    MtxPtr getJointMtx(const LiveActor*, const char*);
};

// Merged from Petari (core lane): declarations missing here
#include <JSystem/JGeometry/TVec.hpp>
#include <revolution/types.h>
class J3DJoint;
class J3DModel;
class J3DModelData;

namespace MR {
    J3DJoint* getJoint(J3DModel*, const char*);
    const char* getJointName(const LiveActor*, int);
    s32 getJointNum(const LiveActor*);
    bool isExistJoint(const LiveActor*, const char*);
    void copyJointPos(J3DModel*, const char*, TVec3f*);
    void copyJointScale(const LiveActor*, const char*, TVec3f*);
    void hideJoint(J3DJoint*);
    void hideJointAndChildren(J3DJoint*);
    void showJoint(J3DJoint*);
    void showJointAndChildren(J3DJoint*);
    f32 getJointTransX(const J3DJoint*);
    f32 getJointTransY(const J3DJoint*);
    f32 getJointTransZ(const J3DJoint*);
    J3DJoint* searchChildJoint(J3DJoint* pRoot, J3DJoint* pJoint);
    J3DJoint* getParentJoint(J3DModelData*, J3DJoint*);
};  // namespace MR
