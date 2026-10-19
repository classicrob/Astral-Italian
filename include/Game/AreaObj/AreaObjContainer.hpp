#pragma once

#include "Game/NameObj/NameObj.hpp"
#include "Game/Util/Array.hpp"
#include <JSystem/JGeometry/TVec.hpp>

class AreaObj;
class AreaObjMgr;

typedef AreaObjMgr* (*ManagerCreatorFuncPtr)(s32, const char*);

struct Name2CreateManagerFunc {
    /* 0x0 */ const char* mName;
    /* 0x4 */ s32 mNumMax;
    /* 0x8 */ ManagerCreatorFuncPtr mCreateFunc;
};

class AreaObjContainer : public NameObj {
public:
    /// @brief Creates a new `AreaObjContainer`.
    /// @param pName A pointer to the null-terminated name of the object.
    AreaObjContainer(const char* pName);

    /// @brief Initializes the `AreaObjContainer` while being placed into a scene.
    /// @param rIter The reference to an iterator over a `JMapInfo`.
    virtual void init(const JMapInfoIter& rIter);

    AreaObjMgr* getManager(const char*) const;
    AreaObjMgr* getManager2(const char*) const;
    AreaObj* getAreaObj(const char*, const TVec3f&) const;
    bool isExistAreaObj(const char*) const;

private:
    template < typename T >
    static AreaObjMgr* createManager(s32 numMax, const char* pName);

    static const Name2CreateManagerFunc cCreateTable[0x5F];

    /* 0x14 */ MR::AssignableArray< AreaObjMgr* > mManagerArray;
    /* 0x1C */ s32 mNumManagers;
};

namespace MR {
    AreaObjContainer* getAreaObjContainer();

    template < typename T >
    T* getAreaObj(const char* pName, const TVec3f& rVec) {
        return static_cast< T* >(getAreaObjContainer()->getAreaObj(pName, rVec));
    }
};  // namespace MR
