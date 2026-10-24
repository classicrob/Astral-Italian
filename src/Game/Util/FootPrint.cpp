#define JGEOMETRY_VEC3_SUB2_NOINLINE
#include "Game/Util/FootPrint.hpp"

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include <cstdio>
#include <cstring>
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Util/Color.hpp"
#include "Game/Util/DirectDraw.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include <JSystem/JUtility/JUTTexture.hpp>
#include <revolution/gx/GXEnum.h>
#include <revolution/gx/GXStruct.h>
#include <JSystem/JGeometry/TPartition3.hpp>

#ifndef GALAXY_NATIVE
template void TVec3f::negate(const TVec3f&);
#endif
template void TPos3f::getTrans(TVec3f&) const;
template void JGeometry::TPartition3<f32>::set(const TVec3f&, const TVec3f&, const TVec3f&);
#ifndef GALAXY_NATIVE
template void TVec3f::sub(const TVec3f&, const TVec3f&);
#endif

namespace {
    static const s32 sFadeBufferCount = 10;
    static const s32 sFadeOutTime = 60;
};  // namespace

FootPrint::FootPrint(const char* pName, s32 amount, s32 drawType) : NameObj(pName) {
    initMember(amount, drawType);
}

FootPrint::FootPrint(const char* pName, s32 amount) : NameObj(pName) {
    initMember(amount, MR::DrawType_0x17);
}

void FootPrint::initMember(s32 amount, s32 drawType) {
    _C = nullptr;
    mPrints = nullptr;
    mPrintMaxNum = 0;
    mPrintNum = 0;
    mRotatingPrintIndex = 0;

#ifdef GALAXY_NATIVE
    // Wii leaves this byte untouched until addPrint/clear. Start in the normal
    // print mode before movement can read uninitialised host heap contents.
    _3C = false;
#endif

    _2C = 20.0f;
    _30 = 20.0f;
    _34 = 20.0f;
    mMinPrintDistance = 80.0f;

    mLastPrintPos.zero();

    mPrints = new FootPrintInfo[amount];
    mPrintMaxNum = amount;

    for (int i = 0; i < mPrintMaxNum; i++) {
        mPrints[i].mIsValid = false;
    }

    MR::connectToScene(this, MR::MovementType_MapObj, MR::CalcAnimType_None, MR::DrawBufferType_None, drawType);
}

FootPrintInfo::FootPrintInfo() {
}

void FootPrint::setTexture(ResTIMG* img) {
    MR::CurrentHeapRestorer heapRestorer = MR::CurrentHeapRestorer(reinterpret_cast< JKRHeap* >(MR::getSceneHeapGDDR3()));
    _C = new JUTTexture(img, 0);
}

void FootPrint::movement() {
    if (_3C) {
        for (int i = 0; i < mPrintNum; i++) {
            if (!mPrints[i].mIsValid) {
                continue;
            }

            mPrints[i].mTimeLeft--;

            if (mPrints[i].mTimeLeft <= 0) {
                mPrints[i].mIsValid = false;
            }
        }

        return;
    }

    for (s32 i = mRotatingPrintIndex; i < mRotatingPrintIndex + ::sFadeBufferCount; i++) {
        s32 rotatedIndex = i;
        if (i >= mPrintMaxNum) {
            rotatedIndex -= mPrintMaxNum;
        }

        if (!mPrints[rotatedIndex].mIsValid) {
            continue;
        }

        mPrints[rotatedIndex].mTimeLeft--;

        if (mPrints[rotatedIndex].mTimeLeft <= 0) {
            mPrints[rotatedIndex].mIsValid = false;
        }
    }
}

bool FootPrint::addPrint(const TVec3f& rPos, const TVec3f& rArg2, const TVec3f& rArg3, bool arg4) {
    _3C = false;

    if (mPrintNum > 0 && rPos.distance(mLastPrintPos) < mMinPrintDistance) {
        return false;
    }

    mLastPrintPos = rPos;
    mPrints[mRotatingPrintIndex].mPos = rPos;
    mPrints[mRotatingPrintIndex]._C = rArg2;
    mPrints[mRotatingPrintIndex]._18 = rArg3;
    mPrints[mRotatingPrintIndex].mTimeLeft = ::sFadeOutTime;
    mPrints[mRotatingPrintIndex].mIsValid = true;
    mPrints[mRotatingPrintIndex]._29 = arg4;

    mRotatingPrintIndex++;
    mPrintNum++;

    if (mRotatingPrintIndex >= mPrintMaxNum) {
        mRotatingPrintIndex -= mPrintMaxNum;
    }

    if (mPrintNum > mPrintMaxNum) {
        mPrintNum = mPrintMaxNum;
    }

    return true;
}

