#include "Game/MapObj/PlacementCircleObj.hpp"
#include "Game/Util.hpp"
#include <JSystem/JMath/JMATrigonometric.hpp>
PlacementObj::PlacementObj(const char* name) : LiveActor(name),mPlacementNum(8) {}
PlacementObj::~PlacementObj() {}
s32 PlacementObj::getPlacementNum() const { return mPlacementNum; }
void PlacementObj::calcPlacementPosition(s32 index,TVec3f* out) { calcPosition(index,mPlacementNum,out); }
PlacementCircleObj::PlacementCircleObj() : PlacementObj("円形配置オブジェ"),mRight(1.0f,0.0f,0.0f),mFront(0.0f,0.0f,1.0f),mRadius(200.0f) {}
PlacementCircleObj::~PlacementCircleObj() {}
void PlacementCircleObj::init(const JMapInfoIter& iter) {
    MR::initDefaultPos(this,iter);
    Mtx mtx;
    MR::makeMtxTR(mtx,this);
    f32 rz=mtx[2][0]; f32 ry=mtx[1][0]; f32 rx=mtx[0][0];
    mRight.set<f32>(rx,ry,rz);
    f32 fz=mtx[2][2]; f32 fy=mtx[1][2]; f32 fx=mtx[0][2];
    mFront.set<f32>(fx,fy,fz);
    makeActorDead();
    s32 count=8;
    MR::getJMapInfoArg0NoInit(iter,&count);
    mPlacementNum=count;
    MR::getJMapInfoArg2NoInit(iter,&mRadius);
}
// NONMATCHING: equivalent temporary registers, frame slots and commuted multiply operands.
void PlacementCircleObj::calcPosition(s32 index,s32 count,TVec3f* out) {
    TVec3f center(mPosition);
    f32 angle=(6.2831855f/count)*index;
    f32 x=mRadius*JMath::sSinCosTable.cosRadian(angle)*mScale.x;
    f32 z=mRadius*JMath::sSinCosTable.sinRadian(angle)*mScale.z;
    TVec3f result((mFront*x+mRight*z)+center);
    out->x=result.x;
    out->y=result.y;
    out->z=result.z;
}

#include "Game/MapObj/PlacementRailObj.hpp"
#include "Game/Util.hpp"
PlacementRailObj::PlacementRailObj() : PlacementObj("レール配置オブジェ") {}
void PlacementRailObj::init(const JMapInfoIter& iter) {
    s32 count = 8;
    MR::getJMapInfoArg0NoInit(iter, &count);
    mPlacementNum = count;
    MR::initDefaultPos(this, iter);
    initRailRider(iter);
    makeActorDead();
}
void PlacementRailObj::calcPosition(s32 index, s32 count, TVec3f* position) {
    f32 length = MR::getRailTotalLength(this);
    if (count <= 1) { MR::calcRailPosAtCoord(position, this, 0.0f); return; }
    f32 interval;
    if (MR::isLoopRail(this)) interval = length / count;
    else interval = length / (count - 1);
    MR::calcRailPosAtCoord(position, this, interval * index);
}
