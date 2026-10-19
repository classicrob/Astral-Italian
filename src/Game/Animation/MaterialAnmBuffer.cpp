#include "Game/Animation/MaterialAnmBuffer.hpp"
#include "Game/System/ResourceHolder.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include <JSystem/J3DGraphAnimator/J3DMaterialAnm.hpp>
#include <JSystem/J3DGraphAnimator/J3DModelData.hpp>
#include <JSystem/J3DGraphBase/J3DMaterial.hpp>
#include <JSystem/J3DGraphBase/J3DPacket.hpp>

template < typename T >
void modifyDiffFlag(u32* pFlags, const T* pAnm, J3DDiffFlag flag, bool enable, const char* pType, const char* pResourceName, const char* pName) {
    for (u16 i = 0; i < pAnm->getUpdateMaterialNum(); i++) {
        u16 material = pAnm->getUpdateMaterialID(i);

        if (material != 0xFFFF) {
            if (enable) {
                pFlags[material] |= flag;
            } else {
                pFlags[material] &= ~flag;
            }
        }
    }
}

void modifyDiffFlagBrk(u32*, const J3DAnmTevRegKey*, bool, const char*, const char*);

MaterialAnmBuffer::MaterialAnmBuffer(const ResourceHolder* pResourceHolder, J3DModelData* pModelData, bool useDiffFlag) : mMaterialAnimations(), mDiffFlags() {
    searchUpdateMaterialID(pResourceHolder, pModelData);
    u16 materialNum = pModelData->getMaterialNum();

    if (useDiffFlag) {
        mDiffFlags = new u32[materialNum];
        MR::zeroMemory(mDiffFlags, sizeof(u32) * materialNum);
        setDiffFlag(pResourceHolder);
    }

    u16 anmNum = getAllocMaterialAnmNum(pModelData, useDiffFlag);
    mMaterialAnimations = new J3DMaterialAnm[anmNum];
    attachMaterialAnmBuffer(pModelData, useDiffFlag);
}

u32 MaterialAnmBuffer::getDiffFlag(s32 index) const {
    return mDiffFlags[index];
}

u16 MaterialAnmBuffer::getAllocMaterialAnmNum(J3DModelData* pModelData, bool useDiffFlag) const {
    if (useDiffFlag) {
        return getDifferedMaterialNum(pModelData);
    }

    return pModelData->getMaterialNum();
}

void MaterialAnmBuffer::searchUpdateMaterialID(const ResourceHolder* pResourceHolder, J3DModelData* pModelData) {
    const ResTable* pBpkTable = pResourceHolder->mBpkResTable;

    for (u32 i = 0; i < pBpkTable->mCount; i++) {
        static_cast< J3DAnmColorKey* >(pBpkTable->getRes(i))->searchUpdateMaterialID(pModelData);
    }

    const ResTable* pBtpTable = pResourceHolder->mBtpResTable;

    for (u32 i = 0; i < pBtpTable->mCount; i++) {
        static_cast< J3DAnmTexPattern* >(pBtpTable->getRes(i))->searchUpdateMaterialID(pModelData);
    }

    const ResTable* pBtkTable = pResourceHolder->mBtkResTable;

    for (u32 i = 0; i < pBtkTable->mCount; i++) {
        static_cast< J3DAnmTextureSRTKey* >(pBtkTable->getRes(i))->searchUpdateMaterialID(pModelData);
    }

    const ResTable* pBrkTable = pResourceHolder->mBrkResTable;

    for (u32 i = 0; i < pBrkTable->mCount; i++) {
        static_cast< J3DAnmTevRegKey* >(pBrkTable->getRes(i))->searchUpdateMaterialID(pModelData);
    }
}

// NONMATCHING: register allocation differs; material animation checks and archive arguments match.
void MaterialAnmBuffer::setDiffFlag(const ResourceHolder* pResourceHolder) {
    const char* pBpkName;
    const ResTable* pBpkTable = pResourceHolder->mBpkResTable;

    for (u32 iBpk = 0; iBpk < pBpkTable->mCount; iBpk++) {
        pBpkName = pBpkTable->getResName(iBpk);
        const char* pResourceName = pResourceHolder->mName;
        const J3DAnmColorKey* pAnm = static_cast< J3DAnmColorKey* >(pBpkTable->getRes(iBpk));
        MR::onDiffFlagBpk(mDiffFlags, pAnm, pResourceName, pBpkName);
    }

    u32 iBtp;
    const ResTable* pBtpTable = pResourceHolder->mBtpResTable;

    for (iBtp = 0; iBtp < pBtpTable->mCount; iBtp++) {
        const char* pBtpName = pBtpTable->getResName(iBtp);
        const char* pResourceName = pResourceHolder->mName;
        const J3DAnmTexPattern* pAnm = static_cast< J3DAnmTexPattern* >(pBtpTable->getRes(iBtp));
        MR::onDiffFlagBtp(mDiffFlags, pAnm, pResourceName, pBtpName);
    }

    u32 iBtk;
    const ResTable* pBtkTable = pResourceHolder->mBtkResTable;

    for (iBtk = 0; iBtk < pBtkTable->mCount; iBtk++) {
        const char* pBtkName = pBtkTable->getResName(iBtk);
        const char* pResourceName = pResourceHolder->mName;
        const J3DAnmTextureSRTKey* pAnm = static_cast< J3DAnmTextureSRTKey* >(pBtkTable->getRes(iBtk));
        MR::onDiffFlagBtk(mDiffFlags, pAnm, pResourceName, pBtkName);
    }

    u32 iBrk;
    const ResTable* pBrkTable = pResourceHolder->mBrkResTable;

    for (iBrk = 0; iBrk < pBrkTable->mCount; iBrk++) {
        const char* pBrkName = pBrkTable->getResName(iBrk);
        const char* pResourceName = pResourceHolder->mName;
        const J3DAnmTevRegKey* pAnm = static_cast< J3DAnmTevRegKey* >(pBrkTable->getRes(iBrk));
        MR::onDiffFlagBrk(mDiffFlags, pAnm, pResourceName, pBrkName);
    }
}

