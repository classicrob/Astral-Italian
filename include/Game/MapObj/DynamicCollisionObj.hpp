#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class CollisionParts;
struct KCLFile;

class DynamicCollisionObj : public LiveActor {
public:
    class TriangleIndexing {
    public:
        inline TriangleIndexing() {
        }

        inline void set(u16 a, u16 b, u16 c) {
            mIndex[0] = a;
            mIndex[1] = b;
            mIndex[2] = c;
        }

        u16 mIndex[3];
    };

    DynamicCollisionObj(const char* pName);

    virtual ~DynamicCollisionObj() {
    }

    virtual void init(const JMapInfoIter& rIter);

    void syncCollision();
    void updateTriangle();
    void updateCollisionHeader();
    void createCollision();

    /* 0x90 */ KCLFile* mKCLFile;
    /* 0x94 */ CollisionParts* mParts;
    /* 0x98 */ u16 mTriangleNum;
    /* 0x9A */ u16 mPositionNum;
    /* 0x9C */ TVec3f* mPositions;
    /* 0xA0 */ TVec3f* mFaceNormals;
    /* 0xA4 */ TriangleIndexing* mIndices;
    /* 0xA8 */ u32 _A4;
    /* 0xAC */ u32 _A8;
    /* 0xB0 */ u32 _AC;
    /* 0xB4 */ u8 _B0[0x6C];
    /* 0x120 */ u32 _11C;
    /* 0x124 */ u32 _124;  // SMG2
};
