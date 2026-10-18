#include "Game/LiveActor/LiveActorGroupArray.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util.hpp"
#include "Game/Util/JMapIdInfo.hpp"
#include <cstdio>

// nonmatching
// NONMATCHING: JMapIdInfo has a user-declared copy ctor, so MR::createJMapIdInfoFromGroupId returns via memory (target returns in r3:r4); needs header fix
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
        for (s32 i = 0; i < mObjectCount; i++) {
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

// NONMATCHING: JMapIdInfo user copy ctor forces sret return (target uses r3:r4); loop shape also differs (find_if-style bool flag)
MsgSharedGroup* LiveActorGroupArray::findGroup(const JMapInfoIter& rIter) const {
    JMapIdInfo idInfo = MR::createJMapIdInfoFromGroupId(rIter);
    MsgSharedGroup* const* it = mGroups;
    MsgSharedGroup* const* end = mGroups + mNumUsedGroups;

    for (; it != end; ++it) {
        if (*(*it)->mIdInfo == idInfo) {
            break;
        }
    }

    if (it != end) {
        return *it;
    }

    return nullptr;
}

// NONMATCHING: outer loop shape differs (target nests a flagged inner getActor loop, ours breaks directly)
MsgSharedGroup* LiveActorGroupArray::findGroup(const LiveActor* pActor) const {
    MsgSharedGroup* const* it = mGroups;
    MsgSharedGroup* const* end = mGroups + mNumUsedGroups;

    for (; it != end; ++it) {
        if ((*it)->hasActor(pActor)) {
            break;
        }
    }

    if (it != end) {
        return *it;
    }

    return nullptr;
}

MsgSharedGroup::~MsgSharedGroup() {
}

LiveActorGroupArray::~LiveActorGroupArray() {
}
