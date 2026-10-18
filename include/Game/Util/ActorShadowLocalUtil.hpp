#pragma once

#include <revolution/types.h>
#include "Inline.hpp"

class JMapInfoIter;
class LiveActor;
class ShadowController;
class ShadowDrawer;
class ShadowSurfaceBox;
class ShadowSurfaceCircle;
class ShadowSurfaceOval;
class ShadowVolumeBox;
class ShadowVolumeCylinder;
class ShadowVolumeDrawer;
class ShadowVolumeFlatModel;
class ShadowVolumeLine;
class ShadowVolumeOval;
class ShadowVolumeOvalPole;
class ShadowVolumeSphere;

namespace ActorShadow {
    NO_INLINE u32 getShadowControllerCount(const LiveActor*);
    NO_INLINE ShadowController* getShadowController(const LiveActor*, u32);
    NO_INLINE ShadowController* getShadowController(const LiveActor*, const char*);
    NO_INLINE bool isExistShadowController(const LiveActor*, const char*);
    NO_INLINE ShadowController* createShadowControllerVolumeParam(LiveActor*, const char*);
    NO_INLINE ShadowController* createShadowControllerSuefaceParam(LiveActor*, const char*);
    NO_INLINE ShadowSurfaceOval* getShadowSurfaceOval(const LiveActor*, const char*);
    NO_INLINE ShadowVolumeSphere* getShadowVolumeSphere(const LiveActor*, const char*);
    NO_INLINE ShadowVolumeCylinder* getShadowVolumeCylinder(const LiveActor*, const char*);
    NO_INLINE ShadowVolumeBox* getShadowVolumeBox(const LiveActor*, const char*);
    NO_INLINE ShadowVolumeDrawer* getShadowVolumeDrawer(const LiveActor*, const char*);
    NO_INLINE bool getJointNameFromCSV(const char**, const JMapInfoIter&);
    NO_INLINE s32 getShadowTypeFromCSV(const JMapInfoIter&);
    NO_INLINE void setUpShadowControlBaseMtxFromCSV(ShadowController*, LiveActor*, const JMapInfoIter&);
    NO_INLINE void setUpShadowControlIsFollowScaleFromCSV(ShadowController*, const JMapInfoIter&);
    NO_INLINE void setUpShadowControlIsSyncShowFromCSV(ShadowController*, const JMapInfoIter&);
    NO_INLINE void setUpShadowControlCalcCollisionFromCSV(ShadowController*, const JMapInfoIter&);
    NO_INLINE void setUpShadowControlCalcGravityFromCSV(ShadowController*, const JMapInfoIter&);
    NO_INLINE void setUpShadowControlFromCSV(ShadowController*, LiveActor*, const JMapInfoIter&);
    NO_INLINE ShadowController* createShadowControlFromCSV(LiveActor*, const JMapInfoIter&);
    NO_INLINE void setUpShadowVolumeFromCSV(ShadowVolumeDrawer*, const JMapInfoIter&);
    NO_INLINE ShadowSurfaceCircle* createShadowSurfaceCircleFromCSV(LiveActor*, const JMapInfoIter&);
    NO_INLINE ShadowSurfaceOval* createShadowSurfaceOvalFromCSV(LiveActor*, const JMapInfoIter&);
    NO_INLINE ShadowSurfaceBox* createShadowSurfaceBoxFromCSV(LiveActor*, const JMapInfoIter&);
    NO_INLINE ShadowVolumeSphere* createShadowVolumeSphereFromCSV(LiveActor*, const JMapInfoIter&);
    NO_INLINE ShadowVolumeOval* createShadowVolumeOvalFromCSV(LiveActor*, const JMapInfoIter&);
    NO_INLINE ShadowVolumeOvalPole* createShadowVolumeOvalPoleFromCSV(LiveActor*, const JMapInfoIter&);
    NO_INLINE ShadowVolumeCylinder* createShadowVolumeCylinderFromCSV(LiveActor*, const JMapInfoIter&);
    NO_INLINE ShadowVolumeBox* createShadowVolumeBoxFromCSV(LiveActor*, const JMapInfoIter&);
    NO_INLINE ShadowVolumeFlatModel* createShadowVolumeFlatModelFromCSV(LiveActor*, const JMapInfoIter&);
    NO_INLINE ShadowVolumeLine* createShadowVolumeLineFromCSV(LiveActor*, const JMapInfoIter&);
    NO_INLINE void addShadowFromCSV(LiveActor*, const JMapInfoIter&);
};  // namespace ActorShadow
