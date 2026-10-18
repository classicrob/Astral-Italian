#include "Game/Scene/ModelWipeScene.hpp"
#include "Game/Camera/CameraContext.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/NameObj/NameObjGroup.hpp"
#include "Game/Scene/MultiSceneActor.hpp"
#include "Game/Util/DrawUtil.hpp"
#include "Game/Util/MutexHolder.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include <JSystem/J3DGraphBase/J3DDrawBuffer.hpp>
#include <JSystem/J3DGraphBase/J3DSys.hpp>

namespace NrvModelWipeScene {
    class Hide : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static Hide sInstance;
    };
    Hide Hide::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(Start, ModelWipeScene, Start);
    NEW_NERVE(Wait, ModelWipeScene, Wait);
    NEW_NERVE(End, ModelWipeScene, End);
}  // namespace NrvModelWipeScene

ModelWipeScene::ModelWipeScene()
    : Scene("モデルワイプシーン"), mDrawBufferOpa(nullptr), mDrawBufferXlu(nullptr), mCamera(nullptr), mGroup(nullptr), mIsForward(false),
      mIsPendingEnd(false), mCurrentActor(nullptr), mCloudActor(nullptr), mGalaxyActor(nullptr), mSpaceActor(nullptr) {
    mGroup = new NameObjGroup("処理グループ", 0x40);
    initNerve(&NrvModelWipeScene::Hide::sInstance, 0);
}

ModelWipeScene::~ModelWipeScene() {}

void ModelWipeScene::init() {
    mDrawBufferOpa = createDrawBuffer();
    mDrawBufferXlu = createDrawBuffer();

    mCamera = new CameraContext();

    // Look from (0, 0, 3000) at the origin with up = +Y.
    TVec3f lookAt(0.0f, 0.0f, 0.0f);
    TVec3f eye(0.0f, 0.0f, 3000.0f);
    TVec3f up(0.0f, 1.0f, 0.0f);

    TVec3f zAxis;
    PSVECSubtract(&lookAt, &eye, &zAxis);
    PSVECNormalize(&zAxis, &zAxis);
    zAxis.x = -zAxis.x;
    zAxis.y = -zAxis.y;
    zAxis.z = -zAxis.z;

    TVec3f xAxis;
    PSVECCrossProduct(&up, &zAxis, &xAxis);
    TVec3f yAxis;
    PSVECCrossProduct(&zAxis, &xAxis, &yAxis);
    PSVECNormalize(&xAxis, &xAxis);
    PSVECNormalize(&yAxis, &yAxis);

    TPos3f viewMtx;
    viewMtx.set(xAxis.x, xAxis.y, xAxis.z, -(eye.x * xAxis.x + eye.y * xAxis.y + eye.z * xAxis.z), yAxis.x, yAxis.y, yAxis.z,
                -(eye.x * yAxis.x + eye.y * yAxis.y + eye.z * yAxis.z), zAxis.x, zAxis.y, zAxis.z,
                -(eye.x * zAxis.x + eye.y * zAxis.y + eye.z * zAxis.z));

    mCamera->setViewMtx(viewMtx, false, false, lookAt);

    mCloudActor = new MultiSceneActor("雲ワイプ", "WipeModelCloud", reinterpret_cast< s32 >(&mCamera->mProjection), false);
    mCloudActor->initWithoutIter();
    mGroup->registerObj(mCloudActor);
    mCloudActor->kill();

    mGalaxyActor = new MultiSceneActor("銀河ワイプ", "WipeModelGalaxy", reinterpret_cast< s32 >(&mCamera->mProjection), false);
    mGalaxyActor->initWithoutIter();
    mGroup->registerObj(mGalaxyActor);
    mGalaxyActor->kill();

    mSpaceActor = new MultiSceneActor("宇宙ワイプ", "WipeModelSpace", reinterpret_cast< s32 >(&mCamera->mProjection), false);
    mSpaceActor->initWithoutIter();
    mGroup->registerObj(mSpaceActor);
    mSpaceActor->kill();
}

