#include "Game/Util/BgmUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Demo/MeetKoopaDemoObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

// Nerve objects (sbss) and their vtables/ctors live in the unsplit area next to this unit.
extern "C" s32 lbl_807D1DE8[1];
extern "C" s32 lbl_807D1DEC[1];
extern "C" s32 lbl_807D1DF0[1];
extern "C" s32 lbl_807D1DF4[1];
extern "C" s32 lbl_807D1DF8[1];

// Target-side unnamed helpers.
extern "C" LiveActor* __ct__19KoopaAssaultFallObjFPCc(LiveActor*, const char*);
extern "C" void tryTalkTimeKeepDemoForceWithoutDemo__2MRFP15TalkMessageCtrl(TalkMessageCtrl*);

namespace MR {
    s32 getRandom(s32, s32);
};

namespace {
    // rodata table indexed by (nerve step / 30) (see exeWait)
    const s32 sShakeTable[16] = {-1, -1, 0, -1, -1, -1, 0, 0, -1, -1, -1, -1, 1, -1, -1, -1};

    const char* sPeachKidnapName = "ピーチ誘拐";
    const char* sDemoMeetKoopaName = "DemoMeetKoopa";
    const char* sPeachKidnapPosName = "ピーチ誘拐デモ基準点";

    // Out-of-line stand-ins for the target's TVec3 ctor and set<f> calls.
    TVec3f* makeVec3(TVec3f* pVec, f32 x, f32 y, f32 z) NO_INLINE;
    TVec3f* makeVec3(TVec3f* pVec, f32 x, f32 y, f32 z) {
        pVec->x = x;
        pVec->y = y;
        pVec->z = z;
        return pVec;
    }

    void copyVec3(TVec3f* pDst, const TVec3f* pSrc) NO_INLINE;
    void copyVec3(TVec3f* pDst, const TVec3f* pSrc) {
        pDst->set(*pSrc);
    }

    struct Mtx34Copy {
        f32 m[3][4];
    };

    // Struct copy of a 3x4 matrix (compiles to paired-single moves, as in the target).
    void copyMtx34(f32 (*pDst)[4], const f32 (*pSrc)[4]) {
        *reinterpret_cast<Mtx34Copy*>(pDst) = *reinterpret_cast<const Mtx34Copy*>(pSrc);
    }

    // Writes the 3x4 identity (target: JGeometry::TMatrix34::identity).
    void identityMtx(f32 (*pMtx)[4]) NO_INLINE;
    void identityMtx(f32 (*pMtx)[4]) {
        pMtx[0][0] = 1.0f;
        pMtx[0][1] = 0.0f;
        pMtx[0][2] = 0.0f;
        pMtx[0][3] = 0.0f;
        pMtx[1][0] = 0.0f;
        pMtx[1][1] = 1.0f;
        pMtx[1][2] = 0.0f;
        pMtx[1][3] = 0.0f;
        pMtx[2][0] = 0.0f;
        pMtx[2][1] = 0.0f;
        pMtx[2][2] = 1.0f;
        pMtx[2][3] = 0.0f;
    }
};  // namespace

MeetKoopaDemoObj::MeetKoopaDemoObj(const char* pName) : LiveActor(pName), mKoopa(nullptr), mKoopaArm(nullptr), mPeach(nullptr), mTicoBaby(nullptr), mFallObjs(nullptr), mTalkCtrl(nullptr) {
    identityMtx(mMtx);
}

void MeetKoopaDemoObj::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::connectToSceneMapObjMovement(this);
    MR::invalidateClipping(this);
    if (MR::tryRegisterDemoCast(this, rIter)) {
        MR::registerDemoActionNerve(this, reinterpret_cast<const Nerve*>(lbl_807D1DF8), nullptr);
    }

    mKoopa = reinterpret_cast<LiveActor*>(MR::createDemoObj("デモ用クッパ", "DemoKoopa", static_cast<MtxPtr>(nullptr)));
    copyVec3(&mKoopa->mPosition, &mPosition);
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

    TVec3f talkPos;
    mTalkCtrl = MR::createTalkCtrlDirect(mPeach, rIter, "StoryBook_MeetKoopa_PeachHelp", *makeVec3(&talkPos, 0.0f, 200.0f, 0.0f), mMtx);
    initNerve(reinterpret_cast<const Nerve*>(lbl_807D1DE8), 0);

    mFallObjs = new LiveActor*[5];
    for (s32 i = 0; i < 5; i++) {
        LiveActor* obj = static_cast<LiveActor*>(::operator new(0xA0));
        if (obj != nullptr) {
            obj = __ct__19KoopaAssaultFallObjFPCc(obj, "クッパ襲撃落下オブジェ");
        }
        mFallObjs[i] = obj;
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
        setNerve(reinterpret_cast<const Nerve*>(lbl_807D1DEC));
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
        setNerve(reinterpret_cast<const Nerve*>(lbl_807D1DF0));
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
            setNerve(reinterpret_cast<const Nerve*>(lbl_807D1DF4));
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
        copyMtx34(mMtx, MR::getJointMtx(mPeach, "All_Root"));
        tryTalkTimeKeepDemoForceWithoutDemo__2MRFP15TalkMessageCtrl(mTalkCtrl);
    }
    MR::requestStageBGMVolumeDown(0, 0);
}
