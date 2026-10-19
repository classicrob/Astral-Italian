#include "Game/AudioLib/AudStageBgmTable.hpp"
#include "Game/AudioLib/AudWrap.hpp"
#include "Game/AudioLib/AudBgmParamTable.hpp"
#include "Game/AudioLib/AudSoundNameConverter.hpp"
#include "Game/Util.hpp"
#include "Game/Util/JMapInfo.hpp"
#include "Game/Util/FileUtil.hpp"
#include "JSystem/JKernel/JKRArchive.hpp"
#include <cstdio>

namespace AudBgmSetting {
    const char* sStageBgmResourceName = "/AudioRes/Info/StageBgmInfo.arc";
    const char* sStageBgmFileName = "StageBgmInfo.bcsv";
    const char* sScenarioBgmFileName = "ScenarioBgmInfo.bcsv";
    JKRArchive* getStageBgmArchive() NO_INLINE {
        JKRArchive* arc = MR::receiveArchive(sStageBgmResourceName);
        if (!arc)
            arc = MR::mountArchive(sStageBgmResourceName, 0, true);
        return arc;
    }
}  // namespace AudBgmSetting
AudBgmParam::AudBgmParam() : mSoundId(-1), mStartType(0), mStartFrame(0), mIsPrepare(false) {
    reset();
}
void AudBgmParam::reset() {
    mSoundId = -1;
    mStartType = 0;
    mStartFrame = 0;
    mIsPrepare = false;
}
AudBgmParamGroup::AudBgmParamGroup() : mParams(0), mStageName(0) {
    mParams = new AudBgmParam[11];
    reset();
}
void AudBgmParamGroup::reset() {
    for (s32 i = 0; i < 11; ++i)
        mParams[i].reset();
    for (s32 i = 0; i < 5; ++i)
        mChangeSoundIds[i] = -1;
    for (s32 i = 0; i < 5; ++i)
        mChangeStates[i] = -1;
}
AudBgmParam* AudBgmParamGroup::getParam(s32 scenario) {
    AudBgmParam* p = &mParams[scenario];
    if (p->mSoundId != -1U)
        return p;
    if (mParams[0].mSoundId != -1U)
        return mParams;
    return 0;
}
u32 AudBgmParamGroup::getSoundId(s32 scenario, s32 change) {
    if (change == 0)
        return getScenarioSoundId(scenario);
    change -= 1;
    return mChangeSoundIds[change];
}
u32 AudBgmParamGroup::getScenarioSoundId(s32 scenario) {
    u32 id = mParams[scenario].mSoundId;
    if (id == -1U)
        return mParams[0].mSoundId;
    return id;
}
AudBgmParamTable::AudBgmParamTable() : mGroups(0), mGroupCount(0) {
    mGroups = new AudBgmParamGroup[80];
}
void AudBgmParamTable::reset() {
    for (s32 i = 0; i < 80; ++i)
        mGroups[i].reset();
}
void AudBgmParamTable::load() {
    AudSoundNameConverter* converter = AudSoundNameConverter::get();
    reset();
    {
        JMapInfo info;
        info.attach(AudBgmSetting::getStageBgmArchive()->getResource(AudBgmSetting::sStageBgmFileName));
        mGroupCount = 0;
        for (JMapInfoIter i = info.begin(); i != info.end(); ++i.mIndex) {
            AudBgmParamGroup* group = &mGroups[mGroupCount];
            group->mStageName = i.getValueString("StageName");
            char column[256];
            for (s32 j = 0; j < 5; ++j) {
                snprintf(column, sizeof(column), "ChangeBgmIdName%d", j);
                const char* sound = i.getValueString(column);
                if (MR::isEqualString(sound, ""))
                    group->mChangeSoundIds[j] = -1;
                else
                    group->mChangeSoundIds[j] = converter->getSoundID(sound);
            }
            for (s32 j = 0; j < 5; ++j) {
                snprintf(column, sizeof(column), "ChangeBgmState%d", j);
                group->mChangeStates[j] = i.getValueS32(column);
            }
            ++mGroupCount;
        }
    }
    {
        JMapInfo info;
        info.attach(AudBgmSetting::getStageBgmArchive()->getResource(AudBgmSetting::sScenarioBgmFileName));
        const char* previous = 0;
        AudBgmParamGroup* group = 0;
        for (JMapInfoIter i = info.begin(); i != info.end(); ++i.mIndex) {
            const char* stage = i.getValueString("StageName");
            if (!previous || !MR::isEqualString(previous, stage)) {
                group = findGroup(stage);
                previous = stage;
            }
            s32 scenario = i.getValueS32("ScenarioNo");
            AudBgmParam* p = &group->mParams[scenario];
            const char* sound = i.getValueString("BgmIdName");
            if (MR::isEqualString(sound, ""))
                p->mSoundId = -1;
            else
                p->mSoundId = converter->getSoundID(sound);
            p->mStartType = i.getValueS32("StartType");
            p->mStartFrame = i.getValueS32("StartFrame");
            p->mIsPrepare = i.getValueBool("IsPrepare");
        }
    }
}
AudBgmParamGroup* AudBgmParamTable::findGroup(const char* stage) {
    for (s32 i = 0; i < mGroupCount; ++i)
        if (MR::isEqualString(mGroups[i].mStageName, stage))
            return &mGroups[i];
    return 0;
}
AudBgmParam* AudBgmParamTable::getParam(const char* stage, s32 scenario) {
    AudBgmParamGroup* group = findGroup(stage);
    if (group == 0)
        return 0;
    else
        return group->getParam(scenario);
}
u32 AudBgmParamTable::getSoundId(const char* stage, s32 scenario, s32 change) {
    AudBgmParamGroup* group = findGroup(stage);
    if (group == 0)
        return -1;
    else
        return group->getSoundId(scenario, change);
}
s32 AudBgmParamTable::getChangeState(const char* stage, s32 change) {
    AudBgmParamGroup* group = findGroup(stage);
    if (group == 0)
        return -1;
    else
        return group->mChangeStates[change];
}
u32 AudBgmParamTable::getScenarioSoundId(const char* stage, s32 scenario) {
    AudBgmParamGroup* group = findGroup(stage);
    if (group == 0)
        return -1;
    else
        return group->getScenarioSoundId(scenario);
}
namespace AudWrap {
    AudBgmParamTable* getBgmParamTable();
}
namespace AudBgmSetting {
    const char* getStageBgmResourceName() {
        return sStageBgmResourceName;
    }
    void loadStageBgmResource() {
        AudWrap::getBgmParamTable()->load();
    }
}  // namespace AudBgmSetting
namespace AudStageBgmWrap {
    u32 getStageBgmIDAtIndex(const char* stage, s32 scenario, s32 change) {
        return AudWrap::getBgmParamTable()->getSoundId(stage, scenario, change);
    }
    s32 getStageBgmVolumeIDAtIndex(const char* stage, s32 change) {
        return AudWrap::getBgmParamTable()->getChangeState(stage, change);
    }
    u32 changeStageNameToSoundID(const char*, const char* stage, s32 scenario) {
        return AudWrap::getBgmParamTable()->getScenarioSoundId(stage, scenario);
    }
}  // namespace AudStageBgmWrap
bool AudStageBgmTable::isStartOnWelcome(const char* stage, s32 scenario) {
    AudBgmParam* p = AudWrap::getBgmParamTable()->getParam(stage, scenario);
    if (p == 0)
        return false;
    else
        return p->mStartType == 1;
}
s32 AudStageBgmTable::getWelcomeStartOffset(const char* stage, s32 scenario) {
    AudBgmParam* p = AudWrap::getBgmParamTable()->getParam(stage, scenario);
    if (p == 0)
        return 0;
    else
        return p->mStartFrame;
}
bool AudStageBgmTable::isFirstVisitStart(const char* stage, s32 scenario) {
    AudBgmParam* p = AudWrap::getBgmParamTable()->getParam(stage, scenario);
    if (p == 0)
        return false;
    else
        return p->mIsPrepare;
}
