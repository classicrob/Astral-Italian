#define JGEOMETRY_VEC3_MULASSIGN_NOINLINE
#pragma auto_inline off

#include "Game/MapObj/RepeatBlock.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util.hpp"
#include <revolution.h>

namespace {
    // direction -> opposite direction
    const s32 sOppositeDir[6] = {1, 0, 3, 2, 5, 4};
};  // namespace


// direction -> cell offset
static RepeatBlockCell sDirTable[6] = {
    RepeatBlockCell(1, 0, 0), RepeatBlockCell(-1, 0, 0), RepeatBlockCell(0, 1, 0), RepeatBlockCell(0, -1, 0), RepeatBlockCell(0, 0, 1), RepeatBlockCell(0, 0, -1),
};

RepeatBlockGrid::RepeatBlockGrid(const TPos3f& rMtx, f32 x, f32 y, f32 z) : mMtx(rMtx) {
    mCellSize.x = x;
    mCellSize.y = y;
    mCellSize.z = z;
    mInvMtx.set(rMtx);
    mInvMtx.invert(mInvMtx);
}

void RepeatBlockGrid::calcCellPos(TVec3f* pDst, const RepeatBlockCell& rCell) const {
    pDst->set< f32 >(mCellSize.x * rCell.mX, mCellSize.y * rCell.mY, mCellSize.z * rCell.mZ);
}

void RepeatBlockGrid::calcWorldCellPos(TVec3f* pDst, const RepeatBlockCell& rCell) const {
    calcCellPos(pDst, rCell);
    mMtx.mult(*pDst, *pDst);
}

// NONMATCHING: vector add remains outlined; scalar conversion spill order differs.
void RepeatBlockGrid::calcCell(RepeatBlockCell* pCell, const TVec3f& rPos) const {
    TVec3f pos(rPos);
    TVec3f half(mCellSize);
    half.x *= 0.5f;
    half.y *= 0.5f;
    half.z *= 0.5f;
    pos += half;
    pos.x /= mCellSize.x;
    pos.y /= mCellSize.y;
    pos.z /= mCellSize.z;
    s16 x = pos.x;
    s16 y = pos.y;
    s16 z = pos.z;
    pCell->mX = x;
    pCell->mY = y;
    pCell->mZ = z;
    if (pos.x < 0.0f) {
        pCell->mX--;
    }
    if (pos.y < 0.0f) {
        pCell->mY--;
    }
    if (pos.z < 0.0f) {
        pCell->mZ--;
    }
}

void RepeatBlockGrid::localToWorld(TVec3f* pDst, const TVec3f& rSrc) const {
    pDst->set(rSrc);
    mMtx.mult(*pDst, *pDst);
}

#pragma dont_inline on
void RepeatBlockGrid::worldToCell(RepeatBlockCell* pCell, const TVec3f& rPos) const {
    TVec3f local(rPos);
    mInvMtx.mult(local, local);
    calcCell(pCell, local);
}

void RepeatBlockGrid::getAxisX(TVec3f* pDst) const {
    f32 z = mMtx.mMtx[2][0];
    f32 y = mMtx.mMtx[1][0];
    f32 x = mMtx.mMtx[0][0];
    pDst->set< f32 >(x, y, z);
}

void RepeatBlockGrid::getAxisY(TVec3f* pDst) const {
    f32 z = mMtx.mMtx[2][1];
    f32 y = mMtx.mMtx[1][1];
    f32 x = mMtx.mMtx[0][1];
    pDst->set< f32 >(x, y, z);
}

void RepeatBlockGrid::getAxisZ(TVec3f* pDst) const {
    f32 z = mMtx.mMtx[2][2];
    f32 y = mMtx.mMtx[1][2];
    f32 x = mMtx.mMtx[0][2];
    pDst->set< f32 >(x, y, z);
}

#pragma dont_inline reset
RepeatBlockHistory::RepeatBlockHistory(s32 capacity) : mCapacity(capacity), mCount(0), mHead(0) {
    mNodes = new RepeatBlockHistoryNode[capacity];
}

void RepeatBlockHistory::push(const RepeatBlockCell& rCell) {
    if (static_cast<u32>(mCount) == static_cast<u32>(mCapacity)) {
        propagate();
    }
    RepeatBlockHistoryNode* pNode = &mNodes[mHead];
    pNode->mCell.mX = rCell.mX;
    pNode->mCell.mY = rCell.mY;
    pNode->mCell.mZ = rCell.mZ;
    connect();
    mHead++;
    if (static_cast<u32>(mHead) >= static_cast<u32>(mCapacity)) {
        mHead = 0;
    }
    if (static_cast<u32>(mCount) < static_cast<u32>(mCapacity)) {
        mCount++;
    }
}

