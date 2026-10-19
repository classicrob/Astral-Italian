#include "Game/LiveActor/ActorItemGenerator.hpp"

#include "Game/LiveActor/ActorActionKeeper.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/GravityUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/StringUtil.hpp"

namespace {

// Table row (0x10 bytes): item kind (1 = coin, 2 = star piece) and the value multiplied into the count.
struct ActorItemTableEntry {
    /* 0x0 */ const char* mDisplayName;
    /* 0x4 */ const char* mCsvName;
    /* 0x8 */ s32 mKind;
    /* 0xC */ s32 mValue;
};

const ActorItemTableEntry sItemTable[6] = {
    {"なし", "", 0, 0},
    {"コインx１", "Coinx1", 1, 1},
    {"スターピースx１", "StarPiecex1", 2, 1},
    {"スターピースx３", "StarPiecex3", 2, 3},
    {"スターピースx６", "StarPiecex6", 2, 6},
    {"スターピースx１０", "StarPiecex10", 2, 10},
};

const char* sGeneratorPrefix = "ItemGenerator";
const char* sNameA = "ヒット";
const char* sNameB = "踏み";
const char* sNameC = "ヨッシー食べる";
const char* sNameD = "ボスダメージ";

// fn_8022B920: the ItemGenerator of an actor (ActorActionKeeper::mItemGenerator at +0x14).
ActorItemGenerator* getItemGenerator(LiveActor* pActor) {
    if (pActor->mActionKeeper == NULL) {
        return NULL;
    }
    ActorItemGenerator* pGen = pActor->mActionKeeper->mItemGenerator;
    if (pGen != NULL) {
        return pGen;
    }
    return NULL;
}

// fn_8022B990
void appearItemOfToVelocity(LiveActor*, const char*, const TVec3f*, const TVec3f*) NO_INLINE;
void appearItemOfToVelocity(LiveActor* pActor, const char* pName, const TVec3f* pPos, const TVec3f* pVel) {
    ActorItemGenerator* pGen = getItemGenerator(pActor);
    if (pGen != NULL) {
        ActorItemGeneratorEntry* pEntry = pGen->findEntry(pName);
        if (pEntry != NULL && pEntry->mIsEnabled) {
            pGen->appearItemToVelocity(pEntry, pPos, pVel);
        }
    }
}

}  // namespace

ActorItemGeneratorEntry::ActorItemGeneratorEntry() : mName(NULL), mJointName(NULL), mTableIndex(0), mCount(1), mIsEnabled(true) {}

ActorItemGenerator::ActorItemGenerator(LiveActor* pActor, const char* pName) : mActor(pActor), mEntries(NULL), mEntryNum(0) {
    if (!loadEntries(pName)) return;
    declareItems();
}

// Tries to create the generator for pActor/pName, returns NULL if the csv does not exist.
ActorItemGenerator* ActorItemGenerator::tryCreate(LiveActor* pActor, const char* pName) {
    ResourceHolder* pHolder = MR::getResourceHolder(pActor);
    if (pName != NULL) {
        if (MR::isExistFileInArc(pHolder, "%s%s.bcsv", sGeneratorPrefix, pName)) {
            return new ActorItemGenerator(pActor, pName);
        }
    } else if (MR::isExistFileInArc(pHolder, "%s.bcsv", sGeneratorPrefix)) {
        return new ActorItemGenerator(pActor, NULL);
    }
    return NULL;
}

// NONMATCHING: regalloc, string-pool order and equivalent loop branches
// fn_8022B200
bool ActorItemGenerator::loadEntries(const char* pName) {
    ResourceHolder* pHolder = MR::getResourceHolder(mActor);
    JMapInfo* pCsv = MR::tryCreateCsvParser(pHolder, "%s%s.bcsv", sGeneratorPrefix, pName != NULL ? pName : "");
    if (pCsv == NULL) {
        return false;
    }

    mEntryNum = MR::getCsvDataElementNum(pCsv);
    if (mEntryNum == 0) {
        return false;
    }

    mEntries = new ActorItemGeneratorEntry[mEntryNum];
    for (s32 i = 0; i < mEntryNum; i++) {
        ActorItemGeneratorEntry* pEntry = &mEntries[i];
        MR::getCsvDataStr(&pEntry->mName, pCsv, "ActionName", i);

        const char* pKind = NULL;
        MR::getCsvDataStrOrNULL(&pKind, pCsv, "ItemInfoName", i);

        s32 tableIndex = 0;
        if (pKind != NULL) {
            s32 idx = 0;
            while (true) {
                if (MR::isEqualString(sItemTable[idx].mCsvName, pKind)) {
                    break;
                }
                idx++;
                if (idx >= 6) {
                    idx = 0;
                    break;
                }
            }
            tableIndex = idx;
        }
        pEntry->mTableIndex = tableIndex;

        MR::getCsvDataStrOrNULL(&pEntry->mJointName, pCsv, "JointName", i);
        if (MR::hasCsvItem(pCsv, "Count")) {
            MR::getCsvDataS32(&pEntry->mCount, pCsv, "Count", i);
        }
    }
    return true;
}

