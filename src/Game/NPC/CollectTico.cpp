// NONMATCHING: target data-symbol boundaries differ for matching literal offsets.
#include "Game/NPC/CollectTico.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/NPC/StrayTico.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include <JSystem/JMath/JMath.hpp>

namespace NrvCollectTico {
    NEW_NERVE(CollectTicoNrvWait, CollectTico, Wait);
    NEW_NERVE(CollectTicoNrvTryStartDemo, CollectTico, TryStartDemo);
    NEW_NERVE(CollectTicoNrvCompleteDemo, CollectTico, CompleteDemo);
    NEW_NERVE(CollectTicoNrvFlash, CollectTico, Flash);
    NEW_NERVE(CollectTicoNrvAppearPowerStar, CollectTico, AppearPowerStar);
};  // namespace NrvCollectTico

CollectTico::CollectTico(const char* pName) : LiveActor(pName) {
    mCameraInfo=nullptr;
    mStrayTicos = nullptr;
    mTicoNum = 0;
    mHasAppearDemo = 0;
    mHasCamera=false;
    mUsePlacedPosition=false;
    MR::registerCollectTico(this);
}

void CollectTico::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this,rIter);
    MR::connectToSceneNpcMovement(this);
    mTicoNum = MR::getChildObjNum(rIter);
    if(mTicoNum>0) mStrayTicos = new StrayTico*[mTicoNum];
    for (s32 i = 0; i < mTicoNum; i++) {
        mStrayTicos[i] = new StrayTico("はぐれチコ", this);
        MR::initChildObj(mStrayTicos[i], rIter, i);
    }

    initEffectKeeper(0, "CollectTico", false);
    initSound(2, "CollectTico", nullptr, TVec3f(0.0f));
    initNerve(GET_NERVE(CollectTico, CollectTicoNrvWait));
    if (MR::tryRegisterDemoCast(this, rIter)) {
        MR::registerDemoActionFunctor(this, MR::Functor_Inline(this, &CollectTico::startAppearPowerStar), "集めチコスター出現");
        mHasAppearDemo = 1;
    }

    mHasCamera=MR::initMultiActorCamera(this,rIter,&mCameraInfo,"集めデモカメラ");
    MR::getJMapInfoArg0WithInit(rIter,&mUsePlacedPosition);
    MR::initUseStageSwitchWriteA(this, rIter);
    MR::declarePowerStar(this);
    MR::invalidateClipping(this);
    makeActorAppeared();
}

void CollectTico::exeWait() {
    bool needsDemo = true;
    for (s32 i = 0; i < mTicoNum; i++) {
        if (!mStrayTicos[i]->isRescued()) {
            needsDemo = false;
            break;
        }
    }

    if (needsDemo && !MR::isPlayerDead()) {
        MR::requestStartDemo(this, "チコ集めコンプリート", GET_NERVE(CollectTico, CollectTicoNrvCompleteDemo),
                             GET_NERVE(CollectTico, CollectTicoNrvTryStartDemo));
    }
}

void CollectTico::exeCompleteDemo() {
    if (MR::isFirstStep(this)) {
        if(!mUsePlacedPosition) {
            mDemoCenter.set(*MR::getPlayerPos());
            MR::setPosition(this,mDemoCenter);
            MR::calcGravity(this);
            mDemoCenter.scaleAdd(300.0f,-mGravity,mDemoCenter);
            MR::setPosition(this,mDemoCenter);
        } else {
            MR::rotateAroundActor(mGravity,TVec3f(0.0f,-1.0f,0.0f),this);
            mDemoCenter.set(mPosition);
        }

        for (s32 i = 0; i < mTicoNum; i++) {
            MR::requestMovementOn(mStrayTicos[i]);
            mStrayTicos[i]->requestCompleteDemo(mDemoCenter, mGravity, 360.0f * ((f32)i / mTicoNum));
        }
        if(mHasCamera) MR::startMultiActorCameraTargetSelf(this,mCameraInfo,"集めデモカメラ",-1);
    }

    MR::startActionSound(this, "SmLvStrayticoComplete");
    bool needFlash = true;
    for (s32 i = 0; i < mTicoNum; i++) {
        if (!mStrayTicos[i]->isCompleteDemoEnd()) {
            needFlash = false;
            break;
        }
    }

    if (needFlash) {
        setNerve(GET_NERVE(CollectTico, CollectTicoNrvFlash));
    }
}

void CollectTico::exeFlash() {
    if (MR::isFirstStep(this)) {
        MR::emitEffect(this, "CollectTicoLight");
        MR::startActionSound(this, "SmStrayticoFusion");
    }

    if (MR::isStep(this, 10)) {
        for (s32 i = 0; i < mTicoNum; i++) {
            mStrayTicos[i]->kill();
        }
    }

    if (MR::isStep(this, 60)) {
        if (!mHasAppearDemo) {
            startAppearPowerStar();
            if(!mHasCamera) MR::endDemo(this,"チコ集めコンプリート");
        }

        if (MR::isValidSwitchA(this)) {
            MR::onSwitchA(this);
        }
    }
}

void CollectTico::exeAppearPowerStar() {
    if (MR::isFirstStep(this)) {
        for (s32 i = 0; i < mTicoNum; i++) {
            mStrayTicos[i]->noticeEnd();
        }

    }
    if(mHasCamera) {
        if(MR::isEndPowerStarAppearDemo(this)) {
            MR::endMultiActorCamera(this,mCameraInfo,"集めデモカメラ",false,-1);
            MR::endDemo(this,"チコ集めコンプリート");
            kill();
        }
    } else {
        kill();
    }
}

s32 CollectTico::calcNoRescuedCount() const {
    s32 count = 0;
    for (s32 i = 0; i < mTicoNum; i++) {
        if (!mStrayTicos[i]->isRescued()) {
            count++;
        }
    }

    return count;
}

void CollectTico::startAppearPowerStar() {
    if(mHasCamera) MR::appearPowerStarContinueCurrentDemo(this,mDemoCenter);
    else MR::requestAppearPowerStar(this, mDemoCenter);
    setNerve(GET_NERVE(CollectTico, CollectTicoNrvAppearPowerStar));
}

void CollectTico::exeTryStartDemo() {
}

void CollectTico::launch(const TPos3f& mtx) {
    for(s32 i=0;i<mTicoNum;i++) mStrayTicos[i]->launch(mtx);
}
