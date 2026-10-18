#include "Game/Demo/TransformFaceTicoBabyDemoObj.hpp"
#include "Game/LiveActor/DemoObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

// Demo name pointer (set up in another TU's sdata: "マリオ顔惑星変化").
extern "C" const char* lbl_807CF780;

// Function name for the demo cast (DemoMeetMeister); initialized in this TU's sdata.
const char* sDemoFuncName = "DemoMeetMeister";

namespace NrvTransformFaceTicoBabyDemoObj {
    class TransformFaceTicoBabyDemoObjNrvWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static TransformFaceTicoBabyDemoObjNrvWait sInstance;
    };

    class TransformFaceTicoBabyDemoObjNrvAppear : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static TransformFaceTicoBabyDemoObjNrvAppear sInstance;
    };

    TransformFaceTicoBabyDemoObjNrvWait TransformFaceTicoBabyDemoObjNrvWait::sInstance ATTRIBUTE_WEAK;
    TransformFaceTicoBabyDemoObjNrvAppear TransformFaceTicoBabyDemoObjNrvAppear::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvTransformFaceTicoBabyDemoObj

TransformFaceTicoBabyDemoObj::TransformFaceTicoBabyDemoObj(const char* pName) : LiveActor(pName) {
    mDemoObj = nullptr;
}

void TransformFaceTicoBabyDemoObj::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::connectToSceneMapObjMovement(this);
    MR::invalidateClipping(this);
    initNerve(&NrvTransformFaceTicoBabyDemoObj::TransformFaceTicoBabyDemoObjNrvWait::sInstance, 0);

    MR::tryRegisterDemoCast(this, lbl_807CF780, rIter);
    mDemoObj = MR::createDemoObjWithAnimArc("ベビチコ[顔惑星変化用]", "TicoBaby", "TicoBabyAnim", MR::getPlayerBaseMtx());
    MR::tryInitDemoSheetAnim(mDemoObj, rIter, lbl_807CF780, sDemoFuncName);
    MR::invalidateClipping(mDemoObj);
    mDemoObj->makeActorDead();
    makeActorDead();
}

void TransformFaceTicoBabyDemoObj::kill() {
    LiveActor::kill();
    mDemoObj->makeActorDead();
}

TransformFaceTicoBabyDemoObj::~TransformFaceTicoBabyDemoObj() {
}

namespace NrvTransformFaceTicoBabyDemoObj {
    void TransformFaceTicoBabyDemoObjNrvWait::execute(Spine* pSpine) const {
        TransformFaceTicoBabyDemoObj* pActor = reinterpret_cast<TransformFaceTicoBabyDemoObj*>(pSpine->mExecutor);
        if (!MR::isDemoActive(lbl_807CF780)) {
            pActor->kill();
        }
    }

    void TransformFaceTicoBabyDemoObjNrvAppear::execute(Spine* pSpine) const {
        TransformFaceTicoBabyDemoObj* pActor = reinterpret_cast<TransformFaceTicoBabyDemoObj*>(pSpine->mExecutor);
        pActor->mDemoObj->makeActorAppeared();
        pActor->setNerve(&TransformFaceTicoBabyDemoObjNrvAppear::sInstance);
    }
};  // namespace NrvTransformFaceTicoBabyDemoObj
