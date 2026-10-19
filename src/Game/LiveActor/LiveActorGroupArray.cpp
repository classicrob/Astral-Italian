#define JMAPIDINFO_TRIVIAL_COPY
#include "Game/LiveActor/LiveActorGroupArray.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util.hpp"
#include "Game/Util/JMapIdInfo.hpp"
#include <algorithm>
#include <cstdio>

MsgSharedGroup::MsgSharedGroup(const char* pName, s32 maxActor, const JMapInfoIter& rIter) : LiveActorGroup(pName, maxActor) {
    mIdInfo = nullptr;
    mMsg = -1;
    mSensor = nullptr;
    mSensorName = nullptr;
    MR::copyString(mGroupName, pName, GROUP_NAME_LEN);
    setName(mGroupName);

    JMapIdInfo* pIdInfo = new JMapIdInfo();

    if (pIdInfo != nullptr) {
        JMapIdInfo newInfo = MR::createJMapIdInfoFromGroupId(rIter);
        pIdInfo->_0 = newInfo._0;
        pIdInfo->mZoneID = newInfo.mZoneID;
    }

    mIdInfo = pIdInfo;
}

void MsgSharedGroup::init(const JMapInfoIter& rIter) {
    MR::connectToScene(this, 6, -1, -1, -1);
}

void MsgSharedGroup::movement() {
    NameObj::movement();

    if (mMsg != 0xFFFFFFFF) {
        for (s32 i = 0; i < mObjNum; i++) {
            HitSensor* sensor = getActor(i)->getSensor(mSensorName);
            sensor->receiveMessage(mMsg, mSensor);
        }

        mMsg = -1;
    }
}

void MsgSharedGroup::sendMsgToGroupMember(u32 msg, HitSensor* pSensor, const char* pName) {
    mMsg = msg;
    mSensor = pSensor;
    mSensorName = pName;
}

LiveActorGroupArray::LiveActorGroupArray(const char* pName) : NameObj(pName) {
    mNumUsedGroups = 0;
}

void LiveActorGroupArray::init(const JMapInfoIter&) {
}

MsgSharedGroup* LiveActorGroupArray::tryCreateGroupForLiveActor(LiveActor* pActor, const JMapInfoIter& rIter, const char* pGroupName, s32 max) {
    s32 groupID = -1;
    MR::getJMapInfoGroupID(rIter, &groupID);

    if (pGroupName == nullptr) {
        char buf[32];
        snprintf(buf, sizeof(buf), "group%02d", groupID);
        pGroupName = buf;
    }

    MsgSharedGroup* group = findGroup(rIter);

    if (group == nullptr) {
        group = createGroup(rIter, pGroupName, max);
    }

    group->registerActor(pActor);
    return group;
}

MsgSharedGroup* LiveActorGroupArray::getLiveActorGroup(const LiveActor* pActor) const {
    return findGroup(pActor);
}

MsgSharedGroup* LiveActorGroupArray::createGroup(const JMapInfoIter& rIter, const char* pGroupName, s32 max) {
    MsgSharedGroup* group = new MsgSharedGroup(pGroupName, max, rIter);
    group->initWithoutIter();
    s32 curGroups = mNumUsedGroups;
    mNumUsedGroups = curGroups + 1;
    mGroups[curGroups] = group;
    return group;
}

namespace {
    struct IsGroupIdEqual {
        IsGroupIdEqual(const JMapIdInfo& rInfo) : mInfo(rInfo) {
        }

        bool operator()(const MsgSharedGroup* pGroup) const {
            return *pGroup->mIdInfo == mInfo;
        }

        JMapIdInfo mInfo;
    };

    struct HasActor {
        HasActor(const LiveActor* pActor) : mActor(pActor) {
        }

        bool operator()(const MsgSharedGroup* pGroup) const {
            return pGroup->hasActor(mActor);
        }

        const LiveActor* mActor;
    };
}  // namespace

// NONMATCHING: stack copies of the find_if functor (target copies it twice by value) and loop registers
MsgSharedGroup* LiveActorGroupArray::findGroup(const JMapInfoIter& rIter) const {
    MsgSharedGroup* const* it = std::find_if(mGroups, mGroups + mNumUsedGroups, IsGroupIdEqual(MR::createJMapIdInfoFromGroupId(rIter)));

    if (it != mGroups + mNumUsedGroups) {
        return *it;
    }

    return nullptr;
}

// NONMATCHING: stack copies of the find_if functor and loop registers
MsgSharedGroup* LiveActorGroupArray::findGroup(const LiveActor* pActor) const {
    MsgSharedGroup* const* it = std::find_if(mGroups, mGroups + mNumUsedGroups, HasActor(pActor));

    if (it != mGroups + mNumUsedGroups) {
        return *it;
    }

    return nullptr;
}

MsgSharedGroup::~MsgSharedGroup() {
}

LiveActorGroupArray::~LiveActorGroupArray() {
}
