#pragma once

#include "Game/System/WorldMapResultLocal.hpp"

class MiniGreenComet;
class MiniGreenCometShower;
class TalkMessageCtrl;

namespace NrvWorldMapResultStateGreenStarDemo {
    WMR_NERVE_DECL(WorldMapResultStateGreenStarDemoStart);
    WMR_NERVE_DECL(WorldMapResultStateGreenStarDemoShower);
    WMR_NERVE_DECL(WorldMapResultStateGreenStarDemoWait);
    WMR_NERVE_DECL(WorldMapResultStateGreenStarDemoTalkStart);
    WMR_NERVE_DECL(WorldMapResultStateGreenStarDemoTalk);
    WMR_NERVE_DECL(WorldMapResultStateGreenStarDemoEnd);
};  // namespace NrvWorldMapResultStateGreenStarDemo

/// Green-star demo state ("State[グリーンスター出現デモ]"): two comet actors, a counter array and a talk ctrl.
WMR_OWNER_STATE_DECL(WorldMapResultStateGreenStarDemo, virtual void control(); void showComets(); void exeStart(); void exeShower(); void exeTalkStart(); void exeTalk(); void exeEnd();
               /* 0x10 */ MiniGreenComet* mComet; /* 0x14 */ MiniGreenCometShower* mShower; /* 0x18 */ GalaxyWorldMapInfo** mCounts; /* 0x1C */ TalkMessageCtrl* mTalkCtrl;);
