#include "Game/Player/MarioHolder.hpp"
#include "Game/Util.hpp"
#include "Game/Scene/SceneObjHolder.hpp"

MarioHolder::MarioHolder() : NameObj("マリオ保持") {
    mActor = nullptr;
    mBeeResourceEnabled = false;
    mHopperResourceEnabled = false;
    mFireResourceEnabled = false;
    mIceResourceEnabled = false;
    mInvincibleResourceEnabled = false;
    mTeresaResourceEnabled = false;
    mTornadoResourceEnabled = false;
    mUnknownResourceEnabled = false;
    mYoshiResourceEnabled = false;
    mYoshiBalloonResourceEnabled = false;
    mCloudResourceEnabled = false;
    mRockResourceEnabled = false;
    mHairAndHeadResourceEnabled = false;
    mFreezeIceResourceEnabled = false;

}

MarioHolder::~MarioHolder() {
}

void MarioHolder::setMarioActor(MarioActor* pActor) {
    mActor = pActor;
}

MarioActor* MarioHolder::getMarioActor() const {
    return mActor;
}

namespace MR {
    MarioHolder* getMarioHolder() {
        return MR::getSceneObj< MarioHolder >(SceneObj_MarioHolder);
    }
    void enablePlayerResourceBee() {
        MR::getSceneObj<MarioHolder>(SceneObj_MarioHolder)->mBeeResourceEnabled = true;
    }

    void enablePlayerResourceHopper() {
        MR::getSceneObj<MarioHolder>(SceneObj_MarioHolder)->mHopperResourceEnabled = true;
    }

    void enablePlayerResourceFire() {
        MR::getSceneObj<MarioHolder>(SceneObj_MarioHolder)->mFireResourceEnabled = true;
    }

    void enablePlayerResourceIce() {
        MR::getSceneObj<MarioHolder>(SceneObj_MarioHolder)->mIceResourceEnabled = true;
    }

    void enablePlayerResourceInvincible() {
        MR::getSceneObj<MarioHolder>(SceneObj_MarioHolder)->mInvincibleResourceEnabled = true;
    }

    void enablePlayerResourceTeresa() {
        MR::getSceneObj<MarioHolder>(SceneObj_MarioHolder)->mTeresaResourceEnabled = true;
    }

    void enablePlayerResourceTornado() {
        MR::getSceneObj<MarioHolder>(SceneObj_MarioHolder)->mTornadoResourceEnabled = true;
    }

    void enablePlayerResourceYoshi() {
        MR::getSceneObj<MarioHolder>(SceneObj_MarioHolder)->mYoshiResourceEnabled = true;
    }

    void enablePlayerResourceYoshiBalloon() {
        MR::getSceneObj<MarioHolder>(SceneObj_MarioHolder)->mYoshiBalloonResourceEnabled = true;
    }

    void enablePlayerResourceCloud() {
        MR::getSceneObj<MarioHolder>(SceneObj_MarioHolder)->mCloudResourceEnabled = true;
    }

    void enablePlayerResourceRock() {
        MR::getSceneObj<MarioHolder>(SceneObj_MarioHolder)->mRockResourceEnabled = true;
    }

    void enablePlayerResourceHairAndHead() {
        MR::getSceneObj<MarioHolder>(SceneObj_MarioHolder)->mHairAndHeadResourceEnabled = true;
    }

    void enablePlayerResourceFreezeIce() {
        MR::getSceneObj<MarioHolder>(SceneObj_MarioHolder)->mFreezeIceResourceEnabled = true;
    }

    bool isCreatePlayerResourceBee() {
        return MR::getSceneObj<MarioHolder>(SceneObj_MarioHolder)->mBeeResourceEnabled;
    }

    bool isCreatePlayerResourceHopper() {
        return MR::getSceneObj<MarioHolder>(SceneObj_MarioHolder)->mHopperResourceEnabled;
    }

    bool isCreatePlayerResourceFire() {
        return MR::getSceneObj<MarioHolder>(SceneObj_MarioHolder)->mFireResourceEnabled;
    }

    bool isCreatePlayerResourceIce() {
        return MR::getSceneObj<MarioHolder>(SceneObj_MarioHolder)->mIceResourceEnabled;
    }

    bool isCreatePlayerResourceInvincible() {
        return MR::getSceneObj<MarioHolder>(SceneObj_MarioHolder)->mInvincibleResourceEnabled;
    }

    bool isCreatePlayerResourceTeresa() {
        return MR::getSceneObj<MarioHolder>(SceneObj_MarioHolder)->mTeresaResourceEnabled;
    }

    bool isCreatePlayerResourceTornado() {
        return MR::getSceneObj<MarioHolder>(SceneObj_MarioHolder)->mTornadoResourceEnabled;
    }

    bool isCreatePlayerResourceYoshi() {
        return MR::getSceneObj<MarioHolder>(SceneObj_MarioHolder)->mYoshiResourceEnabled;
    }

    bool isCreatePlayerResourceYoshiBalloon() {
        return MR::getSceneObj<MarioHolder>(SceneObj_MarioHolder)->mYoshiBalloonResourceEnabled;
    }

    bool isCreatePlayerResourceCloud() {
        return MR::getSceneObj<MarioHolder>(SceneObj_MarioHolder)->mCloudResourceEnabled;
    }

    bool isCreatePlayerResourceRock() {
        return MR::getSceneObj<MarioHolder>(SceneObj_MarioHolder)->mRockResourceEnabled;
    }

    bool isCreatePlayerResourceHairAndHead() {
        return MR::getSceneObj<MarioHolder>(SceneObj_MarioHolder)->mHairAndHeadResourceEnabled;
    }

    bool isCreatePlayerResourceFreezeIce() {
        return MR::getSceneObj<MarioHolder>(SceneObj_MarioHolder)->mFreezeIceResourceEnabled;
    }

    void enablePlayerResourceHopperComet() {
        enablePlayerResourceHopper();
    }

    void enablePlayerResourceInvincibleComet() {
        enablePlayerResourceInvincible();
    }

    void enablePlayerResourceNoHat() {
        enablePlayerResourceHairAndHead();
    }

};  // namespace MR
