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
    s32 textureIndex = MR::fn_80043AB0(actor, texture);
    mDisplayLists = new DLholder[4];
    for (s32 i = 0; i < 4; ++i) {
        DLholder* holder = &mDisplayLists[i];
        MR::ProhibitSchedulerAndInterrupts guard(false);
        GDLObj object;
        u8 buffer[512] ATTRIBUTE_ALIGN(32);
        GDInitGDLObj(&object, buffer, sizeof(buffer));
        __GDCurrentDL = &object;
        ResTIMG* image = MR::getResTIMG(actor, static_cast<u16>(textureIndex + i));
        GDSetTexImgPtr(GX_TEXMAP0, reinterpret_cast<u8*>(image) + image->mImageDataOffset);
        GDPadCurr32();
        u32 size = object.ptr - object.start;
        holder->mSize = size;
        u32 alignedSize = (size + 31) & ~31;
        holder->mDL = new (32) u8[alignedSize];
        MR::copyMemory(holder->mDL, buffer, alignedSize);
        DCStoreRange(holder->mDL, alignedSize);
    }
}

void MarioModelDL::setDisplayList(u32 index) {
    DLholder* holders = mDisplayLists;
    u32 material = mMaterialIndex;
    u8* list = holders[index].mDL;
    u16 size = holders[index].mSize;
    J3DModelX* model = mModel;
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

void MarioFrontHistory::update() {
    mValues[mIndex] = mMario->mFrontVec;
    mIndex = static_cast<u16>(mIndex + 1) % mCount;
}

void MarioFrontHistory::getFront(TVec3f* out) const {
    out->set(mValues[mIndex]);
}
