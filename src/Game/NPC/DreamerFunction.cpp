#include "Game/NPC/DreamerFunction.hpp"
#include "Game/NPC/Dreamer.hpp"
#include "Game/NPC/NrvDreamer.hpp"
#include "Game/NameObj/NameObjFactory.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Util.hpp"

Dreamer::~Dreamer() {}

namespace NrvDreamer {
    DreamerNrvAppear DreamerNrvAppear::sInstance;
    DreamerNrvStartHelp DreamerNrvStartHelp::sInstance;
    DreamerNrvInHelp DreamerNrvInHelp::sInstance;
    DreamerNrvEndHelp DreamerNrvEndHelp::sInstance;
    DreamerNrvAppear::DreamerNrvAppear() {}
    DreamerNrvStartHelp::DreamerNrvStartHelp() {}
    DreamerNrvInHelp::DreamerNrvInHelp() {}
    DreamerNrvEndHelp::DreamerNrvEndHelp() {}
    void DreamerNrvEndHelp::execute(Spine* spine) const { static_cast<Dreamer*>(spine->mExecutor)->exeEndHelp(); }
    void DreamerNrvInHelp::execute(Spine* spine) const { static_cast<Dreamer*>(spine->mExecutor)->exeInHelp(); }
    void DreamerNrvStartHelp::execute(Spine* spine) const { static_cast<Dreamer*>(spine->mExecutor)->exeStartHelp(); }
    void DreamerNrvAppear::execute(Spine* spine) const { static_cast<Dreamer*>(spine->mExecutor)->exeAppear(); }
}

bool DreamerFunction::isPadModeDreamerAndStartId(s32 startId) {
    bool result = false;
    if (GameSequenceFunction::isPadModeDreamer() && startId == MR::getCurrentMarioStartId()) result = true;
    return result;
}
bool DreamerFunction::isPadModeSuperDreamerAndStartId(s32 startId) {
    bool result = false;
    if (GameSequenceFunction::isPadModeSuperDreamer() && startId == MR::getCurrentMarioStartId()) result = true;
    return result;
}
void DreamerFunction::initChildObj(const JMapInfoIter& iter) {
    if (!GameSequenceFunction::isPadMode2()) {
        for (s32 i = 0; i < MR::getChildObjNum(iter); i++) NameObjFactory::initChildObj(iter, i);
    }
}
