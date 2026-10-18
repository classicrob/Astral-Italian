#pragma once

#include <revolution.h>

class JMapInfoIter;

class JMapLinkInfo {
public:
    JMapLinkInfo();
    JMapLinkInfo(const JMapInfoIter&, bool);

    bool operator==(const JMapLinkInfo& rOther) const {
        if (!isValid() || !rOther.isValid()) {
            return false;
        }

        return mFollowID == rOther.mFollowID && mZoneID == rOther.mZoneID && mContext == rOther.mContext;
    }

    void setLinkedInfo(const JMapInfoIter&);
    bool isValid() const;
    void invalidate();
    void setLinkInfo(const JMapInfoIter&);

    s16 mFollowID;  // 0x00
    s16 mZoneID;    // 0x02
    s16 mContext;   // 0x04
};
