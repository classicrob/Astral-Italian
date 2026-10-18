#include "Game/AudioLib/AudBgm.hpp"
#include "Game/AudioLib/AudBgmMgr.hpp"
#include "Game/AudioLib/AudBgmSetting.hpp"
#include "Game/AudioLib/AudBgmVolumeController.hpp"
#include "Game/AudioLib/AudFader.hpp"
#include "Game/AudioLib/AudSoundId.hpp"
#include "Game/AudioLib/AudSoundInfo.hpp"
#include "Game/AudioLib/AudSystem.hpp"
#include "Game/AudioLib/AudWrap.hpp"
#include "Game/RhythmLib/AudBgmTempoAdjuster.hpp"
#include "Game/RhythmLib/AudRhythmMeSystem.hpp"
#include "Game/RhythmLib/AudRhythmSeqParser.hpp"
#include <JSystem/JAudio2/JAISound.hpp>
#include <JSystem/JAudio2/JAISoundChild.hpp>
#include <JSystem/JAudio2/JAISoundInfo.hpp>
#include <JSystem/JAudio2/JASCriticalSection.hpp>

AudSingleBgm::AudSingleBgm() : AudBgm(), mHandle(), mSoundID(-1) {
    init();
}
