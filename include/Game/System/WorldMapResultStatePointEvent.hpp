#pragma once

#include "Game/System/WorldMapResultLocal.hpp"

class TalkMessageCtrl;

/// Point event state ("State[ポイントイベント]"): star checkpoint event on the result map.
WMR_OWNER_STATE_DECL(WorldMapResultStatePointEvent, virtual void control(); void setIndex(s32, s32); void moveCamera(f32); void exeStart(); void exeMove();
               void exeTalkStart(); void exeTalk(); void exeTalkEnd(); void exeFade(); void exeEaseIn(); void exeEvent(); void exeExtend();
               void exeWaitExtend(); void exeMessage(); void exeEnd();
               /* 0x10 */ WorldMapResultRoute* mRoute; /* 0x14 */ s32 mCount; /* 0x18 */ s32 mIndex; /* 0x1C */ s32 mSubIndex;
               /* 0x20 */ TVec3f mPos; /* 0x2C */ TVec3f mOffset; /* 0x38 */ TalkMessageCtrl** mTalkCtrls;);
