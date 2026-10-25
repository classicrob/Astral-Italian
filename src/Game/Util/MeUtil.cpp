#include "Game/AudioLib/AudMeNameConverter.hpp"
#include "Game/AudioLib/AudSystem.hpp"
#include "Game/AudioLib/AudWrap.hpp"
#include "Game/RhythmLib/AudChordInfo.hpp"
#include "Game/RhythmLib/AudMeObject.hpp"
#include "Game/Util/SoundUtil.hpp"

namespace MR {
    u32 getMEID(const char* pName) {
        return AudSingletonHolder< AudMeNameConverter >::get()->getMeID(pName);
    }

    bool hasME() {
        AudChordInfo* pChordInfo = AudWrap::getSystem()->getChordInfo();

        return pChordInfo->mTable.mLoaded && pChordInfo->mCurChord != nullptr && pChordInfo->mCurScale != nullptr;
    }

    void startSystemME(const char* pName) {
        if (!hasME()) {
            return;
        }

        u32 id = AudSingletonHolder< AudMeNameConverter >::get()->getMeID(pName);

        AudWrap::getSystemMeObject()->startMe(id);
    }

    void fn_800422F0(u32 id) {
        if (!hasME()) {
            return;
        }

        AudWrap::getSystemMeObject()->startMe(id);
    }

}