void ModelWipeScene::update() {
    if (!isNerve(&NrvModelWipeScene::Hide::sInstance)) {
        updateNerve();
        for (s32 i = 0; i < mGroup->getObjectCount(); i++) {
            mGroup->getObj(i)->movement();
        }
    }
}

void ModelWipeScene::calcAnim() {
    if (!isNerve(&NrvModelWipeScene::Hide::sInstance)) {
        OSLockMutex(&MR::MutexHolder< 0 >::sMutex);
        PSMTXCopy(mCamera->getViewMtx(), j3dSys.mViewMtx);
        for (s32 i = 0; i < mGroup->getObjectCount(); i++) {
            mGroup->getObj(i)->calcAnim();
        }
        calcViewAndEntry();
        OSUnlockMutex(&MR::MutexHolder< 0 >::sMutex);
    }
}

void ModelWipeScene::calcViewAndEntry() {
    if (!isNerve(&NrvModelWipeScene::Hide::sInstance)) {
        OSLockMutex(&MR::MutexHolder< 0 >::sMutex);
        PSMTXCopy(mCamera->getViewMtx(), j3dSys.mViewMtx);
        j3dSys.mDrawBuffer[0] = mDrawBufferOpa;
        j3dSys.mDrawBuffer[1] = mDrawBufferXlu;
        for (s32 i = 0; i < mGroup->getObjectCount(); i++) {
            mGroup->getObj(i)->calcViewAndEntry();
        }
        OSUnlockMutex(&MR::MutexHolder< 0 >::sMutex);
    }
}

void ModelWipeScene::draw() const {
    if (!isNerve(&NrvModelWipeScene::Hide::sInstance)) {
        MR::clearZBuffer();
        MR::reinitGX();
        OSLockMutex(&MR::MutexHolder< 0 >::sMutex);
        PSMTXCopy(mCamera->getViewMtx(), j3dSys.mViewMtx);
        GXSetProjection(mCamera->mProjection, GX_PERSPECTIVE);
        j3dSys.drawInit();
        j3dSys.mDrawMode = 3;
        mDrawBufferOpa->draw();
        mDrawBufferOpa->frameInit();
        j3dSys.mDrawMode = 4;
        mDrawBufferXlu->draw();
        mDrawBufferXlu->frameInit();
        OSUnlockMutex(&MR::MutexHolder< 0 >::sMutex);
    }
}

void ModelWipeScene::setCurrentActor(MultiSceneActor* pActor) {
    if (mCurrentActor != nullptr) {
        mCurrentActor->kill();
        mCurrentActor = nullptr;
    }
    mIsPendingEnd = false;
    mCurrentActor = pActor;
    pActor->appear();

    const char* pName = mIsForward ? "Start" : "OutStart";
    MultiScene::startBckIfExist(mCurrentActor, pName);
    MultiScene::startBtkIfExist(mCurrentActor, pName);
    MultiScene::startBrkIfExist(mCurrentActor, pName);
    MultiScene::startBtpIfExist(mCurrentActor, pName);
    MultiScene::startBvaIfExist(mCurrentActor, pName);
}

void ModelWipeScene::startCloudWipeForward() {
    mIsForward = true;
    setCurrentActor(mCloudActor);
    setNerve(&NrvModelWipeScene::Start::sInstance);
}

void ModelWipeScene::startGalaxyWipeForward() {
    mIsForward = true;
    setCurrentActor(mGalaxyActor);
    MR::startSystemSE("SE_SY_GMAP_TO_WMAP", -1, -1);
    MR::onDestroySceneKeepAllSound();
    setNerve(&NrvModelWipeScene::Start::sInstance);
}

