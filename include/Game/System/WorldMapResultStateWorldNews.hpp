#pragma once

#include "Game/System/WorldMapResultLocal.hpp"

class TalkMessageCtrl;

namespace NrvWorldMapResultStateWorldNews {
    WMR_NERVE_DECL(WorldMapResultStateWorldNewsWait);
    WMR_NERVE_DECL(WorldMapResultStateWorldNewsStart);
    WMR_NERVE_DECL(WorldMapResultStateWorldNewsTalk);
    WMR_NERVE_DECL(WorldMapResultStateWorldNewsEnd);
    WMR_NERVE_DECL(WorldMapResultStateWorldNewsSave);
};  // namespace NrvWorldMapResultStateWorldNews

/// World-news state ("State[ワールドニュース]"): talk controls for the stage news, one per entry.
WMR_OWNER_STATE_DECL(WorldMapResultStateWorldNews, virtual void control(); bool eventFunc(u32); bool checkStep(); void setIndexFlag(); bool isIndexInRange() const; void nextTalkNerve(); void exeStart(); void exeTalk(); void exeEnd(); void exeSave(); void startNextTalk();
               /* 0x10 */ s32 mIndex; /* 0x14 */ TalkMessageCtrl** mTalkCtrls; /* 0x18 */ u8 _18[0xC]; /* 0x24 */ void* mPanel;);
