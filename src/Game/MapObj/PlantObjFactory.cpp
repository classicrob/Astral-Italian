#include "Game/MapObj/PlantObjFactory.hpp"
#include "Game/MapObj/MarinePlant.hpp"
#include "Game/MapObj/PlantGroup.hpp"
#include "Game/Util/SceneUtil.hpp"
#include <cstring>
namespace {
    struct PlantCreateEntry {
        const char* mPrefix;
        const char* mExactName;
        CreatorFuncPtr mCreator;
    };
    const PlantCreateEntry cPlantCreateTable[] = {
        {"MarinePlant", nullptr, PlantObjFactory::createNameObj<MarinePlant>},
        {"PlantA", nullptr, PlantObjFactory::createNameObj<PlantGroup>},
        {"PlantB", nullptr, PlantObjFactory::createNameObj<PlantGroup>},
        {"PlantC", nullptr, PlantObjFactory::createNameObj<PlantGroup>},
        {"PlantD", nullptr, PlantObjFactory::createNameObj<PlantGroup>},
    };
}
namespace {
    inline const PlantCreateEntry* findPlant(const char* name) {
        for (u32 i = 0; i < 5; ++i) {
            const PlantCreateEntry* entry = &cPlantCreateTable[i];
            bool equal;
            if (entry->mPrefix) equal = strncmp(name, entry->mPrefix, strlen(entry->mPrefix)) == 0;
            else equal = strcmp(name, entry->mExactName) == 0;
            if (equal) return entry;
        }
        return nullptr;
    }
}
CreatorFuncPtr PlantObjFactory::getCreator(const char* name) {
    const PlantCreateEntry* found = findPlant(name);
    if (found) return found->mCreator;
    if (MR::isInitializeStatePlacementSomething()) NameObjFactory::getCreator(name);
    return nullptr;
}
NameObj* PlantObjFactory::initChildObj(const JMapInfoIter& iter, int index) {
    const char* name = nullptr;
    MR::getChildObjName(&name, iter, index);
    CreatorFuncPtr creator = getCreator(name);
    NameObj* obj = creator(MR::getJapaneseObjectName(name));
    MR::initChildObj(obj, iter, index);
    return obj;
}
