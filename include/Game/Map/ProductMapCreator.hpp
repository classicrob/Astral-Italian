#pragma once

#include "Game/Map/PlanetMapCreator.hpp"

class JMapInfo;
class SimpleMapObj;
class AnmModelSwitchMove;
class AnmModelSwitchMoveEndKill;

// One row of ProductMapObjDataTable.bcsv (ModelName -> ClassName).
struct ProductMapEntry {
    /* 0x0 */ const char* mModelName;
    /* 0x4 */ const char* mClassName;
};

// SceneObj 0x7F (SMG2-only). Maps placed model names to the generic map-object class that builds them.
class ProductMapCreator : public NameObj {
public:
    ProductMapCreator(const char*);

    void populateEntries(const JMapInfo*);
    void parseEntry(const JMapInfo*, s32);
    ProductMapEntry* getProductMapEntry(const char*);
    CreatorFuncPtr getProductMapCreator(const char*);

    template < typename T >
    static NameObj* createNameObj(const char* pName) {
        return new T(pName);
    }

    static const PlanetEntry sCreateFuncTable[47];

    /* 0x14 */ ProductMapEntry** mEntries;
    /* 0x18 */ s32 mEntryCount;
};

namespace MR {
    CreatorFuncPtr getProductMapCreator(const char*);
    bool hasProductMapCreator(const char*);
};  // namespace MR

template <> NameObj* ProductMapCreator::createNameObj<SimpleMapObj>(const char*);
template <> NameObj* ProductMapCreator::createNameObj<AnmModelSwitchMove>(const char*);
template <> NameObj* ProductMapCreator::createNameObj<AnmModelSwitchMoveEndKill>(const char*);