// fn_8022B3D0: declares the total coins / star pieces of all enabled entries up front.
void ActorItemGenerator::declareItems() {
    s32 starNum = 0;
    s32 coinNum = 0;
    for (s32 i = 0; i < mEntryNum; i++) {
        ActorItemGeneratorEntry* pEntry = &mEntries[i];
        if (pEntry->mIsEnabled) {
            const ActorItemTableEntry* pTable = &sItemTable[pEntry->mTableIndex];
            s32 num = pTable->mValue * pEntry->mCount;
            switch (pTable->mKind) {
            case 1:
                if (num > coinNum) {
                    coinNum = num;
                }
                break;
            case 2:
                if (num > starNum) {
                    starNum = num;
                }
                break;
            }
        }
    }
    if (starNum > 0) {
        MR::declareStarPiece(mActor, starNum);
    }
    if (coinNum > 0) {
        MR::declareCoin(mActor, coinNum);
    }
}

// fn_8022B4B0
ActorItemGeneratorEntry* ActorItemGenerator::findEntry(const char* pName) {
    for (s32 i = 0; i < mEntryNum; i++) {
        if (MR::isEqualString(mEntries[i].mName, pName)) {
            return &mEntries[i];
        }
    }
    return NULL;
}

// fn_8022B530: emits at the joint named by the entry (or at the actor position).
void ActorItemGenerator::emitItem(const char* pName) {
    ActorItemGeneratorEntry* pEntry = findEntry(pName);
    if (pEntry != NULL && pEntry->mIsEnabled) {
        TVec3f pos;
        if (MR::isNullOrEmptyString(pEntry->mJointName)) {
            pos.set(mActor->mPosition);
        } else {
            MR::copyJointPos(mActor, pEntry->mJointName, &pos);
        }
        appearItem(pEntry, &pos);
    }
}

// fn_8022B5C0
void ActorItemGenerator::emitItemAtPos(const char* pName, const TVec3f* pPos) {
    ActorItemGeneratorEntry* pEntry = findEntry(pName);
    if (pEntry != NULL && pEntry->mIsEnabled) {
        appearItem(pEntry, pPos);
    }
}

// fn_8022B620: pDir defaults to the negated gravity of the actor.
void ActorItemGenerator::emitItemAtPosAndDir(const char* pName, const TVec3f* pPos, const TVec3f* pDir) {
    ActorItemGeneratorEntry* pEntry = findEntry(pName);
    if (pEntry != NULL && pEntry->mIsEnabled) {
        TVec3f pos;
        if (pPos != NULL) {
            pos.set(*pPos);
        } else if (MR::isNullOrEmptyString(pEntry->mJointName)) {
            pos.set(mActor->mPosition);
        } else {
            MR::copyJointPos(mActor, pEntry->mJointName, &pos);
        }

        TVec3f dir;
        dir.negate(mActor->mGravity);
        if (pDir != NULL) {
            dir.set(*pDir);
        }
        appearItemToDirection(pEntry, &pos, &dir);
    }
}

// fn_8022B8A0
void ActorItemGenerator::enableAllEntries() {
    for (s32 i = 0; i < mEntryNum; i++) {
        mEntries[i].mIsEnabled = true;
    }
}

// fn_8022B8E0
void ActorItemGenerator::disableAllEntries() {
    for (s32 i = 0; i < mEntryNum; i++) {
        mEntries[i].mIsEnabled = false;
    }
}

