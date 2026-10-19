#include "Game/AudioLib/AudBgmSetting.hpp"
#include "Game/AudioLib/AudBgmParamTable.hpp"
#include "Game/AudioLib/AudStageBgmTable.hpp"
#include "Game/AudioLib/AudSoundNameConverter.hpp"
#include "Game/AudioLib/AudWrap.hpp"
#include "Game/Util.hpp"
#include "Game/Util/JMapInfo.hpp"
#include "JSystem/JAudio2/JAUSoundTable.hpp"
#include "JSystem/JKernel/JKRArchive.hpp"

namespace AudBgmSetting {
    const char* sBgmParamResourceName = "/AudioRes/Info/BgmParam.arc";
    const char* sBgmParamFileName = "BgmParam.bcsv";
    const char* getBgmParamResourceName() {
        return sBgmParamResourceName;
    }
    void loadBgmParamResource() {
        JMapInfo info;
        JKRArchive* arc = MR::receiveArchive(sBgmParamResourceName);
        if (!arc)
            arc = MR::mountArchive(sBgmParamResourceName, 0, true);
        info.attach(arc->getResource(sBgmParamFileName));
        for (JMapInfoIter i = info.begin(); i != info.end(); ++i.mIndex) {
            const char* sound = i.getValueString("SoundIdName");
            s32 volume = i.getValueS32("Volume");
            JAUSoundTableItem* data = getSoundData(AudSoundNameConverter::get()->getSoundID(sound));
            if (data)
                data->mVolume = volume;
        }
    }

    JAUSoundTableItem* getSoundData(JAISoundID id) {
        JAUSoundTableItem* result = 0;
        JAUSoundTableItem* data = JAUSoundTable::getInstance()->getData(id);
        u8 type = JAUSoundTable::getInstance()->getTypeID(id);
        if (data && (type & 0x40))
            result = data;
        return result;
    }
}  // namespace AudBgmSetting

// The receiver is a sound ID copied by AudEffector::modifySoundPitch, not a sound-table item.
bool JAISoundID::fn_80089670() const {
    u8 group = getGroupID();
    if (group == 8)
        return true;
    if (group == 9)
        return true;
    return group == 5;
}
