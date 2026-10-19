#include "Game/LiveActor/ActionFlagCtrl.hpp"
#include "Game/LiveActor/ActionFootPrintCtrl.hpp"
#include "Game/LiveActor/ActorJointCtrl.hpp"
#include "Game/LiveActor/HitSensorKeeper.hpp"
#include "Game/LiveActor/HitSensorInfo.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/System/ResourceHolder.hpp"
#include "Game/Util.hpp"
#include <cstdio>

extern char lbl_80694810[];
extern const char* lbl_807CFA18;

ActionFlagSensorInfo::ActionFlagSensorInfo() : mName(nullptr), mFlag(0), mStartFrame(-1), mEndFrame(-1) {}

ActionFlagCtrl* ActionFlagCtrl::tryCreate(LiveActor* pHost) {
    if (MR::isExistFileInArc(MR::getResourceHolder(pHost), lbl_80694810, lbl_807CFA18)) {
        ActionFlagCtrl* pCtrl = new ActionFlagCtrl(pHost);
        if (pCtrl != nullptr) {
            return pCtrl;
        }
        return pCtrl;
    }
    return nullptr;
}

namespace {
    u16 parseActionFlag(const char* value) {
        if (*value == 'o') return 3;
        return *value == 'x' ? 2 : 0;
    }
    const char* sFlagNames[] = {"SwitchA", "SwitchB", "ShowModel", "CollisionParts", "Binder", "CalcGravity", "InvalidateClipping", "FaceCtrl"};
}

void ActionFlagCtrl::init() {
    mHitSensorKeeper = mHost->mSensorKeeper;
    JMapInfo* csv = MR::tryCreateCsvParser(MR::getResourceHolder(mHost), "%s.bcsv", lbl_807CFA18);
    if (!csv) return;
    mNumEntries = MR::getCsvDataElementNum(csv);
    mEntries = new ActionFlagInfo[mNumEntries];
    for (s32 i = 0; i < mNumEntries; i++) {
        ActionFlagInfo* info = &mEntries[i];
        MR::getCsvDataStr(&info->mName, csv, "ActionName", i);
        info->mFlags = new u16[8];
        for (s32 j = 0; j < 8; j++) {
            const char* name = sFlagNames[j];
            u16 flag;
            if (!MR::hasCsvItem(csv, name)) flag = 0;
            else {
                const char* value = nullptr;
                MR::getCsvDataStr(&value, csv, name, i);
                flag = parseActionFlag(value);
            }
            info->mFlags[j] = flag;
        }
        if (mHitSensorKeeper) {
            info->mSensors = new ActionFlagSensorInfo[mHitSensorKeeper->mSensorCount];
            for (s32 j = 0; j < mHitSensorKeeper->mSensorCount; j++) {
                ActionFlagSensorInfo* sensor = &info->mSensors[j];
                sensor->mName = mHitSensorKeeper->getNthSensorInfo(j)->mSensorName;
                char column[128];
                snprintf(column, sizeof(column), "Sensor%s", mHitSensorKeeper->getNthSensorInfo(j)->mSensorName);
                if (csv->searchItemInfo(column) >= 0) {
                    const char* value = nullptr;
                    MR::getCsvDataStr(&value, csv, column, i);
                    sensor->mFlag = parseActionFlag(value);
                    char minColumn[128];
                    snprintf(minColumn, sizeof(minColumn), "Sensor%sStep%s", mHitSensorKeeper->getNthSensorInfo(j)->mSensorName, "Min");
                    s16 min = -1;
                    MR::getCsvDataS16(&min, csv, minColumn, i);
                    sensor->mStartFrame = min;
                    char maxColumn[128];
                    snprintf(maxColumn, sizeof(maxColumn), "Sensor%sStep%s", mHitSensorKeeper->getNthSensorInfo(j)->mSensorName, "Max");
                    s16 max = -1;
                    MR::getCsvDataS16(&max, csv, maxColumn, i);
                    sensor->mEndFrame = max;
                }
            }
        }
    }
    startAction(MR::getResourceHolder(mHost)->mName);
}

void ActionFlagCtrl::setJointCtrl(ActorJointCtrl* ctrl) {
    mJointCtrl = ctrl;
    updateFaceCtrl();
}

void ActionFlagCtrl::startAction(const char* name) {
    mFlagData = findInfo(name);
    mIsActive = false;
    if (mFlagData) {
        updateFlags();
        initSensors();
    }
}

void ActionFlagCtrl::update() {
    if (!mFlagData) return;
    if (!mIsActive) return;
    updateSensors();
}

