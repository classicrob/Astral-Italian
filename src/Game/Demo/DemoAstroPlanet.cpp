#include "Game/Demo/DemoAstroPlanet.hpp"
#include "Game/Util.hpp"

DemoAstroPlanet::DemoAstroPlanet(const char* pName, MtxPtr pMtx)
    : LiveActor(pName), mJointController(NULL), mBaseMtx(pMtx), mEaseTimer(0.0f), mRotationY(0.0f), mIsRotating(false) {
    if (pMtx != NULL) {
        mPosition.set(pMtx[0][3], pMtx[1][3], pMtx[2][3]);
    }
}

DemoAstroPlanet::~DemoAstroPlanet() {
}

void DemoAstroPlanet::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "DemoAstroPlanet", false);

    JointControlDelegator< DemoAstroPlanet >::func calcFunc = &DemoAstroPlanet::calcJointMatrix;
    JointControlDelegator< DemoAstroPlanet >* pDelegator = new JointControlDelegator< DemoAstroPlanet >(this, calcFunc, NULL);
    MR::setJointControllerParam(pDelegator, this, "Model");
    mJointController = pDelegator;

    makeActorAppeared();
}

void DemoAstroPlanet::control() {
    if (mIsRotating && !MR::isStoryBookDemoActive()) {
        f32 ease = 1.0f;
        s32 limit = 60;
        if (mEaseTimer < (f32)limit) {
            f32 t = MR::normalize(mEaseTimer, 0.0f, (f32)limit);
            ease = MR::getEaseInValue(t, 0.0f, 1.0f, 1.0f);
            mEaseTimer += 1.0f;
        }

        mRotationY += 0.05f * ease;
        mRotationY = MR::repeatDegree(mRotationY);
    }
}

void DemoAstroPlanet::calcAndSetBaseMtx() {
    if (mBaseMtx != NULL) {
        mPosition.set(mBaseMtx[0][3], mBaseMtx[1][3], mBaseMtx[2][3]);
        MR::setBaseTRMtx(this, mBaseMtx);
    } else {
        LiveActor::calcAndSetBaseMtx();
    }

    mJointController->registerCallBack();
}

bool DemoAstroPlanet::calcJointMatrix(TPos3f* pMtx, const JointControllerInfo&) {
    MR::rotateMtxLocalYDegree(pMtx->toMtxPtr(), mRotationY);
    return true;
}