void ModelWipeScene::startSpaceWipeForward() {
    mIsForward = true;
    setCurrentActor(mSpaceActor);
    MR::startSystemSE("SE_SY_WMAP_TO_SHIP", -1, -1);
    MR::onDestroySceneKeepAllSound();
    setNerve(&NrvModelWipeScene::Start::sInstance);
}

void ModelWipeScene::startCloudWipeBackward() {
    mIsForward = false;
    setCurrentActor(mCloudActor);
    MR::startSystemSE("SE_SY_SHIP_TO_WMAP", -1, -1);
    MR::onDestroySceneKeepAllSound();
    setNerve(&NrvModelWipeScene::Start::sInstance);
}

void ModelWipeScene::startGalaxyWipeBackward() {
    mIsForward = false;
    setCurrentActor(mGalaxyActor);
    MR::startSystemSE("SE_SY_WMAP_TO_GMAP", -1, -1);
    MR::onDestroySceneKeepAllSound();
    setNerve(&NrvModelWipeScene::Start::sInstance);
}

void ModelWipeScene::startSpaceWipeBackward() {
    mIsForward = false;
    setCurrentActor(mSpaceActor);
    MR::startSystemSE("SE_SY_SHIP_TO_WMAP", -1, -1);
    MR::onDestroySceneKeepAllSound();
    setNerve(&NrvModelWipeScene::Start::sInstance);
}

void ModelWipeScene::requestEnd() {
    mIsPendingEnd = true;
}

bool ModelWipeScene::tryEnd() {
    if (isNerve(&NrvModelWipeScene::Start::sInstance)) {
        return false;
    }
    if (isNerve(&NrvModelWipeScene::Hide::sInstance)) {
        return true;
    }
    if (isNerve(&NrvModelWipeScene::End::sInstance)) {
        return true;
    }
    if (MultiScene::isBckEnd(mCurrentActor)) {
        setNerve(&NrvModelWipeScene::End::sInstance);

        const char* pName = mIsForward ? "End" : "OutEnd";
        MultiScene::startBckIfExist(mCurrentActor, pName);
        MultiScene::startBtkIfExist(mCurrentActor, pName);
        MultiScene::startBrkIfExist(mCurrentActor, pName);
        MultiScene::startBtpIfExist(mCurrentActor, pName);
        MultiScene::startBvaIfExist(mCurrentActor, pName);
        return true;
    }
    return false;
}

void ModelWipeScene::forceHide() {
    if (!isNerve(&NrvModelWipeScene::Hide::sInstance)) {
        mCurrentActor->kill();
        mCurrentActor = nullptr;
        setNerve(&NrvModelWipeScene::Hide::sInstance);
    }
}

bool ModelWipeScene::isHideOrEnd() const {
    return isNerve(&NrvModelWipeScene::Hide::sInstance) || isNerve(&NrvModelWipeScene::End::sInstance);
}

bool ModelWipeScene::isHidden() const {
    return isNerve(&NrvModelWipeScene::Hide::sInstance);
}

void ModelWipeScene::exeStart() {
    if (MultiScene::isBckStopped(mCurrentActor)) {
        const char* pName = mIsForward ? "Loop" : "OutLoop";
        MultiScene::startBckIfExist(mCurrentActor, pName);
        MultiScene::startBtkIfExist(mCurrentActor, pName);
        MultiScene::startBrkIfExist(mCurrentActor, pName);
        MultiScene::startBtpIfExist(mCurrentActor, pName);
        MultiScene::startBvaIfExist(mCurrentActor, pName);
        setNerve(&NrvModelWipeScene::Wait::sInstance);
    }
}

void ModelWipeScene::exeWait() {
    if (mIsPendingEnd && tryEnd()) {
        mIsPendingEnd = false;
    }
}

void ModelWipeScene::exeEnd() {
    if (MultiScene::isBckStopped(mCurrentActor)) {
        mCurrentActor->kill();
        mCurrentActor = nullptr;
        setNerve(&NrvModelWipeScene::Hide::sInstance);
    }
}