ActionFlagCtrl::ActionFlagCtrl(LiveActor* pHost) {
    mHost = pHost;
    mHitSensorKeeper = nullptr;
    mJointCtrl = nullptr;
    mNumEntries = 0;
    mEntries = nullptr;
    mFlagData = nullptr;
    mIsActive = false;
}


void ActionFlagCtrl::updateFlags() {
    if (MR::isValidSwitchA(mHost)) {
        if (isFlagOn(0, MR::isOnSwitchA(mHost))) MR::onSwitchA(mHost);
        else if (isFlagOff(0, MR::isOnSwitchA(mHost))) MR::offSwitchA(mHost);
    }
    if (MR::isValidSwitchB(mHost)) {
        if (isFlagOn(1, MR::isOnSwitchB(mHost))) MR::onSwitchB(mHost);
        else if (isFlagOff(1, MR::isOnSwitchB(mHost))) MR::offSwitchB(mHost);
    }
    if (isFlagOn(2, !MR::isHiddenModel(mHost))) MR::showModel(mHost);
    else if (isFlagOff(2, !MR::isHiddenModel(mHost))) MR::hideModel(mHost);
    if (MR::isExistCollisionParts(mHost)) {
        if (isFlagOn(3, MR::isValidCollisionParts(mHost))) MR::validateCollisionParts(mHost);
        else if (isFlagOff(3, MR::isValidCollisionParts(mHost))) MR::invalidateCollisionParts(mHost);
    }
    if (mHost->mBinder) {
        if (isFlagOn(4, !MR::isNoBind(mHost))) MR::onBind(mHost);
        else if (isFlagOff(4, !MR::isNoBind(mHost))) MR::offBind(mHost);
    }
    if (isFlagOn(5, MR::isCalcGravity(mHost))) MR::onCalcGravity(mHost);
    else if (isFlagOff(5, MR::isCalcGravity(mHost))) MR::offCalcGravity(mHost);
    if (isFlagOn(6, MR::isInvalidClipping(mHost))) MR::invalidateClipping(mHost);
    else if (isFlagOff(6, MR::isInvalidClipping(mHost))) MR::validateClipping(mHost);
    updateFaceCtrl();
}

void ActionFlagCtrl::updateFaceCtrl() {
    if (mJointCtrl && mJointCtrl->mFaceJointCtrl) {
        if (isFlagOn(7, false)) {
            mJointCtrl->startFaceCtrl(-1);
            return;
        }
        if (isFlagOff(7, true)) mJointCtrl->endFaceCtrl(-1);
    }
}

void ActionFlagCtrl::initSensors() {
    if (!mHitSensorKeeper) return;
    for (s32 i = 0; i < mHitSensorKeeper->mSensorCount; i++) {
        ActionFlagSensorInfo* info = &mFlagData->mSensors[i];
        if (info->mStartFrame > 0) mIsActive = true;
        else {
            if (info->mEndFrame > 0) mIsActive = true;
            if (info->mFlag == 3) MR::validateHitSensor(mHost, info->mName);
            else if (info->mFlag == 2) MR::invalidateHitSensor(mHost, info->mName);
        }
    }
}

void ActionFlagCtrl::updateSensors() {
    if (!mHitSensorKeeper) return;
    J3DFrameCtrl* ctrl = MR::getBckCtrl(mHost);
    for (s32 i = 0; i < mHitSensorKeeper->mSensorCount; i++) {
        ActionFlagSensorInfo* info = &mFlagData->mSensors[i];
        if (info->mStartFrame > 0 && ctrl->checkPass(info->mStartFrame)) MR::validateHitSensor(mHost, info->mName);
        if (info->mEndFrame > 0 && ctrl->checkPass(info->mEndFrame)) MR::invalidateHitSensor(mHost, info->mName);
    }
}

bool ActionFlagCtrl::isFlagOn(s32 index, bool current) const {
    if (!mFlagData) return false;
    if (mFlagData->mFlags[index] == 3 && !current) return true;
    return false;
}

bool ActionFlagCtrl::isFlagOff(s32 index, bool current) const {
    if (!mFlagData) return false;
    if (mFlagData->mFlags[index] == 2 && current) return true;
    return false;
}

// NONMATCHING: register/stack or instruction scheduling (FP order/grouping proven by default rdclass a).
ActionFlagInfo* ActionFlagCtrl::findInfo(const char* name) const {
    for (s32 i = 0; i < mNumEntries; i++) {
        ActionFlagInfo* info = &mEntries[i];
        if (MR::isEqualStringCase(info->mName, name)) return info;
    }
    return nullptr;
}

ActionFootPrintInfo::ActionFootPrintInfo() : mBckName(nullptr), mStartFrame(0.0f), mJointName(nullptr), mIsRight(0), mIsEnabled(0) {
}