void FootPrint::draw() const {
    if (mPrintNum <= 0) {
        return;
    }

    TDDraw::setup(2, 1, 0);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
    _C->load(GX_TEXMAP0);

    Color8 color(0, 0, 0, 255);

    for (int i = 0; i < mPrintNum; i++) {
        if (!mPrints[i].mIsValid) {
            continue;
        }

        color.a = 255.0f * MR::normalize(mPrints[i].mTimeLeft, 0.0f, ::sFadeOutTime);
        GXSetTevColor(GX_TEVREG0, color);

        TVec3f cross = mPrints[i]._C.cross(mPrints[i]._18);
        TVec3f v1;
        if ((i & 1) != 0) {
            v1 = mPrints[i].mPos + cross * _2C;
        } else {
            v1 = mPrints[i].mPos - cross * _2C;
        }

        v1 += mPrints[i]._18 * 5.0f;

        TDDraw::drawTexture3D(v1, mPrints[i]._18, mPrints[i]._C, _34, _30, nullptr, mPrints[i]._29 != false, false);
    }

    TDDraw::close();
}

void FootPrint::clear() {
    _3C = true;
}

void FootPrint::clearForce() {
    mPrintNum = 0;
    mRotatingPrintIndex = 0;
}

TVec3f* FootPrint::getPrintPos(u32 index) const {
    return &mPrints[index % mPrintMaxNum].mPos;
}

void FootPrint::invalidate(u32 index) {
    mPrints[index % mPrintMaxNum].mIsValid = false;
}

bool FootPrint::isValid(u32 index) const {
    return mPrints[index % mPrintMaxNum].mIsValid != false;
}

namespace {
    const char* sFootPrintControl = "FootPrintCtrl";
    const char* sFootPrintSuffix = "の足跡";
    const char* sFootPrintTexture = "FootPrint.bti";
}

namespace MR {
    // NONMATCHING: register/stack or instruction/branch lowering (FP operand order/grouping checked).
    FootPrint* createFootPrintHandle(LiveActor* actor) {
        FootPrint* print;
        u32 suffixLength;
        u32 length;
        char* name;
        ResTIMG* texture = MR::getTexFromArc(sFootPrintTexture, actor);
        JMapInfo* csv = MR::tryCreateCsvParser(actor, "%s.bcsv", sFootPrintControl);
        if (!csv) {
            print = new FootPrint("ダミー足跡", 32);
            print->setTexture(texture);
            return print;
        }
        s32 count = 32;
        f32 sideOffset = 0.0f;
        f32 sideSize = 100.0f;
        f32 frontSize = 100.0f;
        f32 distanceMin = 10.0f;
        MR::getCsvDataS32(&count, csv, "FootNumMax", 0);
        MR::getCsvDataF32(&sideOffset, csv, "SideOffset", 0);
        MR::getCsvDataF32(&sideSize, csv, "SideSize", 0);
        MR::getCsvDataF32(&frontSize, csv, "FrontSize", 0);
        MR::getCsvDataF32(&distanceMin, csv, "DistanceMin", 0);
        suffixLength = strlen(sFootPrintSuffix);
        length = suffixLength + strlen(actor->mName) + 1;
        name = new char[length];
        snprintf(name, length, "%s%s", actor->mName, sFootPrintSuffix);
        print = new FootPrint(name, count);
        print->setTexture(texture);
        print->_2C = sideOffset;
        print->_30 = sideSize;
        print->_34 = frontSize;
        print->mMinPrintDistance = distanceMin;
        return print;
    }

    // NONMATCHING: register/stack or instruction/branch lowering (FP operand order/grouping checked).
    bool addFootPrintOnGround(FootPrint* handle, LiveActor* actor, const TVec3f* pos, const TVec3f* front, bool right, f32 distance) {
        TVec3f groundPos(0.0f);
        TVec3f normal;
        normal.negate(actor->mGravity);
        TVec3f ray = actor->mGravity * distance;
        if (MR::getFirstPolyNormalOnLineToMap(&normal, *pos, ray, &groundPos, nullptr)) {
            handle->addPrint(groundPos, *front, normal, right);
            return true;
        }
        return false;
    }

    void addFootPrintToPos(FootPrint* handle, LiveActor* actor, const TVec3f* pos, bool right, f32 distance) {
        TVec3f front(0.0f, 0.0f, 1.0f);
        MR::calcFrontVec(&front, actor);
        addFootPrintOnGround(handle, actor, pos, &front, right, distance);
    }

    void addFootPrintToJoint(FootPrint* handle, LiveActor* actor, const char* joint, bool right, f32 distance) {
        MtxPtr matrix = MR::getJointMtx(actor, joint);
        TVec3f front(matrix[0][0], matrix[1][0], matrix[2][0]);
        TVec3f pos(matrix[0][3], matrix[1][3], matrix[2][3]);
        addFootPrintOnGround(handle, actor, &pos, &front, right, distance);
    }
}

// These base virtuals are empty in the target and are emitted in this unit.
void NameObj::endMovement() {
}

void NameObj::startMovement() {
}