bool RepeatBlockHistory::hasNeighbor(s32 n, s32 dir) const {
    return mNodes[calcIndex(n)].mNeighbor[dir] != -1;
}

s32 RepeatBlockHistory::getDistance(s32 n, s32 dir) const {
    return mNodes[calcIndex(n)].mDistance[dir];
}

#pragma dont_inline on
RepeatBlockHistoryNode* RepeatBlockHistory::getNode(s32 n) const {
    return &mNodes[calcIndex(n)];
}

#pragma dont_inline reset
s32 RepeatBlockHistory::calcIndex(s32 n) const {
    s32 idx = mHead - (n + 1);
    if (idx < 0) {
        idx += mCapacity;
    }
    return idx;
}

void RepeatBlockHistory::propagate() {
    RepeatBlockHistoryNode* pHead = &mNodes[mHead];
    for (s32 i = 0; i < 6; i++) {
        s32 neighbor = pHead->mNeighbor[i];
        if (neighbor >= 0) {
            s32 opp = sOppositeDir[i];
            s32 dist = pHead->mDistance[i] + pHead[0].mDistance[opp];
            RepeatBlockHistoryNode* pOppHead = &pHead[0];
            if (dist < 10000) {
                mNodes[neighbor].mNeighbor[opp] = pOppHead->mNeighbor[opp];
                mNodes[pHead->mNeighbor[i]].mDistance[opp] = dist;
            } else {
                mNodes[neighbor].mNeighbor[opp] = -1;
                mNodes[pHead->mNeighbor[i]].mDistance[opp] = 10000;
            }
        }
    }
}

// NONMATCHING: packed cell temporary slots, GPR allocation and loop scheduling differ.
void RepeatBlockHistory::connect() {
    RepeatBlockHistoryNode* pNode = &mNodes[mHead];
    for (s32 i = 0; i < 6; i++) {
        pNode->mNeighbor[i] = -1;
        pNode->mDistance[i] = 10000;
    }
    for (u32 i = 0; i < mCount; i++) {
        if (i != mHead) {
            REPEAT_BLOCK_CELL_SUB(diff, mNodes[i].mCell, pNode->mCell);
            RepeatBlockCell sgn = RepeatBlockCell::sign(diff);
            s32 dist = RepeatBlockCell::absSum(diff);
            for (s32 dir = 0; dir < 6; dir++) {
                if (sgn == sDirTable[dir] && dist < 10000) {
                    RepeatBlockHistoryNode* pOther = &mNodes[i];
                    s32 opp = sOppositeDir[dir];
                    if (pOther->mNeighbor[opp] == -1 || pOther->mDistance[opp] > dist) {
                        pNode->mNeighbor[dir] = i;
                        pNode->mDistance[dir] = dist;
                        mNodes[i].mNeighbor[opp] = mHead;
                        mNodes[i].mDistance[opp] = dist;
                    }
                }
            }
        }
    }
}

RepeatBlockCell::RepeatBlockCell(s16 x, s16 y, s16 z) {
    mX = x;
    mY = y;
    mZ = z;
}

RepeatBlockShadowDrawer::RepeatBlockShadowDrawer(RepeatBlockHistory* pHistory, RepeatBlockGrid* pGrid) : ShadowVolumeDrawer("リピートブロックの影描画") {
    mHistory = pHistory;
    mGrid = pGrid;
    mPointNum = 0;
    mDirection = 3;
    mMaxLength = 16;
    mVolume = new RepeatBlockVolume(pGrid);
    mOffset = 0.0f;
    mHeadPoints = new TVec3f[8];
    mHasHead = false;
    mTailPoints = new TVec3f[8];
    mHasTail = false;
    mPoints = new TVec3f[pHistory->mCapacity * 8];
}

