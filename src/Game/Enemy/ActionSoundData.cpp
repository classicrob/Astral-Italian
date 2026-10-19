#pragma auto_inline off
#include "Game/Enemy/ActionSoundData.hpp"
#include "Game/GameAudio/ActionSoundInfo.hpp"
#include "Game/System/ResourceCsvGroupHolder.hpp"
#include "Game/AudioLib/AudInstListMgr.hpp"
#include "Game/Util/SystemUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/FileUtil.hpp"
#include "Game/Util/JMapInfo.hpp"
#include "Game/Util/StringUtil.hpp"
#include <JSystem/JKernel/JKRMemArchive.hpp>

namespace {
    const char* sArchivePath = "/AudioRes/Info/ActionSound.arc";
    const char* sTablePath = "ActionSound.bcsv";
}

ActionSoundData* createActionSoundData(const char* name) {
    const ResourceCsvGroupInfo* range = MR::getGameSystemObjHolder3C()->findGroup(name);
    if (!range || range->mCount <= 0) return nullptr;
    return new ActionSoundData(name, range);
}

ActionSoundData::ActionSoundData(const char*, const ResourceCsvGroupInfo* range)
    : mName(nullptr), mInfos(nullptr), mCount(range->mCount) {
    JMapInfo csv;
    csv.attach(MR::mountArchive(sArchivePath, nullptr, false)->getResource(sTablePath));
    mInfos = new ActionSoundInfo*[mCount];
    for (s32 i = 0; i < mCount; i++) {
        s32 row = range->mStartIndex + i;
        JMapInfoIter iter(&csv, row);
        mInfos[i] = new ActionSoundInfo(iter, this);
        if (i == 0) MR::getCsvDataStrOrNULL(&mName, &csv, "ObjName", row);
    }
}

ActionSoundInfo* ActionSoundData::getInfo(s32 index) const {
    return mInfos[index];
}

ActionSoundInfo* ActionSoundData::findInfoByName(const char* name) const {
    ActionSoundInfo* info;
    for (s32 i = 0; i < mCount; i++) {
        info = mInfos[i];
        if (MR::isEqualString(name, info->mName)) return info;
    }
    return nullptr;
}

const char* getArchivePath() {
    return sArchivePath;
}

void AudInstListMgr::fn_802204A0(const char* name, bool async) {
    const ResourceCsvGroupInfo* range = MR::getGameSystemObjHolder3C()->findGroup(name);
    JMapInfo csv;
    csv.attach(MR::mountArchive(sArchivePath, nullptr, false)->getResource(sTablePath));
    s32 count = range->mCount;
    for (s32 i = 0; i < count; i++) {
        JMapInfoIter iter(&csv, range->mStartIndex + i);
        iter.getValueString("ObjName");
        s32 type = iter.getValueS32("SoundType");
        if (ActionSoundInfo::fn_802208A0(type)) {
            requestSound(iter.getValueString("SoundLabel"), async);
        }
    }
}
