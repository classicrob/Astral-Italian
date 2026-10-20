#include "Game/System/SaveDataBannerCreator.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/System/NANDManager.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include "Game/Util/MessageUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include <JSystem/JKernel/JKRExpHeap.hpp>
#include <JSystem/JUtility/JUTTexture.hpp>
#include <cstdio>

namespace {
    NEW_NERVE(SaveDataBannerCreatorNoOperation, SaveDataBannerCreator, NoOperation);
    NEW_NERVE(SaveDataBannerCreatorCreateOnTemporary, SaveDataBannerCreator, CreateOnTemporary);
    NEW_NERVE(SaveDataBannerCreatorMoveToHomeDir, SaveDataBannerCreator, MoveToHomeDir);
};  // namespace

SaveDataBannerCreator::SaveDataBannerCreator() : NerveExecutor("BannerCreator"), mNANDRequestInfo(), mBanner(), _50() {
    mNANDRequestInfo = new NANDRequestInfo();

    initNerve(GET_NERVE_ANON(SaveDataBannerCreatorNoOperation));
    setupBannerInfo();
}

void SaveDataBannerCreator::execute() {
    _50 = false;
    setNerve(GET_NERVE_ANON(SaveDataBannerCreatorCreateOnTemporary));
}

void SaveDataBannerCreator::startMoveToHomeDir() {
    if (_50) {
        setNerve(GET_NERVE_ANON(SaveDataBannerCreatorMoveToHomeDir));
    }
}

bool SaveDataBannerCreator::isDone() const {
    return isNerve(GET_NERVE_ANON(SaveDataBannerCreatorNoOperation));
}

NANDResultCode SaveDataBannerCreator::getResultCode() const {
    return mNANDRequestInfo->mResult;
}

void SaveDataBannerCreator::exeNoOperation() {
}

void SaveDataBannerCreator::exeCreateOnTemporary() {
    if (MR::isFirstStep(this)) {
        mNANDRequestInfo->setWriteSeq("/tmp/banner.bin", mBanner, NAND_BANNER_SIZE(1),
                                      NAND_PERM_RGRP | NAND_PERM_WGRP | NAND_PERM_RUSR | NAND_PERM_WUSR, 0);
        MR::addRequestToNANDManager(mNANDRequestInfo);
    }

    if (!mNANDRequestInfo->isDone()) {
        return;
    }

    NANDResultCode resultCode = mNANDRequestInfo->mResult;

    if (resultCode.isSuccess()) {
        _50 = true;
        setNerve(GET_NERVE_ANON(SaveDataBannerCreatorNoOperation));
    } else {
        setNerve(GET_NERVE_ANON(SaveDataBannerCreatorNoOperation));
    }
}

void SaveDataBannerCreator::exeMoveToHomeDir() {
    if (MR::isFirstStep(this)) {
        mNANDRequestInfo->setMove("/tmp/banner.bin", mHomeDir);
        MR::addRequestToNANDManager(mNANDRequestInfo);
    }

    if (!mNANDRequestInfo->isDone()) {
        return;
    }

    _50 = false;

    NANDResultCode resultCode = mNANDRequestInfo->mResult;

    if (resultCode.isSuccess()) {
        setNerve(GET_NERVE_ANON(SaveDataBannerCreatorNoOperation));
    } else {
        setNerve(GET_NERVE_ANON(SaveDataBannerCreatorNoOperation));
        return;
    }
}

void SaveDataBannerCreator::setupBannerInfo() {
    mBanner = new (MR::getStationedHeapGDDR3(), 32) NANDBanner;

    const u16* title = reinterpret_cast< const u16* >(MR::getGameMessageDirect("TITLE_MAIN"));
    const u16* comment = reinterpret_cast< const u16* >(MR::getGameMessageDirect("TITLE_SUB"));
    NANDInitBanner(mBanner, NAND_BANNER_FLAG_ANIM_LOOP, title, comment);

    const ResTIMG* bannerTex = MR::loadTexFromArc("SaveIconBanner.arc", "SaveBanner.bti", nullptr);
    MR::copyMemory(mBanner->bannerTexture, reinterpret_cast< const u8* >(bannerTex) + JUT_RESTIMG_OFFSET(bannerTex->mImageDataOffset), NAND_BANNER_TEXTURE_SIZE);

    for (s32 i = 0; i < 1; i++) {
        char iconTexName[32];
        snprintf(iconTexName, sizeof(iconTexName), "SaveIcon%02d.bti", i);

        const ResTIMG* iconTex = MR::loadTexFromArc("SaveIconBanner.arc", iconTexName, nullptr);
        MR::copyMemory(mBanner->iconTexture[i], reinterpret_cast< const u8* >(iconTex) + JUT_RESTIMG_OFFSET(iconTex->mImageDataOffset), NAND_BANNER_ICON_SIZE);

        NANDSetIconSpeed(mBanner, i, NAND_BANNER_ICON_ANIM_SPEED_SLOW);
    }

    NANDSetIconSpeed(mBanner, 1, NAND_BANNER_ICON_ANIM_SPEED_END);
    NANDGetHomeDir(mHomeDir);
}
