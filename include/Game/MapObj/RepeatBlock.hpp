#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/ShadowVolumeDrawer.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include <JSystem/JGeometry.hpp>

class RepeatBlock;
class RepeatBlockAtom;
class RepeatBlockGrid;
class RepeatBlockHistory;

// Cell coordinate (x, y, z) on the block lattice.
#pragma push
#pragma auto_inline on
struct RepeatBlockCell {
    RepeatBlockCell() {
    }
    RepeatBlockCell(s16);
    RepeatBlockCell(s16, s16, s16);

    inline RepeatBlockCell(const RepeatBlockCell& rOther) {
        *((s32*)this) = *((const s32*)&rOther);
        mZ = rOther.mZ;
    }

    inline RepeatBlockCell& operator=(const RepeatBlockCell& rOther) {
        *((s32*)this) = *((const s32*)&rOther);
        mZ = rOther.mZ;
        return *this;
    }

    bool isEqual(const RepeatBlockCell&) const;

    inline bool operator==(const RepeatBlockCell& rOther) const {
        bool xy = false;
        bool xyz = false;
        if (mX == rOther.mX && mY == rOther.mY) {
            xy = true;
        }
        if (xy && mZ == rOther.mZ) {
            xyz = true;
        }
        return xyz;
    }

    static inline s16 signOf(s16 v) {
        if (v > 0) {
            return 1;
        }
        if (v < 0) {
            return -1;
        }
        return v;
    }

    static inline RepeatBlockCell sign(RepeatBlockCell c) {
        if (c.mX > 0) {
            c.mX = 1;
        } else if (c.mX < 0) {
            c.mX = -1;
        }
        if (c.mY > 0) {
            c.mY = 1;
        } else if (c.mY < 0) {
            c.mY = -1;
        }
        if (c.mZ > 0) {
            c.mZ = 1;
        } else if (c.mZ < 0) {
            c.mZ = -1;
        }
        return c;
    }

    static inline s32 absSum(RepeatBlockCell c) {
        return __abs(c.mX) + __abs(c.mY) + __abs(c.mZ);
    }

    s16 mX;  // 0x0
    s16 mY;  // 0x2
    s16 mZ;  // 0x4
};
#pragma pop

// mwcc refuses to inline a 3-statement helper that is used from several functions, so the cell difference is spelled out at each use.
#define REPEAT_BLOCK_CELL_SUB(dst, a, b) \
    RepeatBlockCell dst;           \
    *((s32*)&dst) = *((const s32*)&(a)); \
    dst.mZ = (a).mZ;               \
    dst.mX -= (b).mX;              \
    dst.mY -= (b).mY;              \
    dst.mZ -= (b).mZ;

// Node of the cell history: the cell plus, for each of the 6 directions, the index of the neighbour and the distance to it.
struct RepeatBlockHistoryNode {
    RepeatBlockHistoryNode() {
    }

    /* 0x00 */ RepeatBlockCell mCell;
    /* 0x08 */ s32 mNeighbor[6];
    /* 0x20 */ s32 mDistance[6];
};

class RepeatBlockHead : public LiveActor {
public:
    RepeatBlockHead(const char*, RepeatBlock*);
    virtual void appear();
    void updateAtomPositions();
    virtual ~RepeatBlockHead();
    virtual void kill();

    /* 0x90 */ TPos3f mBaseMtx;
    /* 0xC0 */ RepeatBlock* mParent;
    /* 0xC4 */ RepeatBlockAtom** mAtoms;
};

class RepeatBlockAtom : public LiveActor {
public:
    RepeatBlockAtom(RepeatBlock*);
    virtual ~RepeatBlockAtom();
    virtual void appear();
};

class RepeatBlockTail : public RepeatBlockHead {
public:
    RepeatBlockTail(RepeatBlock*, const char*);
    virtual ~RepeatBlockTail();
    virtual void appear();
    virtual void kill();

    void exeAppear();
    void exeWait() {
    }
    void exeAppearLoop();
    void forEachAtom(s32, s32, void (RepeatBlockTail::*)(RepeatBlockAtom*));
    void appearAtom(RepeatBlockAtom*);
    void startAtom(RepeatBlockAtom*);
    s32 calcAppearNum(s32) const;
    s32 getCellNum(s32) const;

    /* 0xC8 */ s32 mAppearDir;
    /* 0xCC */ s32 mMoveDir;
    /* 0xD0 */ f32 mAppearProgress;
    /* 0xD4 */ s32 mAppearStep;
};

class RepeatBlockGrid {
public:
    RepeatBlockGrid(const TPos3f&, f32, f32, f32);

    void calcCellPos(TVec3f*, const RepeatBlockCell&) const;
    void calcWorldCellPos(TVec3f*, const RepeatBlockCell&) const __attribute__((noinline));
    void calcCell(RepeatBlockCell*, const TVec3f&) const;
    void localToWorld(TVec3f*, const TVec3f&) const __attribute__((noinline));
    void worldToCell(RepeatBlockCell*, const TVec3f&) const;
    void getAxisX(TVec3f*) const;
    void getAxisY(TVec3f*) const;
    void getAxisZ(TVec3f*) const;

    /* 0x00 */ TPos3f mMtx;
    /* 0x30 */ TPos3f mInvMtx;
    /* 0x60 */ TVec3f mCellSize;
};

