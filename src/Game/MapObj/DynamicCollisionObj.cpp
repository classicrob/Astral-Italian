#include "Game/MapObj/DynamicCollisionObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/CollisionParts.hpp"
#include "Game/Map/KCollision.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Util.hpp"
#include <new>

DynamicCollisionObj::DynamicCollisionObj(const char* pName)
    : LiveActor(pName), mKCLFile(nullptr), mParts(nullptr), _94(0), mPositionNum(0), mPositions(nullptr), _9C(nullptr),
      mIndices(nullptr), _A4(0), _A8(0), _AC(0) {}

void DynamicCollisionObj::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::connectToScene(this, MR::MovementType_Enemy, MR::CalcAnimType_Enemy, MR::DrawBufferType_None, MR::DrawType_FlexibleSphere);
}

void DynamicCollisionObj::syncCollision() {
    updateCollisionHeader();
    updateTriangle();
    mParts->mServer->calcFarthestVertexDistance();
    mParts->updateBoundingSphereRange(mScale);
}

// NONMATCHING: regalloc only; the target recomputes i * sizeof(TVec3f) for mPos[i] instead of reusing the _9C[i] offset
void DynamicCollisionObj::updateTriangle() {
    s32 normalidx = 0;

    for (u16 i = 0; i < _94; i++) {
        TVec3f Vertex1(mPositions[mIndices[i].mIndex[0]]);
        TVec3f Vertex2(mPositions[mIndices[i].mIndex[1]]);
        TVec3f Vertex3(mPositions[mIndices[i].mIndex[2]]);

        TVec3f a(Vertex3);
        a -= Vertex1;
        TVec3f SaveForLater(a);
        TVec3f b(Vertex2);
        b -= Vertex1;
        TVec3f c(Vertex3);
        c -= Vertex2;
        MR::normalizeOrZero(&a);
        MR::normalizeOrZero(&b);
        MR::normalizeOrZero(&c);
        TVec3f cross = b.cross(a);
        MR::normalizeOrZero(&cross);
        _9C[i] = cross;

        if (mKCLFile == nullptr) {
            continue;
        }

        TVec3f cross2 = (-a).cross(cross);
        TVec3f cross3 = b.cross(cross);
        TVec3f cross4 = c.cross(cross);

        MR::normalizeOrZero(&cross2);
        MR::normalizeOrZero(&cross3);
        MR::normalizeOrZero(&cross4);

        mKCLFile->mPos[i] = Vertex1;
        mKCLFile->mNorms[(u16)normalidx] = cross;
        s32 edge0 = (u16)normalidx + 1;
        mKCLFile->mNorms[edge0] = cross2;
        s32 edge1 = (u16)normalidx + 2;
        mKCLFile->mNorms[edge1] = cross3;
        s32 edge2 = (u16)normalidx + 3;
        mKCLFile->mNorms[edge2] = cross4;

        s32 attr = i + 1;
        KC_PrismData* prism = &mKCLFile->mPrisms[attr];
        MR::vecKillElement(SaveForLater, c, &SaveForLater);

        prism->mHeight = SaveForLater.length();
        prism->mPositionIndex = i;
        prism->mNormalIndex = normalidx;
        normalidx += 4;
        prism->mEdgeIndices[0] = edge0;
        prism->mEdgeIndices[1] = edge1;
        prism->mEdgeIndices[2] = edge2;
        prism->mAttribute = attr;
    }
}

void DynamicCollisionObj::updateCollisionHeader() {
    // Compute bounding box and extent
    TVec3f min, max;
    MR::createBoundingBox(this->mPositions, this->mPositionNum, &min, &max);
    TVec3f extent(max);
    extent -= min;

    // Initialize masks with extent
    u32 masks[3];
    masks[0] = (s32)extent.x;
    masks[1] = (s32)extent.y;
    masks[2] = (s32)extent.z;
    if (!masks[0]) {
        masks[0] = 1;
    }

    if (!masks[1]) {
        masks[1] = 1;
    }

    if (!masks[2]) {
        masks[2] = 1;
    }

    // Find highest number of bits to shift
    u32 max_entropy = 0;
    for (u32 component = 0; component < 3; component++) {
        u32 bits_sel = 0x80000000;
        u32 mask_sel = 0xFFFFFFFF;
        u32 val = masks[component];
        u32 i = 0;

        for (u32 bit = 0; bit < 32; bit++) {
            if ((val & bits_sel)) {
                masks[component] = ~mask_sel;
                if (max_entropy < i) {
                    max_entropy = i;
                }

                break;
            }

            bits_sel >>= 1;
            mask_sel >>= 1;
            i++;
        }
    }

    // Update header fields
    u32 bit_shift = 33 - max_entropy;
    u32 area_y_width_mask = masks[1];
    this->mKCLFile->mXMask = masks[0];
    u32 area_z_width_mask = masks[2];
    this->mKCLFile->mYMask = area_y_width_mask;
    this->mKCLFile->mZMask = area_z_width_mask;
    this->mKCLFile->mBlockWidthShift = bit_shift;
    this->mKCLFile->mMin = min;
}

void DynamicCollisionObj::createCollision() {
    s32 v = _94;
    u32 posSize = v * sizeof(TVec3f);
    u32 normSize = v * (4 * sizeof(TVec3f));
    u32 octSize = (v + 3) * sizeof(u16);
    u32 prismSize = (v + 1) * sizeof(KC_PrismData);
    u8* buf = new (4) u8[prismSize + posSize + (octSize + normSize)];
    mKCLFile = new KCLFile();
    mKCLFile->mPos = reinterpret_cast<TVec3f*>(buf);
    mKCLFile->mNorms = reinterpret_cast<TVec3f*>(reinterpret_cast<u8*>(mKCLFile->mPos) + posSize);
    mKCLFile->mPrisms = reinterpret_cast<KC_PrismData*>(reinterpret_cast<u8*>(mKCLFile->mNorms) + normSize);
    u16* u16array = reinterpret_cast<u16*>(reinterpret_cast<u8*>(mKCLFile->mPrisms) + prismSize);
    mKCLFile->mOctree = u16array;

    u16array[0] = 0x8000;
    u16array[1] = 2;

    s32 count = v;
    for (s32 i = 0; i <= v; i++) {
        u16array[i + 2] = count--;
    }

    mKCLFile->mThickness = 40.f;
    mKCLFile->mBlockXShift = -1;
    mKCLFile->mBlockXYShift = -1;
    updateCollisionHeader();
    updateTriangle();

    mParts = new CollisionParts();
    TPos3f posmtx;
    PSMTXTrans(posmtx, mPosition.x, mPosition.y, mPosition.z);
    mParts->init(posmtx, getSensor("body"), mKCLFile, nullptr, 0, true);
    MR::validateCollisionParts(mParts);
    mParts->mServer->calcFarthestVertexDistance();
    mParts->updateBoundingSphereRange(TVec3f(mScale));
}
