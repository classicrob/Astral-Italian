#include "Game/Util/BgmUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Demo/MeetKoopaDemoObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

#include "Game/Demo/KoopaAssaultFallObj.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/Util/TalkUtil.hpp"
#include "Game/Util/MathUtil.hpp"

// MeetKoopaDemoObj and KoopaAssaultFallObj are one translation unit in SMG2: the two destructors and the shared
// static initializer (nerves of both classes, in this order) follow both classes' code.
namespace NrvMeetKoopaDemoObj {
    NEW_NERVE(MeetKoopaDemoObjNrvWait, MeetKoopaDemoObj, Wait);
    NEW_NERVE(MeetKoopaDemoObjNrvFound, MeetKoopaDemoObj, Found);
    NEW_NERVE(MeetKoopaDemoObjNrvFallObjects, MeetKoopaDemoObj, FallObjects);
    NEW_NERVE(MeetKoopaDemoObjNrvWaitBack, MeetKoopaDemoObj, WaitBack);
    NEW_NERVE(MeetKoopaDemoObjNrvDemo, MeetKoopaDemoObj, Demo);
};  // namespace NrvMeetKoopaDemoObj

// WaitBack has no symbol of its own in the target: its body is inlined into the nerve's execute.
inline void MeetKoopaDemoObj::exeWaitBack() {
    if (MR::isStep(this, 120)) {
        setNerve(&NrvMeetKoopaDemoObj::MeetKoopaDemoObjNrvFound::sInstance);
    }
}

namespace NrvKoopaAssaultFallObj {
    NEW_NERVE(KoopaAssaultFallObjNrvFall, KoopaAssaultFallObj, Fall);
    NEW_NERVE(KoopaAssaultFallObjNrvBreak, KoopaAssaultFallObj, Break);
};  // namespace NrvKoopaAssaultFallObj

namespace {
    // rodata table indexed by (nerve step / 30) (see exeWait)
    const s32 sShakeTable[16] = {-1, -1, 0, -1, -1, -1, 0, 0, -1, -1, -1, -1, 1, -1, -1, -1};

    const char* sPeachKidnapName = "ピーチ誘拐";
    const char* sDemoMeetKoopaName = "DemoMeetKoopa";
    const char* sPeachKidnapPosName = "ピーチ誘拐デモ基準点";
};  // namespace

MeetKoopaDemoObj::MeetKoopaDemoObj(const char* pName) : LiveActor(pName), mKoopa(nullptr), mKoopaArm(nullptr), mPeach(nullptr), mTicoBaby(nullptr), mFallObjs(nullptr), mTalkCtrl(nullptr) {
    mMtx.identity();
}

