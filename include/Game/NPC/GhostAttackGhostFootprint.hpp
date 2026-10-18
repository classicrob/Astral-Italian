#pragma once
#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
class FootPrint;
class GhostAttackGhostFootprint : public LiveActor {
public:
    GhostAttackGhostFootprint(LiveActor*);
    virtual ~GhostAttackGhostFootprint();
    virtual void appear();
    virtual void control();
    virtual MtxPtr getBaseMtx() const;
    void stop();
    void initPrintParameters();
    void updatePrint();
    void addPrintFromRay();
    void addPrintFromGround();
    bool tryAddPrintOnSlide();
    bool tryAddPrintInWater();
    LiveActor* mHost;
    FootPrint* mFootPrint;
    TPos3f mBaseMtx;
    TVec3f mPreviousMovement;
    TVec3f mDirection;
    TPos3f mJointMtx;
};