class RepeatBlockHistory {
public:
    RepeatBlockHistory(s32);

    void push(const RepeatBlockCell&);
    bool hasNeighbor(s32, s32) const;
    s32 getDistance(s32, s32) const;
    RepeatBlockHistoryNode* getNode(s32) const;
    s32 calcIndex(s32) const;
    void propagate();
    void connect();

    /* 0x0 */ s32 mCapacity;
    /* 0x4 */ s32 mCount;
    /* 0x8 */ s32 mHead;
    /* 0xC */ RepeatBlockHistoryNode* mNodes;
};

class RepeatBlockVolume {
public:
    RepeatBlockVolume(RepeatBlockGrid*);

    void setup(const RepeatBlockCell*, s32, f32);
    bool calcFace(TVec3f*, s32, const TVec3f&);

    /* 0x0 */ RepeatBlockGrid* mGrid;
    /* 0x4 */ bool mIsValid;
    /* 0x8 */ TVec3f mPoints[8];
};

class RepeatBlockShadowDrawer : public ShadowVolumeDrawer {
public:
    RepeatBlockShadowDrawer(RepeatBlockHistory*, RepeatBlockGrid*);

    virtual ~RepeatBlockShadowDrawer();

    virtual void loadModelDrawMtx() const;
    virtual void drawShape() const;
    virtual bool isDraw() const;

    void makeVertexBuffer();
    void makeFace();
    void calcDropOffset(TVec3f*) const;
    s32 calcDirection(const RepeatBlockCell*, const RepeatBlockCell*) const;
    void drawBox(const TVec3f*) const;
    s32 calcFaceLength(s32, s32) const;
    void makeHeadFace();
    void makeTailFace();
    void setDirection(s32);

    /* 0x24 */ RepeatBlockHistory* mHistory;
    /* 0x28 */ RepeatBlockGrid* mGrid;
    /* 0x2C */ TVec3f* mPoints;
    /* 0x30 */ s32 mPointNum;
    /* 0x34 */ s32 mDirection;
    /* 0x38 */ s32 mMaxLength;
    /* 0x3C */ RepeatBlockVolume* mVolume;
    /* 0x40 */ f32 mOffset;
    /* 0x44 */ TVec3f* mHeadPoints;
    /* 0x48 */ bool mHasHead;
    /* 0x4C */ TVec3f* mTailPoints;
    /* 0x50 */ bool mHasTail;
};

class RepeatBlock : public LiveActor {
public:
    RepeatBlock(const char*);

    virtual ~RepeatBlock();

    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void kill();
    virtual void makeActorDead();
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void exeWait() {
    }
    void exeMove();
    void exeStop();
    f32 getCellSize(s32) const;
    void initHead();
    void updateHead();
    void initTail();
    void updateTail();
    void initTails();
    void initHistory();
    void initGrid();
    void initShadow();
    void appearNextTail();
    void startTailMove();
    void killTails();
    void moveTails();
    bool stepForward();
    void calcMovedPos(TVec3f*, s32) const;
    bool calcArrive(TVec3f*, RepeatBlockCell*, s32);
    void initChildren(const JMapInfoIter&);
    bool startMove();
    bool checkShadowArea();
    s32 getBlockNum() const;

    /* 0x090 */ s16 mNumX;
    /* 0x092 */ s16 mNumY;
    /* 0x094 */ s16 mNumZ;
    /* 0x098 */ s32 mTailNum;
    /* 0x09C */ RepeatBlockHead* mHead;
    /* 0x0A0 */ RepeatBlockTail* mTail;
    /* 0x0A4 */ RepeatBlockTail** mTails;
    /* 0x0A8 */ s32 mTailIdx;
    /* 0x0AC */ TPos3f mMtx;
    /* 0x0DC */ TPos3f mInvMtx;
    /* 0x10C */ s32 _10C;
    /* 0x110 */ f32 mSpeed;
    /* 0x114 */ f32 _114;
    /* 0x118 */ f32 _118;
    /* 0x11C */ f32 _11C;
    /* 0x120 */ f32 mCellLength;
    /* 0x124 */ TVec3f mCellSize;
    /* 0x130 */ s32 mMaxNum;
    /* 0x134 */ s32 mDir;
    /* 0x138 */ RepeatBlockCell mCell;
    /* 0x13E */ RepeatBlockCell mRailCell;
    /* 0x144 */ RepeatBlockCell mNextCell;
    /* 0x14C */ s32 mCellStep;
    /* 0x150 */ f32 mCellRate;
    /* 0x154 */ s32* mDirHistory;
    /* 0x158 */ TVec3f* mPosHistory;
    /* 0x15C */ s32 mHistoryWrite;
    /* 0x160 */ s32 mHistoryRead;
    /* 0x164 */ bool mHistoryFull;
    /* 0x165 */ bool mTailsAppeared;
    /* 0x168 */ RepeatBlockHistory* mHistory;
    /* 0x16C */ RepeatBlockGrid* mGrid;
    /* 0x170 */ RepeatBlockShadowDrawer* mShadowDrawer;
    /* 0x174 */ RepeatBlock* mParent;
    /* 0x178 */ RepeatBlock** mChildren;
    /* 0x17C */ s32 mChildNum;
};