void MeetKoopaDemoObj::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::connectToSceneMapObjMovement(this);
    MR::invalidateClipping(this);
    if (MR::tryRegisterDemoCast(this, rIter)) {
        MR::registerDemoActionNerve(this, &NrvMeetKoopaDemoObj::MeetKoopaDemoObjNrvDemo::sInstance, nullptr);
    }

    mKoopa = reinterpret_cast<LiveActor*>(MR::createDemoObj("デモ用クッパ", "DemoKoopa", static_cast<MtxPtr>(nullptr)));
    mKoopa->mPosition.set< f32 >(mPosition);
    MR::tryInitDemoSheetAnim(mKoopa, rIter, sPeachKidnapName, sDemoMeetKoopaName);
    MR::invalidateClipping(mKoopa);
    mKoopa->makeActorDead();

    mKoopaArm = reinterpret_cast<LiveActor*>(MR::createDemoObj("デモ用クッパの手", "DemoKoopaArm", static_cast<MtxPtr>(nullptr)));
    MR::initTalkAnimDemoCast(mKoopaArm, rIter, sPeachKidnapName, sDemoMeetKoopaName);
    MR::invalidateClipping(mKoopaArm);
    mKoopaArm->makeActorDead();

    mPeach = reinterpret_cast<LiveActor*>(MR::createDemoObjWithAnimArc("デモ用ピーチ", "Peach", "PeachAnim", static_cast<MtxPtr>(nullptr)));
    MR::initTalkAnimDemoCast(mPeach, rIter, sPeachKidnapName, sDemoMeetKoopaName);
    MR::invalidateClipping(mPeach);
    mPeach->makeActorDead();

    mTicoBaby = reinterpret_cast<LiveActor*>(MR::createDemoObjWithAnimArc("デモ用ベビチコ", "TicoBaby", "TicoBabyAnim", static_cast<MtxPtr>(nullptr)));
    MR::initTalkAnimDemoCast(mTicoBaby, rIter, sPeachKidnapName, sDemoMeetKoopaName);
    MR::invalidateClipping(mTicoBaby);
    mTicoBaby->makeActorDead();

    mTalkCtrl = MR::createTalkCtrlDirect(mPeach, rIter, "StoryBook_MeetKoopa_PeachHelp", TVec3f(0.0f, 200.0f, 0.0f), mMtx);
    initNerve(&NrvMeetKoopaDemoObj::MeetKoopaDemoObjNrvWait::sInstance, 0);

    mFallObjs = new LiveActor*[5];
    for (s32 i = 0; i < 5; i++) {
        mFallObjs[i] = new KoopaAssaultFallObj("クッパ襲撃落下オブジェ");
        MR::initChildObj(mFallObjs[i], rIter, i);
    }
    MR::syncStageSwitchAppearAndSync(this, rIter);
    MR::needStageSwitchReadA(this, rIter);
    MR::needStageSwitchWriteB(this, rIter);
    makeActorDead();
}

void MeetKoopaDemoObj::kill() {
    mKoopa->kill();
    mKoopaArm->kill();
    mPeach->kill();
    mTicoBaby->kill();
    LiveActor::kill();
}

void MeetKoopaDemoObj::exeWait() {
    if (MR::isIntervalStep(this, 30)) {
        s32 shake = sShakeTable[(getNerveStep() / 30) & 0xF];
        if (shake == 0) {
            MR::tryRumblePadMiddle(this, 0);
            MR::shakeCameraNormalWeak();
        } else if (shake == 1) {
            MR::tryRumblePadStrong(this, 0);
            MR::shakeCameraNormal();
        }
    }
    if (MR::isOnSwitchA(this)) {
        setNerve(&NrvMeetKoopaDemoObj::MeetKoopaDemoObjNrvFound::sInstance);
    }
}

void MeetKoopaDemoObj::exeFound() {
    if (MR::isFirstStep(this)) {
        if (MR::isDead(mKoopa)) {
            mKoopa->makeActorAppeared();
        }
        MR::tryStartActionNotPlaying(mKoopa, "CastleAssault");
    }
    if (MR::isIntervalStep(this, 30)) {
        for (s32 i = 0; i < 5; i++) {
            if (MR::isDead(mFallObjs[i]) && MR::getRandom(static_cast<s32>(0), static_cast<s32>(20)) == 0) {
                mFallObjs[i]->appear();
                break;
            }
        }
    }
    if (MR::checkPassBckFrame(mKoopa, 400.0f) || MR::checkPassBckFrame(mKoopa, 1400.0f)) {
        setNerve(&NrvMeetKoopaDemoObj::MeetKoopaDemoObjNrvFallObjects::sInstance);
    }
}

void MeetKoopaDemoObj::exeFallObjects() {
    if (!MR::isLessStep(this, 90)) {
        if (MR::isIntervalStep(this, 30)) {
            for (s32 i = 0; i < 5; i++) {
                if (MR::isDead(mFallObjs[i])) {
                    mFallObjs[i]->appear();
                    break;
                }
            }
        }
        if (MR::isStep(this, 240)) {
            setNerve(&NrvMeetKoopaDemoObj::MeetKoopaDemoObjNrvWaitBack::sInstance);
        }
    }
}

