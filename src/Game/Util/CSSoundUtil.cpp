#include "Game/Util/SoundUtil.hpp"
#include "Game/AudioLib/AudSpeakerWrap.hpp"
#include "Game/AudioLib/AudSystem.hpp"
#include "Game/AudioLib/CSSoundNameConverter.hpp"
#include "Game/Util/GamePadUtil.hpp"

namespace MR {
    u32 getCSSoundID(const char* pName) {
        return AudSingletonHolder< CSSoundNameConverter >::get()->getSoundID(pName);
    }

    void startCSSound(s32 csID, const char* pSEName, s32 param3, s32 param4, f32 volume) {
        if (AudSystem::msBasic->isHomeButtonMenuActive()) {
            return;
        }

        if (AudSpeakerWrap::isPlayable(-1)) {
            AudSpeakerWrap::start(csID, param3, param4, volume);
        } else if (pSEName != nullptr) {
            startSystemSE(pSEName, -1, -1);
        }
    }

    void startCSSound(const char* pCSName, const char* pSEName, s32 param3, s32 param4, f32 volume) {
        if (AudSystem::msBasic->isHomeButtonMenuActive()) {
            return;
        }

        if (AudSpeakerWrap::isPlayable(-1)) {
            s32 id = AudSingletonHolder< CSSoundNameConverter >::get()->getSoundID(pCSName);
            if (id != -1) {
                AudSpeakerWrap::start(id, param3, param4, volume);
            }
        } else if (pSEName != nullptr) {
            startSystemSE(pSEName, -1, -1);
        }
    }

    void startCSSound1P(const char* pCSName, s32 param2, f32 volume) {
        startCSSound(pCSName, nullptr, -1, param2, volume);
    }

    void startCSSound2P(const char* pCSName, const char* pSEName, s32 param3, f32 volume) {
        if (isConnectedWPad(1)) {
            startCSSound(pCSName, pSEName, 1, param3, volume);
        }
    }

    void stopCSSound(s32 param1, s32 param2) {
        AudSpeakerWrap::stop(param2, param1);
    }

    void stopAndStartCSSound1P(const char* pCSName, s32 param2, const char* pSEName, s32 param4, f32 volume) {
        AudSpeakerWrap::stop(0, param2);
        startCSSound(pCSName, pSEName, 0, param4, volume);
    }

    void stopAndStartCSSound2P(const char* pCSName, s32 param2, const char* pSEName, s32 param4, f32 volume) {
        AudSpeakerWrap::stop(1, param2);
        if (isConnectedWPad(1)) {
            startCSSound(pCSName, pSEName, 1, param4, volume);
        }
    }

    void startCSSoundAfterPlayed(s32 param1, s32 param2) {
        if (param1 == -1) {
            return;
        }
        AudSpeakerWrap::startLevel(param1, param2);
    }
}  // namespace MR
