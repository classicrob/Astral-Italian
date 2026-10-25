#include "Game/Player/MarioAllModel.hpp"
#include "Game/Player/DLchanger.hpp"
#include "Game/Player/J3DModelX.hpp"
#include "Game/Player/Mario.hpp"
#include "Game/Screen/FullScreenBlur.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/SchedulerUtil.hpp"
#include <JSystem/JUtility/JUTTexture.hpp>
#include <revolution/gd.h>
#include <revolution/os.h>

MarioModelDL::MarioModelDL(LiveActor* actor, const char* material, const char* texture) {
    mModel = nullptr;
    mMaterialIndex = 0;
    mDisplayLists = nullptr;
    mModel = static_cast<J3DModelX*>(MR::getJ3DModel(actor));
    mMaterialIndex = MR::getMaterialNo(MR::getJ3DModelData(actor), material);
    s32 textureIndex = MR::getTextureIndex(actor, texture);
    mDisplayLists = new DLholder[4];
    DLholder* holder;
    u32 alignedSize;
    for (s32 i = 0; i < 4; ++i) {
        holder = &mDisplayLists[i];
        MR::ProhibitSchedulerAndInterrupts guard(false);
        GDLObj object;
        u8 buffer[512] ATTRIBUTE_ALIGN(32);
        GDInitGDLObj(&object, buffer, sizeof(buffer));
        __GDCurrentDL = &object;
        ResTIMG* image = MR::getResTIMG(actor, static_cast<u16>(textureIndex + i));
        GDSetTexImgPtr(GX_TEXMAP0, reinterpret_cast<u8*>(image) + JUT_RESTIMG_OFFSET(image->mImageDataOffset));
        GDPadCurr32();
        u32 size = object.ptr - object.start;
        holder->mSize = size;
        alignedSize = (size + 31) & ~31;
        holder->mDL = new (32) u8[alignedSize];
        MR::copyMemory(holder->mDL, buffer, alignedSize);
        DCStoreRange(holder->mDL, alignedSize);
    }
}

inline u32 getDisplayListMaterial(const MarioModelDL& display) {
    return display.mMaterialIndex;
}

void MarioModelDL::setDisplayList(u32 index) {
    J3DModelX* model;
    u8* list;
    u16 size;
    DLholder* holders = mDisplayLists;
    u32 material = getDisplayListMaterial(*this);
    list = holders[index].mDL;
    size = holders[index].mSize;
    model = mModel;
    if (!list) size = 0;
    model->_1C8[static_cast<u16>(material)] = list;
    model->_1CC[static_cast<u16>(material)] = size;
}

MarioScreenBlur::MarioScreenBlur() {
    mEnabled = 0;
    mStrength = 0.0f;
    mScale = 0.0f;
    mAlpha = 0;
    mMode = 0;
}

void MarioScreenBlur::draw() const {
    if (mEnabled) MR::drawFullScreenBlur(mStrength, mScale, mAlpha, mMode);
}

MarioFrontHistory::MarioFrontHistory(Mario* mario) {
    mValues = nullptr;
    mIndex = 0;
    mCount = 1;
    mMario = mario;
    mValues = new TVec3f[30];
    for (u32 i = 0; i < 30; ++i) {
        TVec3f* value = &mValues[i];
        value->x = 1.0f;
        value->y = 0.0f;
        value->z = 0.0f;
    }
}

inline TVec3f& getFrontHistorySlot(MarioFrontHistory& history) {
    return history.mValues[history.mIndex];
}

#pragma opt_common_subs off
void MarioFrontHistory::update() {
    getFrontHistorySlot(*this) = mMario->mFrontVec;
    mIndex = static_cast<u16>(mIndex + 1) % mCount;
}

#pragma opt_common_subs reset

void MarioFrontHistory::getFront(TVec3f* out) const {
    out->set(mValues[mIndex]);
}
