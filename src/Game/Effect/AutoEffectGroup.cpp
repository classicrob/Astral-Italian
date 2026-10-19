#include "Game/Effect/AutoEffectGroup.hpp"
#include "Game/Effect/AutoEffectInfo.hpp"
#include "Game/Effect/EffectSystemUtil.hpp"
#include "Game/Util/JMapInfo.hpp"
#include "Game/System/ResourceCsvGroupHolder.hpp"
#include "Game/Util/SystemUtil.hpp"

AutoEffectGroup::AutoEffectGroup(const char* pName, int capacity) : mName(pName), mEffects(), mCapacity(), mCount() {
    mEffects = new AutoEffectInfo*[capacity];
    mCapacity = capacity;
}

void AutoEffectGroup::add(const JMapInfoIter& rIter) {
    AutoEffectInfo* pInfo = new AutoEffectInfo;
    pInfo->init(rIter);
    const s32 index = mCount++;
    mEffects[index] = pInfo;
}

namespace MR {
    namespace Effect {
        void addAutoEffectsFromGroup(const AutoEffectGroup* pGroup, EffectKeeper* pKeeper, const LiveActor* pActor) {
            for (s32 i = 0; i < pGroup->mCount; i++) {
                addAutoEffect(pKeeper, pActor, pGroup->mEffects[i]);
            }
        }

        void addAutoEffectsFromGroup(const AutoEffectGroup* pGroup, PaneEffectKeeper* pKeeper, const LayoutActor* pActor) {
            for (s32 i = 0; i < pGroup->mCount; i++) {
                addAutoEffect(pKeeper, pActor, pGroup->mEffects[i]);
            }
        }

        void addAutoEffectsFromGroup(const AutoEffectGroup* pGroup, MultiSceneEffectKeeper* pKeeper, const MultiSceneActor* pActor) {
            for (s32 i = 0; i < pGroup->mCount; i++) {
                addAutoEffect(pKeeper, pActor, pGroup->mEffects[i]);
            }
        }

        // NONMATCHING: equivalent count-selection branch is materialized with a mask; register allocation differs.
        AutoEffectGroup* createAutoEffectGroup(const char* pName) {
            const ResourceCsvGroupInfo* range = getGameSystemObjHolder3C()->fn_804CC650(pName);
            if (range == nullptr || static_cast<s32>(range->_C) <= 0) return nullptr;
            s32 count = range != nullptr ? range->_C : 0;
            AutoEffectGroup* group = new AutoEffectGroup(pName, count);
            for (s32 i = 0; i < count; i++) {
                JMapInfoIter iter(getAutoEffectListBinary(), range->_10 + i);
                group->add(iter);
            }
            return group;
        }
    };  // namespace Effect
};  // namespace MR