u16 MaterialAnmBuffer::getDifferedMaterialNum(const J3DModelData* pModelData) const {
    u16 materialNum = pModelData->getMaterialNum();
    u16 count = 0;

    for (u16 i = 0; i < materialNum; i++) {
        if (mDiffFlags[i] != 0) {
            count++;
        }
    }

    return count;
}

void MaterialAnmBuffer::attachMaterialAnmBuffer(J3DModelData* pModelData, bool useDiffFlag) {
    u32 anmIndex = 0;

    for (u16 i = 0; i < pModelData->getMaterialNum(); i++) {
        if (!useDiffFlag || mDiffFlags[i] != 0) {
            pModelData->getMaterialNodePointer(i)->mMaterialAnm = &mMaterialAnimations[anmIndex];
            anmIndex++;
        }
    }
}

void modifyDiffFlagBrk(u32* pFlags, const J3DAnmTevRegKey* pAnm, bool enable, const char* pResourceName, const char* pName) {
    for (u16 i = 0; i < pAnm->getCRegUpdateMaterialNum(); i++) {
        u16 material = pAnm->getCRegUpdateMaterialID(i);

        if (material != 0xFFFF) {
            if (enable) {
                pFlags[material] |= J3DDiffFlag_TevReg;
            } else {
                pFlags[material] &= ~J3DDiffFlag_TevReg;
            }
        }
    }

    for (u16 i = 0; i < pAnm->getKRegUpdateMaterialNum(); i++) {
        u16 material = pAnm->getKRegUpdateMaterialID(i);

        if (material != 0xFFFF) {
            if (enable) {
                pFlags[material] |= J3DDiffFlag_TevReg;
            } else {
                pFlags[material] &= ~J3DDiffFlag_TevReg;
            }
        }
    }
}

namespace MR {
    void onDiffFlagBpk(u32* pFlags, const J3DAnmColorKey* pAnm, const char* pResourceName, const char* pName) {
        modifyDiffFlag(pFlags, pAnm, J3DDiffFlag_MatColor, true, "bpk", pResourceName, pName);
    }

    void offDiffFlagBpk(u32* pFlags, const J3DAnmColorKey* pAnm, const char* pName) {
        modifyDiffFlag(pFlags, pAnm, J3DDiffFlag_MatColor, false, "bpk", pName, "");
    }

    void onDiffFlagBtp(u32* pFlags, const J3DAnmTexPattern* pAnm, const char* pResourceName, const char* pName) {
        modifyDiffFlag(pFlags, pAnm, static_cast< J3DDiffFlag >(0x20000), true, "btp", pResourceName, pName);
    }

    void offDiffFlagBtp(u32* pFlags, const J3DAnmTexPattern* pAnm, const char* pName) {
        modifyDiffFlag(pFlags, pAnm, static_cast< J3DDiffFlag >(0x20000), false, "btp", pName, "");
    }

    void onDiffFlagBtk(u32* pFlags, const J3DAnmTextureSRTKey* pAnm, const char* pResourceName, const char* pName) {
        modifyDiffFlag(pFlags, pAnm, static_cast< J3DDiffFlag >(0x200), true, "btk", pResourceName, pName);
    }

    void offDiffFlagBtk(u32* pFlags, const J3DAnmTextureSRTKey* pAnm, const char* pName) {
        modifyDiffFlag(pFlags, pAnm, static_cast< J3DDiffFlag >(0x200), false, "btk", pName, "");
    }

    void onDiffFlagBrk(u32* pFlags, const J3DAnmTevRegKey* pAnm, const char* pResourceName, const char* pName) {
        modifyDiffFlagBrk(pFlags, pAnm, true, pResourceName, pName);
    }

    void offDiffFlagBrk(u32* pFlags, const J3DAnmTevRegKey* pAnm, const char* pName) {
        modifyDiffFlagBrk(pFlags, pAnm, false, pName, "");
    }
};  // namespace MR
