// Model and effect texture prewarm (called from Host::prewarmHudTextures, hud_prewarm.cpp). A model's GPU textures are created
// when it is first drawn, an effect's when it first emits; in play that is a 20-40 ms frame each time a planet comes into view,
// Mario's own model appears after the intro, or the first walk/jump/spin dust is emitted. On the Wii none of this costs anything
// at first use (the texels are in memory already). This hands every static texture of the stage's loaded model resources and
// of the particle resource to Aurora's texture cache (petari_gx_texture_prewarm), which creates them under a per-frame time budget
// over the following frames (patch_aurora_texture_prewarm.py). The texture objects are built with JUTTexture::storeTIMG, the
// game's own path, so a later draw finds the same content-cache entry. Game thread; SDK side.

#include "Game/System/ResourceHolderManager.hpp"

#include "Game/Effect/ParticleResourceHolder.hpp"
#include "Game/System/GameSystem.hpp"
#include "Game/System/GameSystemObjHolder.hpp"
#include "Game/System/ResourceHolder.hpp"
#include "Game/Util/SingletonHolder.hpp"

#include <JSystem/J3DGraphAnimator/J3DModelData.hpp>
#include <JSystem/J3DGraphBase/J3DTexture.hpp>
#include <JSystem/JParticle/JPAResourceManager.hpp>
#include <JSystem/JParticle/JPATexture.hpp>
#include <JSystem/JUtility/JUTTexture.hpp>

#include <cstdint>
#include <unordered_set>

#include "host.hpp"

extern "C" void petari_gx_texture_prewarm(const void* texObj);

namespace PetariNative::App::Host {
namespace {

// A static, non-palette texture image: the same GXTexObj the game's own draw builds. Returns whether it was queued.
bool prewarmImage(const ResTIMG* timg, std::unordered_set<const void*>& seen) {
    if (timg == nullptr || timg->mWidth == 0 || timg->mHeight == 0 || timg->mPaletteNum != 0) return false;
    if (timg->mFormat == GX_TF_C4 || timg->mFormat == GX_TF_C8 || timg->mFormat == GX_TF_C14X2) return false;
    if (!seen.insert(timg).second) return false;
    JUTTexture texture;
    texture.storeTIMG(timg, static_cast< u8 >(0));
    petari_gx_texture_prewarm(&texture.mObj);
    return true;
}

}  // namespace

unsigned prewarmModelTextures() {
    unsigned queued = 0;
    ResourceHolderManager* manager = SingletonHolder< ResourceHolderManager >::get();
    if (manager == nullptr) return 0;
    std::unordered_set<const void*> seen;
    for (const ResourceHolderManagerName2Resource* entry = manager->nativeBegin(); entry != manager->nativeEnd(); ++entry) {
        const ResourceHolder* holder = entry->mResourceHolder;
        const ResTable* models = holder != nullptr ? holder->mModelResTable : nullptr;
        if (models == nullptr || models->mFileInfoTable == nullptr) continue;
        for (u32 i = 0; i < models->mCount; ++i) {
            const J3DModelData* data = static_cast< const J3DModelData* >(models->mFileInfoTable[i].mResource);
            const J3DTexture* textures = data != nullptr ? data->getTexture() : nullptr;
            if (textures == nullptr) continue;
            for (u16 t = 0; t < textures->getNum(); ++t) queued += prewarmImage(textures->getResTIMG(t), seen) ? 1u : 0u;
        }
    }
    return queued;
}

unsigned prewarmEffectTextures() {
    unsigned queued = 0;
    const GameSystem* system = SingletonHolder< GameSystem >::get();
    const GameSystemObjHolder* objects = system != nullptr ? system->mObjHolder : nullptr;
    const ParticleResourceHolder* particles = objects != nullptr ? objects->mParticleResHolder : nullptr;
    const JPAResourceManager* manager = particles != nullptr ? particles->mResourceMgr : nullptr;
    if (manager == nullptr || manager->mpTexArr == nullptr) return 0;
    std::unordered_set<const void*> seen;
    for (u16 i = 0; i < manager->mTexNum; ++i) {
        const JPATexture* texture = manager->mpTexArr[i];
        if (texture != nullptr) queued += prewarmImage(texture->mTexture.getTexInfo(), seen) ? 1u : 0u;
    }
    return queued;
}

}  // namespace PetariNative::App::Host