void MeetKoopaDemoObj::exeDemo() {
    if (MR::isFirstStep(this)) {
        if (MR::isDead(mKoopa)) {
            mKoopa->makeActorAppeared();
        }
        mKoopaArm->makeActorAppeared();
        mPeach->makeActorAppeared();
        mTicoBaby->makeActorAppeared();
        for (s32 i = 0; i < 5; i++) {
            mFallObjs[i]->makeActorDead();
        }
        MR::resetPosition(mKoopa, sPeachKidnapPosName);
        MR::resetPosition(mKoopaArm, sPeachKidnapPosName);
        MR::resetPosition(mPeach, sPeachKidnapPosName);
        MR::resetPosition(mTicoBaby, sPeachKidnapPosName);
    }
    if (MR::isDemoPartActive("ピーチ発見") && MR::isStoryBookDemoActive() == 0) {
        JMath::gekko_ps_copy12(&mMtx, MR::getJointMtx(mPeach, "All_Root"));
        MR::tryTalkTimeKeepDemoForceWithoutDemo(mTalkCtrl);
    }
    MR::requestStageBGMVolumeDown(0, 0);
}

KoopaAssaultFallObj::KoopaAssaultFallObj(const char* pName) : LiveActor(pName) {
    mBasePos.set(0.0f, 0.0f, 0.0f);
    mModel = nullptr;
}

void KoopaAssaultFallObj::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "CrystalCageS", "KoopaAssault", false);
    mScale.set(2.0f, 2.0f, 2.0f);
    mBasePos.set(mPosition);

    TVec3f up(0.0f, 1.0f, 0.0f);
    MR::calcUpVec(&up, this);
    JMAVECScaleAdd(&up, &mBasePos, &mPosition, 6000.0f);

    mModel = MR::createModelObjMapObjStrongLight("クッパ襲撃落下オブジェ壊れモデル", "CrystalCageSBreak", nullptr);
    mModel->mPosition.set(mBasePos);
    mModel->mRotation.set(mRotation);

    initNerve(GET_NERVE(KoopaAssaultFallObj, KoopaAssaultFallObjNrvFall), 0);
    makeActorDead();
}

void KoopaAssaultFallObj::appear() {
    TVec3f up(0.0f, 1.0f, 0.0f);
    MR::calcUpVec(&up, this);
    JMAVECScaleAdd(&up, &mBasePos, &mPosition, 6000.0f);

    LiveActor::appear();
    MR::showModel(this);
    MR::invalidateClipping(this);
    setNerve(GET_NERVE(KoopaAssaultFallObj, KoopaAssaultFallObjNrvFall));
}

void KoopaAssaultFallObj::makeActorDead() {
    mModel->makeActorDead();
    LiveActor::makeActorDead();
}

void KoopaAssaultFallObj::exeFall() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "FallStart", -1, -1, -1);
        MR::emitEffect(this, "Blur");
    }

    TVec3f zero(0.0f, 0.0f, 0.0f);
    TVec3f up(0.0f, 1.0f, 0.0f);
    MR::calcUpVec(&up, this);
    JMAVECScaleAdd(&up, &mBasePos, &zero, 6000.0f);
    MR::vecBlend(&mPosition, zero, mBasePos, MR::calcNerveEaseInRate(this, 90));

    if (MR::isStep(this, 90)) {
        MR::deleteEffect(this, "Blur");
        MR::tryHitReaction(this, "砕ける");
        setNerve(GET_NERVE(KoopaAssaultFallObj, KoopaAssaultFallObjNrvBreak));
    }
}

void KoopaAssaultFallObj::exeBreak() {
    if (MR::isFirstStep(this)) {
        MR::hideModel(this);
        mModel->appear();
        MR::invalidateClipping(mModel);
        MR::startAction(mModel, "Break");
    }

    if (MR::isActionEnd(mModel)) {
        MR::validateClipping(this);
        kill();
    }
}

MeetKoopaDemoObj::~MeetKoopaDemoObj() {
}

KoopaAssaultFallObj::~KoopaAssaultFallObj() {
}
