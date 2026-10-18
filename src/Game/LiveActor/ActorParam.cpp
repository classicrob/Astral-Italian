#include "Game/LiveActor/ActorParam.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/ModelManager.hpp"
#include "Game/System/ResourceHolder.hpp"
#include "Game/Util.hpp"
#include "Game/Util/JMapInfo.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/StringUtil.hpp"

namespace {
    const char* sFileName = "ActorParam";
}  // namespace

ActorParam* ActorParam::tryCreate(LiveActor* pActor) {
    ResourceHolder* pHolder = MR::getResourceHolder(pActor);
    if (pHolder == NULL) {
        return NULL;
    }

    if (MR::isExistFileInArc(pHolder, "%s.bcsv", sFileName)) {
        return new ActorParam(pActor);
    }

    return NULL;
}

// Looks up a row by name. Returns NULL if no row has that name.
static ActorParamEntry* findEntry(const ActorParam* pParam, const char* pName) {
    for (s32 i = 0; i < pParam->mEntryNum; i++) {
        ActorParamEntry* pEntry = &pParam->mEntries[i];
        if (MR::isEqualString(pEntry->mName, pName)) {
            return pEntry;
        }
    }

    return NULL;
}

namespace {
    const f32 sDefaultF32 = 0.0f;
    const s32 sDefaultS32 = 0;
    const f32 sDefaultBlock8[2] = {0.0f, 0.0f};
    const f32 sDefaultBlock16[4] = {0.0f, 0.0f, 0.0f, 0.0f};
}  // namespace

// Accessors: return the value slot of the named row, or a zeroed default if the row is missing.
void* getActorParamValuePtr(const ActorParam* pParam, const char* pName) {
    ActorParamEntry* pEntry = findEntry(pParam, pName);
    if (pEntry != NULL) {
        return &pEntry->mValue;
    }
    return (void*)&sDefaultBlock8;
}

void* getActorParamS32Ptr(const ActorParam* pParam, const char* pName) {
    ActorParamEntry* pEntry = findEntry(pParam, pName);
    if (pEntry != NULL) {
        return &pEntry->mValue;
    }
    return (void*)&sDefaultS32;
}

void* getActorParamBlockPtr(const ActorParam* pParam, const char* pName) {
    ActorParamEntry* pEntry = findEntry(pParam, pName);
    if (pEntry != NULL) {
        return pEntry->mValue.mPtr;
    }
    return (void*)&sDefaultBlock16;
}

void* getActorParamF32Ptr(const ActorParam* pParam, const char* pName) {
    ActorParamEntry* pEntry = findEntry(pParam, pName);
    if (pEntry != NULL) {
        return &pEntry->mValue;
    }
    return (void*)&sDefaultF32;
}

// Used to reset a 0x1C-byte default parameter block.
void initActorParamDefault(f32* pParam) {
    pParam[0] = 0.0f;
    pParam[1] = 0.0f;
    pParam[2] = 0.0f;
    pParam[3] = 1.0f;
    pParam[4] = 0.0f;
    pParam[5] = 0.0f;
    pParam[6] = 1.0f;
}

ActorParam::ActorParam(LiveActor* pActor) : mEntryNum(0), mEntries(NULL) {
    JMapInfo* pCsv = MR::tryCreateCsvParser(MR::getResourceHolder(pActor), "%s.bcsv", sFileName);

    s32 num = 0;
    if (pCsv != NULL) {
        num = MR::getCsvDataElementNum(pCsv);
    }

    mEntryNum = num;
    mEntries = new ActorParamEntry[mEntryNum];

    f32 tmp = 0.0f;
    for (s32 i = 0; i < mEntryNum; i++) {
        ActorParamEntry* pEntry = &mEntries[i];
        MR::getCsvDataStr(&pEntry->mName, pCsv, "ParamName", i);

        const char* pType = NULL;
        MR::getCsvDataStr(&pType, pCsv, "ParamType", i);

        if (MR::isEqualString(pType, "s32")) {
            pEntry->mType = 0;
            tmp = 0.0f;
            MR::getCsvDataF32(&tmp, pCsv, "Param00", i);
            pEntry->mValue.mS32 = (s32)tmp;
        } else if (MR::isEqualString(pType, "f32")) {
            pEntry->mType = 1;
            MR::getCsvDataF32(&pEntry->mValue.mF32, pCsv, "Param00", i);
        } else if (MR::isEqualString(pType, "ParamMove")) {
            pEntry->mType = 2;
            f32* pBlock = new f32[4];
            MR::getCsvDataF32(&pBlock[0], pCsv, "Param00", i);
            MR::getCsvDataF32(&pBlock[1], pCsv, "Param01", i);
            MR::getCsvDataF32(&pBlock[2], pCsv, "Param02", i);
            MR::getCsvDataF32(&pBlock[3], pCsv, "Param03", i);
            pEntry->mValue.mPtr = pBlock;
        } else if (MR::isEqualString(pType, "ParamSight")) {
            pEntry->mType = 4;
            f32* pBlock = new f32[3];
            MR::getCsvDataF32(&pBlock[0], pCsv, "Param00", i);
            MR::getCsvDataF32(&pBlock[1], pCsv, "Param01", i);
            MR::getCsvDataF32(&pBlock[2], pCsv, "Param02", i);
            pEntry->mValue.mPtr = pBlock;
        } else if (MR::isEqualString(pType, "ParamRebound")) {
            pEntry->mType = 5;
            f32* pBlock = new f32[3];
            MR::getCsvDataF32(&pBlock[0], pCsv, "Param00", i);
            MR::getCsvDataF32(&pBlock[1], pCsv, "Param01", i);
            MR::getCsvDataF32(&pBlock[2], pCsv, "Param02", i);
            pEntry->mValue.mPtr = pBlock;
        } else if (MR::isEqualString(pType, "ParamJump")) {
            pEntry->mType = 3;
            f32* pBlock = new f32[2];
            MR::getCsvDataF32(&pBlock[0], pCsv, "Param00", i);
            MR::getCsvDataF32(&pBlock[1], pCsv, "Param01", i);
            pEntry->mValue.mPtr = pBlock;
        }
    }
}
