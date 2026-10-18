#include "Game/Player/GhostPlayerSound.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StringUtil.hpp"

#pragma auto_inline off

namespace {
    struct AnimSoundInfo {
        const char* mAnimation;
        const char* mBodySound;
        const char* mVoiceSound;
    };

    const AnimSoundInfo sAnimSoundTable[] = {
        {"ジャンプ", "SE_BM_GHOST_MARIO_JUMP_S", "SE_BV_GHOST_MARIO_JUMP_S"},
        {"ジャンプB", "SE_BM_GHOST_MARIO_JUMP_M", "SE_BV_GHOST_MARIO_JUMP_M"},
        {"ジャンプC", "SE_BM_GHOST_MARIO_JUMP_L", "SE_BV_GHOST_MARIO_JUMP_L"},
        {"壁ジャンプ", "SE_BM_GHOST_MARIO_JUMP_S", "SE_BV_GHOST_MARIO_JUMP_S"},
        {"埋まり脱出ジャンプ", "SE_BM_GHOST_MARIO_JUMP_S", "SE_BV_GHOST_MARIO_JUMP_S"},
        {"幅とび", "SE_BM_GHOST_MARIO_JUMP_L", "SE_BV_GHOST_MARIO_JUMP_L"},
        {"しゃがみジャンプ", "SE_BM_GHOST_MARIO_JUMP_L", "SE_BV_GHOST_MARIO_JUMP_L"},
        {"崖つかまり開始", nullptr, "SE_BV_GHOST_MARIO_HANG"},
        {"崖つかまり終了", nullptr, "SE_BV_GHOST_MARIO_CLIMB"},
        {"壁押し", nullptr, "SE_BV_GHOST_MARIO_HANG"},
        {"ヒップドロップ開始", "SE_BM_GHOST_MARIO_HIP_DROP_TURN", "SE_BV_GHOST_MARIO_HIPDROP_S"},
        {"ヒップドロップ着地", "SE_BM_GHOST_MARIO_HIP_DROP_LAND", "SE_BV_GHOST_MARIO_HIPDROP_E"},
        {"埋まり脱出ジャンプ", "SE_BM_GHOST_MARIO_JUMP_S", "SE_BV_GHOST_MARIO_JUMP_S"},
        {"水泳スピン", "SE_BM_GHOST_MARIO_SPIN", "SE_BV_GHOST_MARIO_SPIN"},
        {"水泳ジェット開始", nullptr, "SE_BV_GHOST_MARIO_SWM_ACCEL"},
        {"カメ持ちリング準備", nullptr, "SE_BV_GHOST_MARIO_TAKE"},
        {"カメ持ちリング", nullptr, "SE_BV_GHOST_MARIO_SWM_ACCEL"},
        {"投げ", nullptr, "SE_BV_GHOST_MARIO_THROW"},
        {"空中ひねり", "SE_BM_GHOST_MARIO_SPIN", "SE_BV_GHOST_MARIO_SPIN"},
        {"地上ひねり", "SE_BM_GHOST_MARIO_SPIN", "SE_BV_GHOST_MARIO_SPIN"},
        {"空パンチ", "SE_BM_GHOST_MARIO_SPIN", "SE_BV_GHOST_MARIO_SPIN"},
        {"アイスひねり", "SE_BM_GHOST_MARIO_SPIN", "SE_BV_GHOST_MARIO_SPIN"},
        {"アイスひねり移動", "SE_BM_GHOST_MARIO_SPIN", "SE_BV_GHOST_MARIO_SPIN"},
        {"スケートアクセルジャンプ", "SE_BM_GHOST_MARIO_JUMP_L", "SE_BV_GHOST_MARIO_JUMP_L"},
        {"スケートジャンプ2", "SE_BM_GHOST_MARIO_JUMP_L", "SE_BV_GHOST_MARIO_JUMP_L"},
        {"氷上力行右", nullptr, "SE_BV_GHOST_MARIO_JUMP_S"},
        {"氷上力行左", nullptr, "SE_BV_GHOST_MARIO_JUMP_S"},
        {"ショートジャンプ", "SE_BM_GHOST_MARIO_JUMP_S", "SE_BV_GHOST_MARIO_JUMP_S"},
        {"雲スピン空中", "SE_BM_GHOST_MARIO_SPIN", "SE_BV_GHOST_MARIO_SPIN"},
        {"雲スピン", "SE_BM_GHOST_MARIO_SPIN", "SE_BV_GHOST_MARIO_SPIN"},
    };

    const AnimSoundInfo* getAnimSoundInfo(const char* animation) {
        for (u32 index = 0; index < 30; ++index) {
            if (MR::isEqualString(animation, sAnimSoundTable[index].mAnimation)) return &sAnimSoundTable[index];
        }
        return nullptr;
    }
}

namespace MR {
    void fn_803811B0(const LiveActor* actor, const char* animation) {
        const AnimSoundInfo* info = getAnimSoundInfo(animation);
        if (info == nullptr) return;
        if (info->mBodySound != nullptr) startSound(actor, info->mBodySound, -1, -1);
    }

    void fn_80381210(const LiveActor* actor, const char* animation) {
        const AnimSoundInfo* info = getAnimSoundInfo(animation);
        if (info == nullptr) return;
        if (info->mVoiceSound != nullptr) startSound(actor, info->mVoiceSound, -1, -1);
    }
}
