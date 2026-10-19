#include "Game/GameAudio/ActionSoundSystem.hpp"
#include "Game/Enemy/ActionSoundData.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/System/ResourceHolder.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/StringUtil.hpp"

ActionSoundSystem::ActionSoundSystem(const char* name) : NameObj(name), mCount(0) {
    mData = new ActionSoundData*[128];
    for (s32 i = 0; i < 128; i++) {
        mData[i] = nullptr;
    }
}

ActionSoundData* ActionSoundSystem::fn_80220970(const char* name) {
    if (!name) return nullptr;
    for (s32 i = 0; i < mCount; i++) {
        if (MR::isEqualStringCase(mData[i]->mName, name)) return mData[i];
    }
    ActionSoundData* data = createActionSoundData(name);
    if (!data) return nullptr;
    mData[mCount] = data;
    mCount++;
    return data;
}

namespace MR {
    const char* getActionSoundName(const LiveActor* actor, const char* name) {
        if (!name) {
            ResourceHolder* resource = getModelResourceHolder(actor);
            if (resource) {
                name = resource->mName;
            } else {
                return nullptr;
            }
        }
        return name;
    }

    ActionSoundData* getActionSoundData(const LiveActor* actor, const char* name) {
        const char* soundName = getActionSoundName(actor, name);
        ActionSoundSystem* system = static_cast<ActionSoundSystem*>(getSceneObjHolder()->getObj(SceneObj_ActionSoundSystem));
        return system->fn_80220970(soundName);
    }

    ActionSoundData* getActionSoundData(const char* name) {
        if (!isExistSceneObj(SceneObj_ActionSoundSystem)) return nullptr;
        ActionSoundSystem* system = static_cast<ActionSoundSystem*>(getSceneObjHolder()->getObj(SceneObj_ActionSoundSystem));
        return system->fn_80220970(name);
    }
}

ActionSoundSystem::~ActionSoundSystem() {}