void RepeatBlockShadowDrawer::loadModelDrawMtx() const {
    GXLoadPosMtxImm(MR::getCameraViewMtx(), 0);
    GXSetCurrentMtx(0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
}

void RepeatBlockShadowDrawer::drawShape() const {
    if (mHasHead) {
        drawBox(mHeadPoints);
    }
    if (mHasTail) {
        drawBox(mTailPoints);
    }
    for (s32 i = 0; i < mPointNum; i++) {
        drawBox(&mPoints[i * 8]);
    }
}

bool RepeatBlockShadowDrawer::isDraw() const {
    return mPointNum != 0;
}

void RepeatBlockShadowDrawer::setDirection(s32 dir) {
    mDirection = dir;
}

// Target calls the vector multiply-assignment helper; shared vector headers currently outline scale instead.
void RepeatBlockShadowDrawer::makeVertexBuffer() {
    mPointNum = 0;
    TVec3f axisX, axisY, axisZ;
    mGrid->getAxisX(&axisX);
    mGrid->getAxisY(&axisY);
    mGrid->getAxisZ(&axisZ);
    f32 sizeX = mGrid->mCellSize.x;
    TVec3f X = axisX.scaleInline(0.5f).scaleInline(sizeX);
    f32 sizeY = mGrid->mCellSize.y;
    TVec3f Y = axisY.scaleInline(0.5f).scaleInline(sizeY);
    f32 sizeZ = mGrid->mCellSize.z;
    TVec3f Z = axisZ.scaleInline(0.5f).scaleInline(sizeZ);
    TVec3f c0, c1, c2, c3;
    switch (mDirection) {
    case 0:
        c0 = (X + Y) + Z;
        c1 = (X - Y) + Z;
        c2 = (X - Y) - Z;
        c3 = (X + Y) - Z;
        break;
    case 1:
        c0 = (-X + Y) - Z;
        c1 = (-X - Y) - Z;
        c2 = (-X - Y) + Z;
        c3 = (-X + Y) + Z;
        break;
    case 2:
        c0 = (X + Y) + Z;
        c1 = (X + Y) - Z;
        c2 = (-X + Y) - Z;
        c3 = (-X + Y) + Z;
        break;
    case 3:
        c0 = (X - Y) - Z;
        c1 = (X - Y) + Z;
        c2 = (-X - Y) + Z;
        c3 = (-X - Y) - Z;
        break;
    case 4:
        c0 = (-X + Y) + Z;
        c1 = (-X - Y) + Z;
        c2 = (X - Y) + Z;
        c3 = (X + Y) + Z;
        break;
    case 5:
        c0 = (X + Y) - Z;
        c1 = (X - Y) - Z;
        c2 = (-X - Y) - Z;
        c3 = (-X + Y) - Z;
        break;
    }

    TVec3f drop;
    calcDropOffset(&drop);
    s32 end = mHistory->mCount;
    if (end == mHistory->mCapacity) {
        end--;
    }
    s32 start = 1;
    if (end == 1) {
        start = 0;
    }
    for (s32 i = start; i < end; i++) {
        if (!mHistory->hasNeighbor(i, mDirection) || mHistory->getDistance(i, mDirection) != 1) {
            f32 len = calcFaceLength(i, mDirection);
            TVec3f pos;
            mGrid->calcWorldCellPos(&pos, mHistory->getNode(i)->mCell);
            s32 base = mPointNum * 8;
            mPoints[base] = pos + c0;
            mPoints[base + 1] = pos + c1;
            mPoints[base + 2] = pos + c2;
            mPoints[base + 3] = pos + c3;
            mPoints[base + 4] = (pos + c0) + drop * len;
            mPoints[base + 5] = (pos + c1) + drop * len;
            mPoints[base + 6] = (pos + c2) + drop * len;
            mPoints[base + 7] = (pos + c3) + drop * len;
            mPointNum++;
        }
    }
}

void RepeatBlockShadowDrawer::makeFace() {
    makeHeadFace();
    makeTailFace();
}

void RepeatBlockShadowDrawer::calcDropOffset(TVec3f* pDst) const {
    TVec3f axisX, axisY, axisZ;
    mGrid->getAxisX(&axisX);
    mGrid->getAxisY(&axisY);
    mGrid->getAxisZ(&axisZ);
    switch (mDirection) {
    case 0:
        pDst->set(axisX * mGrid->mCellSize.x);
        break;
    case 1:
        pDst->set(-axisX * mGrid->mCellSize.x);
        break;
    case 2:
        pDst->set(axisY * mGrid->mCellSize.y);
        break;
    case 3:
        pDst->set(-axisY * mGrid->mCellSize.y);
        break;
    case 4:
        pDst->set(axisZ * mGrid->mCellSize.z);
        break;
    case 5:
        pDst->set(-axisZ * mGrid->mCellSize.z);
        break;
    }
}

// NONMATCHING: target keeps the diff struct in a 0x20 frame with by-value temporary copies; ours uses the straight-line diff
s32 RepeatBlockShadowDrawer::calcDirection(const RepeatBlockCell* pFrom, const RepeatBlockCell* pTo) const {
    REPEAT_BLOCK_CELL_SUB(diff, *pTo, *pFrom);
    if (diff.mX < 0) {
        return 1;
    }
    if (diff.mX > 0) {
        return 0;
    }
    if (diff.mY < 0) {
        return 3;
    }
    if (diff.mY > 0) {
        return 2;
    }
    s32 ret = 4;
    if (diff.mZ < 0) {
        ret = 5;
    }
    return ret;
}

void RepeatBlockShadowDrawer::drawBox(const TVec3f* pPoints) const {
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 10);
    GXPosition3f32(pPoints[0].x, pPoints[0].y, pPoints[0].z);
    GXPosition3f32(pPoints[4].x, pPoints[4].y, pPoints[4].z);
    GXPosition3f32(pPoints[1].x, pPoints[1].y, pPoints[1].z);
    GXPosition3f32(pPoints[5].x, pPoints[5].y, pPoints[5].z);
    GXPosition3f32(pPoints[2].x, pPoints[2].y, pPoints[2].z);
    GXPosition3f32(pPoints[6].x, pPoints[6].y, pPoints[6].z);
    GXPosition3f32(pPoints[3].x, pPoints[3].y, pPoints[3].z);
    GXPosition3f32(pPoints[7].x, pPoints[7].y, pPoints[7].z);
    GXPosition3f32(pPoints[0].x, pPoints[0].y, pPoints[0].z);
    GXPosition3f32(pPoints[4].x, pPoints[4].y, pPoints[4].z);
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
    GXPosition3f32(pPoints[0].x, pPoints[0].y, pPoints[0].z);
    GXPosition3f32(pPoints[1].x, pPoints[1].y, pPoints[1].z);
    GXPosition3f32(pPoints[3].x, pPoints[3].y, pPoints[3].z);
    GXPosition3f32(pPoints[2].x, pPoints[2].y, pPoints[2].z);
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
    GXPosition3f32(pPoints[7].x, pPoints[7].y, pPoints[7].z);
    GXPosition3f32(pPoints[6].x, pPoints[6].y, pPoints[6].z);
    GXPosition3f32(pPoints[4].x, pPoints[4].y, pPoints[4].z);
    GXPosition3f32(pPoints[5].x, pPoints[5].y, pPoints[5].z);
}