// fn_8022B710
void ActorItemGenerator::appearItem(ActorItemGeneratorEntry* pEntry, const TVec3f* pPos) {
    const ActorItemTableEntry* pTable = &sItemTable[pEntry->mTableIndex];
    if (pTable == NULL) {
        return;
    }
    switch (pTable->mKind) {
    case 1:
        MR::appearCoinPop(mActor, *pPos, pTable->mValue);
        break;
    case 2:
        MR::appearStarPiece(mActor, *pPos, pTable->mValue, 10.0f, 40.0f, false);
        break;
    }
}

// fn_8022B770
void ActorItemGenerator::appearItemToDirection(ActorItemGeneratorEntry* pEntry, const TVec3f* pPos, const TVec3f* pDir) {
    const ActorItemTableEntry* pTable = &sItemTable[pEntry->mTableIndex];
    if (pTable == NULL) {
        return;
    }
    switch (pTable->mKind) {
    case 1:
        MR::appearCoinAgainstGravity(mActor, *pPos, pTable->mValue);
        break;
    case 2:
        MR::appearStarPieceToDirection(mActor, *pPos, *pDir, pTable->mValue, 40.0f, 50.0f, false);
        break;
    }
}

// fn_8022B7E0
void ActorItemGenerator::appearItemToVelocity(ActorItemGeneratorEntry* pEntry, const TVec3f* pPos, const TVec3f* pVel) {
    const ActorItemTableEntry* pTable = &sItemTable[pEntry->mTableIndex];
    if (pTable == NULL) {
        return;
    }
    switch (pTable->mKind) {
    case 1: {
        TVec3f vel;
        vel = *pVel;
        vel.scale(0.6f);
        MR::appearCoinToVelocity(mActor, *pPos, vel, pTable->mValue);
        break;
    }
    case 2:
        MR::appearStarPieceToVelocity(mActor, *pPos, *pVel, pTable->mValue, 0, 0, 0, 0.17f);
        break;
    }
}

namespace MR {
    void emitItem(LiveActor* actor, const char* name) {
        ActorItemGenerator* generator = getItemGenerator(actor);
        if (generator) generator->emitItem(name);
    }

    void emitItemHit(LiveActor* pActor) {
        MR::emitItem(pActor, sNameA);
    }

    void emitItemHitAtPos(LiveActor* pActor, const TVec3f* pPos) {
        const char* name = sNameA;
        ActorItemGenerator* pGen = getItemGenerator(pActor);
        if (pGen != NULL) {
            pGen->emitItemAtPos(name, pPos);
        }
    }

    void emitItemStomp(LiveActor* pActor) {
        const char* name = sNameB;
        ActorItemGenerator* pGen = getItemGenerator(pActor);
        if (pGen != NULL) {
            pGen->emitItem(name);
        }
    }

    void emitItemYoshiEat(LiveActor* pActor) {
        const char* name = sNameC;
        ActorItemGenerator* pGen = getItemGenerator(pActor);
        if (pGen != NULL) {
            pGen->emitItem(name);
        }
    }

    void appearItemYoshiEatToPlayer(LiveActor* pActor) {
        TVec3f vel;
        MR::calcYoshiEatItemVelocity(&vel);
        const TVec3f* pPos = MR::getPlayerCenterPos();
        appearItemOfToVelocity(pActor, sNameC, pPos, &vel);
    }

    void emitItemBossDamage(LiveActor* pActor) {
        const char* name = sNameD;
        ActorItemGenerator* pGen = getItemGenerator(pActor);
        if (pGen != NULL) {
            pGen->emitItemAtPosAndDir(name, NULL, NULL);
        }
    }

    void emitItemBossDamageAtPos(LiveActor* pActor, const TVec3f* pPos) {
        TVec3f dir(0.0f, -1.0f, 0.0f);
        MR::calcGravityVector(pActor, *pPos, &dir, NULL, 0);
        const char* name = sNameD;
        ActorItemGenerator* pGen = getItemGenerator(pActor);
        if (pGen != NULL) {
            pGen->emitItemAtPosAndDir(name, pPos, &dir);
        }
    }

    void enableAllItemGenerator(LiveActor* pActor) {
        ActorItemGenerator* pGen = getItemGenerator(pActor);
        if (pGen != NULL) {
            pGen->enableAllEntries();
        }
    }

    void disableAllItemGenerator(LiveActor* pActor) {
        ActorItemGenerator* pGen = getItemGenerator(pActor);
        if (pGen != NULL) {
            pGen->disableAllEntries();
        }
    }
};  // namespace MR
