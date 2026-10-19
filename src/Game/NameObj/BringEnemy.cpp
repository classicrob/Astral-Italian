#include "Game/NameObj/BringEnemy.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/NameObj/NameObjFactory.hpp"
#include "Game/Enemy/BegomanBase.hpp"
#include "Game/Enemy/BegomanLauncher.hpp"
#include "Game/Map/StageSwitch.hpp"


namespace NrvBringEnemy {
    // Declaration order matches the sInstance init order in __sinit: Wait first, then Bring.
    NEW_NERVE(BringEnemyNrvWait, BringEnemy, Wait);
    NEW_NERVE(BringEnemyNrvBring, BringEnemy, Bring);
}  // namespace NrvBringEnemy

BringEnemy::BringEnemy(const char* pName) : LiveActor(pName), mTargetEnemy(NULL), mChildren(NULL), mChildNum(0), mBringLimit(0), mBroughtNum(0), mEnemyType(0) {
}

void BringEnemy::init(JMapInfoIter& rIter) {
    MR::connectToSceneNpcMovement(this);
    MR::initDefaultPos(this, rIter);

    mChildNum = MR::getChildObjNum(rIter);
    mChildren = new LiveActor*[mChildNum];
    mChildGroups = new LiveActor*[mChildNum];

    for (s32 i = 0; i < mChildNum; i++) {
        mChildGroups[i] = NULL;

        const char* pName = NULL;
        MR::getChildObjName(&pName, rIter, i);

        if (MR::isEqualString(pName, "Kuribo")) {
            mChildren[i] = (LiveActor*)NameObjFactory::initChildObj(rIter, i);
            mEnemyType = 1;
        } else if (MR::isEqualString(pName, "BegomanLauncher") || MR::isEqualString(pName, "BegomanBabyLauncher") ||
                   MR::isEqualString(pName, "TogeBegomanLauncher")) {
            mChildren[i] = (LiveActor*)NameObjFactory::initChildObj(rIter, i);
            mChildGroups[i] = mChildren[i];
            mEnemyType = 2;
        } else {
            mChildren[i] = (LiveActor*)NameObjFactory::initChildObj(rIter, i);
        }
    }

    s32 limit = 1;
    MR::getJMapInfoArg0NoInit(rIter, &limit);
    if (limit >= 0) {
        mBringLimit = limit;
    }

    initHitSensor(1);
    TVec3f offset(0.0f, 0.0f, 0.0f);
    s32 radius = 0xAA;
    MR::addHitSensorEnemy(this, "Search", 0x10, (f32)radius, offset);

    initNerve(&NrvBringEnemy::BringEnemyNrvWait::sInstance, 0);

    MR::initUseStageSwitchWriteA(this, rIter);
    MR::useStageSwitchSyncAppear(this, rIter);
    MR::invalidateClipping(this);
}

void BringEnemy::exeWait() {
    s32 aliveNum = 0;
    for (s32 i = 0; i < mChildNum; i++) {
        if (!MR::isDead(mChildren[i])) {
            aliveNum++;
        }
    }

    if (mBroughtNum >= mBringLimit && !MR::isPlayerDead()) {
        return;
    }
    if (mBroughtNum + aliveNum < mBringLimit) {
        return;
    }
}

void BringEnemy::exeBring() {
    if (MR::isFirstStep(this)) {
        if (MR::isValidSwitchA(this)) {
            MR::onSwitchA(this);
        }

        kill();
    }
}

f32 BringEnemy::calcNearestEnemy(TVec3f* pVec, const LiveActor* pActor) {
    f32 minDist = 0.0f;
    LiveActor* pNearest = NULL;

    for (s32 i = 0; i < mChildNum; i++) {
        LiveActor* pGroup = mChildGroups[i];
        if (pGroup != NULL) {
            s32 groupNum = *(s32*)((u8*)pGroup + 0x90);
            for (s32 j = 0; j < groupNum; j++) {
                LiveActor* pChild = static_cast< BegomanLauncher* >(pGroup)->getLaunchedBegoman(j);
                if (pChild != NULL && !MR::isDead(pChild)) {
                    f32 dist = MR::calcDistance(pActor, pChild);
                    if (dist < minDist) {
                        minDist = dist;
                        pNearest = pChild;
                    }
                }
            }
        } else {
            LiveActor* pChild = mChildren[i];
            if (pChild != NULL && !MR::isDead(pChild)) {
                f32 dist = MR::calcDistance(pActor, pChild);
                if (dist < minDist) {
                    minDist = dist;
                    pNearest = pChild;
                }
            }
        }
    }

    if (pNearest == NULL) {
        return 0.0f;
    }

    MR::calcVecToTargetPosH(pVec, pActor, pNearest->mPosition, NULL);
    return minDist;
}

LiveActor* BringEnemy::getNearestEnemy() const {
    return mTargetEnemy;
}

s32 BringEnemy::getEnemyType() const {
    return mEnemyType;
}

bool BringEnemy::receiveMessage(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    for (s32 i = 0; i < mChildNum; i++) {
        LiveActor* pGroup = mChildGroups[i];
        if (pGroup != NULL) {
            if (static_cast< BegomanLauncher* >(pGroup)->isLaunchedBegoman(MR::getSensorHost(pReceiver))) {
                mTargetEnemy = MR::getSensorHost(pReceiver);
                mBroughtNum++;
            }
        } else {
            if (MR::getSensorHost(pReceiver) == mChildren[i]) {
                mTargetEnemy = mChildren[i];
                mBroughtNum++;
            }
        }
    }

    // SMG2 falls off the end here (no return statement in the target)
}

BringEnemy::~BringEnemy() {
}