s32 RepeatBlockShadowDrawer::calcFaceLength(s32 n, s32 dir) const {
    s32 len = mHistory->getDistance(n, dir);
    if (len > mMaxLength) {
        len = mMaxLength;
    }
    return len;
}

void RepeatBlockShadowDrawer::makeHeadFace() {
    mVolume->mIsValid = false;
    if (static_cast<u32>(mHistory->mCount) < 2u) {
        mHasHead = false;
        return;
    }
    s32 len = calcFaceLength(0, mDirection);
    if (len == 1) {
        mHasHead = false;
        return;
    }
    TVec3f drop;
    calcDropOffset(&drop);
    RepeatBlockCell* pFirst = &mHistory->getNode(0)->mCell;
    mVolume->setup(pFirst, calcDirection(pFirst, &mHistory->getNode(1)->mCell), mOffset);
    TVec3f offset(drop);
    offset *= (f32)len;
    mHasHead = mVolume->calcFace(mHeadPoints, mDirection, offset);
}

void RepeatBlockShadowDrawer::makeTailFace() {
    mVolume->mIsValid = false;
    if (static_cast<u32>(mHistory->mCount) < static_cast<u32>(mHistory->mCapacity)) {
        mHasTail = false;
        return;
    }
    s32 len = calcFaceLength(mHistory->mCount - 1, mDirection);
    if (len == 1) {
        mHasTail = false;
        return;
    }
    TVec3f drop;
    calcDropOffset(&drop);
    RepeatBlockCell* pLast = &mHistory->getNode(mHistory->mCount - 1)->mCell;
    mVolume->setup(pLast, calcDirection(pLast, &mHistory->getNode(mHistory->mCount - 2)->mCell), 1.0f - mOffset);
    TVec3f offset(drop);
    offset *= (f32)len;
    mHasTail = mVolume->calcFace(mTailPoints, mDirection, offset);
}

RepeatBlockVolume::RepeatBlockVolume(RepeatBlockGrid* pGrid) : mGrid(pGrid), mIsValid(false) {
}

