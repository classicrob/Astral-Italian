#include "Game/Util/JMapLinkInfo.hpp"
#include "Game/Util.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/StringUtil.hpp"

namespace {
    static const char* sFollowInfoTagName[3] = {"MapParts_ID", "Obj_ID", "ChildObjId"};
    static const char* sInfoNameTable[3] = {"mappartsinfo", "objinfo", "childobjinfo"};

    s32 getInfoType(const JMapInfo* pInfo) {
        const char* pName = pInfo->getName();

        for (s32 i = 0; i < 2; i++) {
            if (MR::isEqualStringCase(pName, sInfoNameTable[i])) {
                return i;
            }
        }

        return -1;
    }
};  // namespace

JMapLinkInfo::JMapLinkInfo(const JMapInfoIter& rIter, bool linked) {
    mFollowID = -1;
    mZoneID = -1;
    mContext = -1;

    if (linked) {
        setLinkedInfo(rIter);
    } else {
        setLinkInfo(rIter);
    }
}

bool JMapLinkInfo::isValid() const {
    bool ret = false;
    if (mFollowID >= 0 && mZoneID >= 0) {
        if (mContext >= 0) {
            ret = true;
        }
    }

    return ret;
}

void JMapLinkInfo::invalidate() {
    mFollowID = -1;
    mZoneID = -1;
    mContext = -1;
}

void JMapLinkInfo::setLinkedInfo(const JMapInfoIter& rIter) {
    invalidate();

    if (rIter.isValid()) {
        s32 id = -1;
        if (rIter.getValue< s32 >("l_id", &id)) {
            mFollowID = id;
            mZoneID = MR::getPlacedZoneId(rIter);

            mContext = ::getInfoType(rIter.mInfo);
        }
    }
}

void JMapLinkInfo::setLinkInfo(const JMapInfoIter& rIter) {
    invalidate();

    if (rIter.isValid()) {
        mZoneID = MR::getPlacedZoneId(rIter);

        s32 linkId = -1;
        s32 infoType = -1;
        bool found = false;

        for (s32 i = 0; i < 3; i++) {
            s32 id = -1;
            rIter.getValue< s32 >(::sFollowInfoTagName[i], &id);

            if (id >= 0 && !found) {
                linkId = id;
                infoType = i;
                found = true;
            }
        }

        if (found) {
            mFollowID = linkId;
            mContext = infoType;
            mZoneID = MR::getPlacedZoneId(rIter);
        }
    }
}
