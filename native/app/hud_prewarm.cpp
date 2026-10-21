// HUD texture prewarm (Host::prewarmHudTextures, host.hpp). The game builds every in-game HUD layout in
// GameSceneLayoutHolder::init, during scene initialization, and shows them later. On the Wii their textures
// are already in memory and the first draw only points the texture unit at them; in the port the first draw of
// each layout created its GPU textures (7-13 uploads, 20-37 ms, about 85 frames after play began). The seam calls
// this once per stage, on the first gameplay frame: it hands each static (non-palette) layout texture to Aurora's
// texture cache (petari_gx_texture_prewarm, patch_aurora_texture_prewarm.py), which creates the GPU texture
// at that frame's aurora_end_frame. Nothing about the game's layouts, timing or GX state changes.
// Game thread, with the CPU held (the seam calls it before releasing it). SDK side: no Aurora or SDL headers.

#include "Game/Screen/CameraInfo.hpp"
#include "Game/Screen/CoinCounter.hpp"
#include "Game/Screen/CometMedalCounter.hpp"
#include "Game/Screen/CounterLayoutController.hpp"
#include "Game/Screen/DestroyCounter.hpp"
#include "Game/Screen/GameSceneLayoutHolder.hpp"
#include "Game/Screen/InformationMessage.hpp"
#include "Game/Screen/LayoutActor.hpp"
#include "Game/Screen/LayoutManager.hpp"
#include "Game/Screen/MarioMeter.hpp"
#include "Game/Screen/MarioSubMeter.hpp"
#include "Game/Screen/MeterLayout.hpp"
#include "Game/Screen/MissLayout.hpp"
#include "Game/Screen/NoteCounter.hpp"
#include "Game/Screen/OneUpBoard.hpp"
#include "Game/Screen/PlayerLeft.hpp"
#include "Game/Screen/PurpleCoinCounter.hpp"
#include "Game/Screen/StarCounter.hpp"
#include "Game/Screen/StarPieceCounter.hpp"
#include "Game/Screen/SubMeterLayout.hpp"
#include "Game/Screen/SuddenDeathMeter.hpp"
#include "Game/Screen/YesNoLayout.hpp"
#include "Game/Util/SceneUtil.hpp"

#include <nw4r/lyt/material.h>
#include <nw4r/lyt/pane.h>
#include <nw4r/lyt/texMap.h>
#include <petari/host_allocation.hpp>

#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include "host.hpp"

extern "C" void petari_gx_texture_prewarm(const void* texObj);

namespace PetariNative::App::Host {
namespace {

struct Counts {
    unsigned textures = 0, skipped = 0;
};

bool isPaletteFormat(GXTexFmt format) {
    return format == GX_TF_C4 || format == GX_TF_C8 || format == GX_TF_C14X2;
}

void prewarmMaterial(const nw4r::lyt::Material* material, Counts& counts) {
    if (material == nullptr) return;
    const u8 count = material->GetTextureNum();
    for (u8 i = 0; i < count; ++i) {
        const nw4r::lyt::TexMap& map = material->GetTexture(i);
        if (map.mImage == nullptr || isPaletteFormat(map.GetTexelFormat()) || map.mPalette != nullptr) {
            ++counts.skipped;
            continue;
        }
        GXTexObj obj;
        map.Get(&obj);
        petari_gx_texture_prewarm(&obj);
        ++counts.textures;
    }
}

void prewarmLayout(const LayoutActor* actor, Counts& counts) {
    const LayoutManager* manager = actor != nullptr ? actor->mLayoutManager : nullptr;
    if (manager == nullptr || manager->mPaneInfoList == nullptr) return;
    for (u32 i = 0; i < manager->mPaneCount; ++i) {
        const nw4r::lyt::Pane* pane = manager->mPaneInfoList[i].mPane;
        if (pane == nullptr) continue;
        const u8 materials = pane->GetMaterialNum();
        for (u8 m = 0; m < materials; ++m) prewarmMaterial(pane->GetMaterial(m), counts);
    }
}

}  // namespace

void prewarmHudTextures() {
    // PETARI_HUD_PREWARM=0 turns it off (before/after comparisons).
    static const bool enabled = [] {
        const char* value = std::getenv("PETARI_HUD_PREWARM");
        return value == nullptr || value[0] != '0';
    }();
    if (!enabled) return;
    GameSceneLayoutHolder* holder = MR::getGameSceneLayoutHolder();
    if (holder == nullptr) return;
    PetariNative::HostAllocationScope host;
    Counts counts;
    prewarmLayout(holder->mInformationMessage, counts);
    prewarmLayout(holder->mYesNoLayout, counts);
    prewarmLayout(holder->mCameraInfo, counts);
    prewarmLayout(holder->mMissLayout, counts);
    prewarmLayout(holder->mOneUpBoard, counts);
    prewarmLayout(holder->mNoteCounter, counts);
    prewarmLayout(holder->mPurpleCoinCounter, counts);
    prewarmLayout(holder->mDestroyCounter, counts);
    if (const MarioMeter* meter = holder->mMarioMeter) {
        prewarmLayout(meter->mHitPointMeter, counts);
        prewarmLayout(meter->mSuddenDeathMeter, counts);
    }
    if (const MarioSubMeter* sub = holder->mMarioSubMeter) {
        prewarmLayout(sub->mFlyMeter, counts);
        prewarmLayout(sub->mAirMeter, counts);
        prewarmLayout(sub->mActiveMeter, counts);
    }
    if (const CounterLayoutController* ctrl = holder->mCounterLayoutCtrl) {
        prewarmLayout(ctrl, counts);
        prewarmLayout(ctrl->mCoinCounter, counts);
        prewarmLayout(ctrl->mStarPieceCounter, counts);
        prewarmLayout(ctrl->mPlayerLeft, counts);
        prewarmLayout(ctrl->mStarCounter, counts);
        prewarmLayout(ctrl->mCometMedalCounter, counts);
    }
    std::fprintf(stderr, "[hud prewarm] %u layout textures handed to the texture cache (%u palette/empty skipped)\n",
                 counts.textures, counts.skipped);
}

}  // namespace PetariNative::App::Host