void RepeatBlockVolume::setup(const RepeatBlockCell* pCell, s32 dir, f32 rate) {
    TVec3f shift;
    TVec3f scale;
    switch (dir) {
    case 0:
        scale.x = rate;
        scale.y = 1.0f;
        scale.z = 1.0f;
        shift.set< f32 >(-0.5f, 0.0f, 0.0f);
        break;
    case 1:
        scale.x = rate;
        scale.y = 1.0f;
        scale.z = 1.0f;
        shift.set< f32 >(0.5f, 0.0f, 0.0f);
        break;
    case 2:
        scale.y = rate;
        scale.x = 1.0f;
        scale.z = 1.0f;
        shift.set< f32 >(0.0f, -0.5f, 0.0f);
        break;
    case 3:
        scale.y = rate;
        scale.x = 1.0f;
        scale.z = 1.0f;
        shift.set< f32 >(0.0f, 0.5f, 0.0f);
        break;
    case 4:
        scale.z = rate;
        scale.x = 1.0f;
        scale.y = 1.0f;
        shift.set< f32 >(0.0f, 0.0f, -0.5f);
        break;
    case 5:
        scale.z = rate;
        scale.x = 1.0f;
        scale.y = 1.0f;
        shift.set< f32 >(0.0f, 0.0f, 0.5f);
        break;
    }
    mPoints[0].x = 0.5f;
    mPoints[0].y = 0.5f;
    mPoints[0].z = -0.5f;
    mPoints[1].x = 0.5f;
    mPoints[1].y = 0.5f;
    mPoints[1].z = 0.5f;
    mPoints[2].x = -0.5f;
    mPoints[2].y = 0.5f;
    mPoints[2].z = 0.5f;
    mPoints[3].x = -0.5f;
    mPoints[3].y = 0.5f;
    mPoints[3].z = -0.5f;
    mPoints[4].x = 0.5f;
    mPoints[4].y = -0.5f;
    mPoints[4].z = -0.5f;
    mPoints[5].x = 0.5f;
    mPoints[5].y = -0.5f;
    mPoints[5].z = 0.5f;
    mPoints[6].x = -0.5f;
    mPoints[6].y = -0.5f;
    mPoints[6].z = 0.5f;
    mPoints[7].x = -0.5f;
    mPoints[7].y = -0.5f;
    mPoints[7].z = -0.5f;
    TVec3f center;
    mGrid->calcCellPos(&center, *pCell);
    TVec3f size(mGrid->mCellSize.x, mGrid->mCellSize.y, mGrid->mCellSize.z);
    for (s32 i = 0; i < 8; i++) {
        TVec3f* pPoint = &mPoints[i];
        pPoint->add(shift);
        pPoint->mul(scale);
        pPoint->sub(shift);
        pPoint->mul(size);
        pPoint->add(center);
        mGrid->localToWorld(pPoint, *pPoint);
    }
    mIsValid = true;
}

#pragma dont_inline on
bool RepeatBlockVolume::calcFace(TVec3f* pDst, s32 dir, const TVec3f& rOffset) {
    if (!mIsValid) {
        return false;
    }
    if (MR::isNearZero(rOffset, 0.001f)) {
        return false;
    }
    switch (dir) {
    case 0:
        pDst[0].set(mPoints[0]);
        pDst[1].set(mPoints[1]);
        pDst[2].set(mPoints[5]);
        pDst[3].set(mPoints[4]);
        break;
    case 1:
        pDst[0].set(mPoints[2]);
        pDst[1].set(mPoints[3]);
        pDst[2].set(mPoints[7]);
        pDst[3].set(mPoints[6]);
        break;
    case 2:
        pDst[0].set(mPoints[1]);
        pDst[1].set(mPoints[0]);
        pDst[2].set(mPoints[3]);
        pDst[3].set(mPoints[2]);
        break;
    case 3:
        pDst[0].set(mPoints[4]);
        pDst[1].set(mPoints[5]);
        pDst[2].set(mPoints[6]);
        pDst[3].set(mPoints[7]);
        break;
    case 4:
        pDst[0].set(mPoints[2]);
        pDst[1].set(mPoints[6]);
        pDst[2].set(mPoints[5]);
        pDst[3].set(mPoints[1]);
        break;
    case 5:
        pDst[0].set(mPoints[0]);
        pDst[1].set(mPoints[4]);
        pDst[2].set(mPoints[7]);
        pDst[3].set(mPoints[3]);
        break;
    }
    for (s32 i = 0; i < 4; i++) {
        TVec3f tmp(pDst[i]);
        tmp.add(rOffset);
        pDst[i + 4] = tmp;
    }
    return true;
}

#pragma dont_inline reset
RepeatBlockShadowDrawer::~RepeatBlockShadowDrawer() {
}
