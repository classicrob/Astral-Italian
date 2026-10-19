#include "Game/Screen/LensFlare.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/BrightObj.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/System/DrawSyncManager.hpp"
#include "Game/Util/AreaObjUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/TriggerChecker.hpp"

namespace NrvLensFlareModel {
    LensFlareModelNrvKill LensFlareModelNrvKill::sInstance;
    LensFlareModelNrvHide LensFlareModelNrvHide::sInstance;
    LensFlareModelNrvShow LensFlareModelNrvShow::sInstance;
    LensFlareModelNrvFadeIn LensFlareModelNrvFadeIn::sInstance;
    LensFlareModelNrvFadeOut LensFlareModelNrvFadeOut::sInstance;
}

namespace {
    LensFlareDirector* getLensFlareDirector() {
        return MR::getSceneObj< LensFlareDirector >(SceneObj_LensFlareDirector);
    }
};  // namespace

LensFlareModel::LensFlareModel(const char* pName, const char* pArcName)
    : LiveActor(pName), mBrightRate(), mFadeLevel(), mFadeSpeed(), mAreaTrigger(new TriggerChecker()), mBrightTrigger(new TriggerChecker()) {
    MR::initActor(this, pArcName, false);

    MR::connectToScene3DModelFor2D(this);

    MR::invalidateClipping(this);

    initNerve(GET_NERVE_DIRECT(NrvLensFlareModel, LensFlareModelNrvKill));

    kill();
}

void LensFlareModel::update(bool param1, bool param2) {
    mAreaTrigger->update(param1);

    if (mAreaTrigger->getOnTrigger()) {
        notifyInArea();
    } else if (mAreaTrigger->getOffTrigger()) {
        setNerve(GET_NERVE_DIRECT(NrvLensFlareModel, LensFlareModelNrvFadeOut));
    }

    mBrightTrigger->update(param2 && param1);

    if (mBrightTrigger->getOnTrigger()) {
        if (isNerve(GET_NERVE_DIRECT(NrvLensFlareModel, LensFlareModelNrvHide))) {
            setNerve(GET_NERVE_DIRECT(NrvLensFlareModel, LensFlareModelNrvShow));
        }
    } else if (mBrightTrigger->getOffTrigger() && (isNerve(GET_NERVE_DIRECT(NrvLensFlareModel, LensFlareModelNrvShow)) || isNerve(GET_NERVE_DIRECT(NrvLensFlareModel, LensFlareModelNrvFadeIn)))) {
        setNerve(GET_NERVE_DIRECT(NrvLensFlareModel, LensFlareModelNrvHide));
    }
}

void LensFlareModel::exeKill() {
    if (MR::isFirstStep(this)) {
        MR::hideModel(this);
        kill();
    }
}

void LensFlareModel::exeHide() {
    if (MR::isFirstStep(this)) {
        MR::hideModel(this);
        mFadeLevel = 0.0f;
    }
}

void LensFlareModel::exeShow() {
    if (MR::isFirstStep(this)) {
        MR::showModel(this);
        mFadeLevel = 1.0f;
    }
}

void LensFlareModel::exeFadeIn() {
    if (MR::isFirstStep(this)) {
        MR::showModel(this);
    }

    mFadeLevel += mFadeSpeed;

    if (mFadeLevel >= 1.0f) {
        mFadeLevel = 1.0f;
        setNerve(GET_NERVE_DIRECT(NrvLensFlareModel, LensFlareModelNrvShow));
    }
}

void LensFlareModel::exeFadeOut() {
    mFadeLevel -= mFadeSpeed;

    if (mFadeLevel <= 0.0f) {
        mFadeLevel = 0.0f;
        setNerve(GET_NERVE_DIRECT(NrvLensFlareModel, LensFlareModelNrvKill));
    }
}

void LensFlareModel::appear() {
    LiveActor::appear();
    appearAnim();
}

void LensFlareModel::control() {
    controlAnim();
}

void LensFlareModel::notifyInArea() {
    if (MR::isDead(this)) {
        appear();
    }

    if (mBrightTrigger->getLevel() > 0.0f) {
        setNerve(GET_NERVE_DIRECT(NrvLensFlareModel, LensFlareModelNrvFadeIn));
    } else {
        setNerve(GET_NERVE_DIRECT(NrvLensFlareModel, LensFlareModelNrvHide));
    }
}

LensFlareRing::LensFlareRing() : LensFlareModel("レンズフレアリング", "LensFlare"), mRingFrame() {
    mScale.x = 0.135f;
    mScale.y = 0.135f;
    mScale.z = 0.135f;
    mFadeSpeed = 0.05f;
}

void LensFlareRing::appearAnim() {
    MR::startBckWithInterpole(this, "LensFlare", 0);
}

void LensFlareRing::controlAnim() {
    MR::startBrk(this, "LensFlare");
    MR::setBrkRate(this, 0.0f);
    MR::setBrkFrame(this, (1.0f - mBrightRate * mFadeLevel) * MR::getBrkCtrl(this)->getEnd());

    if (MR::isActionEnd(this)) {
        MR::startBckWithInterpole(this, "LensFlare", 0);
    }

    f32 frame = mRingFrame * MR::getBckCtrl(this)->getEnd();
    MR::setBckFrameAndStop(this, frame);
}

LensFlareGlow::LensFlareGlow() : LensFlareModel("グレア（円形）", "GlareGlow") {
    mFadeSpeed = 0.05f;
}

void LensFlareGlow::appearAnim() {
    MR::startBtk(this, "GlareGlow");
}

void LensFlareGlow::controlAnim() {
    MR::startBrk(this, "GlareGlow");
    MR::setBrkRate(this, 0.0f);
    MR::setBrkFrame(this, (1.0f - mBrightRate * mFadeLevel) * MR::getBrkCtrl(this)->getEnd());
}

LensFlareLine::LensFlareLine() : LensFlareModel("グレア（ライン）", "GlareLine") {
    mFadeSpeed = 0.05f;
}

void LensFlareLine::appearAnim() {
}

void LensFlareLine::controlAnim() {
    MR::startBrk(this, "GlareLine");
    MR::setBrkRate(this, 0.0f);
    MR::setBrkFrame(this, (1.0f - mBrightRate * mFadeLevel) * MR::getBrkCtrl(this)->getEnd());
}

LensFlareDirector::LensFlareDirector()
    : NameObj("レンズフレア管理"), mRing(), mGlow(), mLine(), mBrightObjCount(), mBrightCenter(0.0f, 0.0f), mBright(), mCenter(0.0f, 0.0f), mNowCenter(0.0f, 0.0f), mDrawSyncId(),
      mDrawSyncTokenIndex(), mBrightCamInfo() {
    mDrawSyncId = DrawSyncManager::sInstance->setCallback(3, 2, this);
}

LensFlareDirector::~LensFlareDirector() {
}

void LensFlareDirector::init(const JMapInfoIter& rIter) {
    mRing = new LensFlareRing();
    mGlow = new LensFlareGlow();
    mLine = new LensFlareLine();
    mBrightCamInfo = new BrightCamInfo();

    MR::connectToSceneMapObjMovement(this);
}

void LensFlareDirector::movement() {
    s32 area = checkArea();
    controlFlare(area, checkBrightObj(area != 0));

    mBrightCamInfo->write(mDrawSyncTokenIndex, MR::getCameraViewMtx(), MR::getCameraProjectionMtx(), MR::getCamYdir(), MR::getCamPos());
}

void LensFlareDirector::drawSyncCallback(u16 val) {
    u32 i;
    s32 val2 = val - mDrawSyncId;
    for (i = 0; i < mBrightObjCount; i++) {
        mBrightObjArray[i]->calcBrightInfo(val2, *mBrightCamInfo);
    }
}

void LensFlareDirector::pauseOff() {
    MR::requestMovementOn(this);
    MR::requestMovementOn(mRing);
    MR::requestMovementOn(mGlow);
    MR::requestMovementOn(mLine);
}

void LensFlareDirector::setDrawSyncToken() {
    DrawSyncManager::sInstance->pushBreakPoint();

    GXSetDrawSync(mDrawSyncId + mDrawSyncTokenIndex);

    mDrawSyncTokenIndex = mDrawSyncTokenIndex ^ 0x1;
}

s32 LensFlareDirector::checkArea() {
    s32 area = 0;
    AreaObj* pAreaObj = MR::getAreaObj("LensFlareArea", *MR::getPlayerPos());

    if (pAreaObj != nullptr) {
        area = MR::getAreaObjArg(pAreaObj, 0);

        if (area == -1) {
            area = 0xFFFF;
        }
    }

    return area;
}

bool LensFlareDirector::checkBrightObj(bool b1) {
    bool ret = false;
    if (b1 && mBrightObjCount != 0) {
        TVec3f camPos = MR::getCamPos();

        s32 index = -1;
        for (u32 i = 0; i < mBrightObjCount; i++) {
            if (mBrightObjArray[i]->getBright() > 0.0f) {
                index = i;
                break;
            }
        }

        if (index >= 0) {
            ret = true;

            mBrightCenter.set(*mBrightObjArray[index]->getBrightCenter());
            mBright = mBrightObjArray[index]->getBright();
            mCenter.set(*mBrightObjArray[index]->getCenter());
            mBrightObjArray[index]->getNowCenter(&mNowCenter);
        }

        for (u32 i = 0; i < mBrightObjCount; i++) {
            mBrightObjArray[i]->endRead();
        }
    }

    return ret;
}

void LensFlareDirector::controlFlare(s32 area, bool hasBrightObj) {
    mRing->update(area >> 1 & 1, hasBrightObj);
    mGlow->update(area >> 2 & 1, hasBrightObj);
    mLine->update(area >> 3 & 1, hasBrightObj);

    if (!hasBrightObj || area == 0) {
        return;
    }

    TVec2f position(mBrightCenter - mCenter);
    position += mNowCenter;
    TVec3f flarePos(position.x, -position.y, 0.0f);
    MR::setPosition(mRing, flarePos);
    MR::setPosition(mGlow, flarePos);
    MR::setPosition(mLine, flarePos);

    TVec2f screenCenter(MR::getScreenWidth() / 2.0f, static_cast< s32 >(JUTGetVideoManager()->getEfbHeight()) / 2.0f);

    f32 screenRadius = screenCenter.length();
    TVec2f offset(screenCenter - position);
    f32 distanceRate = offset.length() / screenRadius;

    TVec3f rotation(0.0f, 0.0f, 0.0f);
    TVec2f direction(offset.x, offset.y);

    if (MR::isNearZero(direction)) {
        rotation.z = 0.0f;
    } else {
        MR::normalize(&direction);
        rotation.z = MR::calcRotateY(direction.x, direction.y);
    }

    MR::setRotation(mRing, rotation);
    f32 ringFrame = distanceRate;
    if (ringFrame > 1.0f) {
        ringFrame = 1.0f;
    }

    mRing->mRingFrame = ringFrame;
    mRing->mBrightRate = MR::clamp((1.0f - distanceRate) * mBright, 0.0f, 1.0f);
    mGlow->mBrightRate = MR::clamp((1.0f - distanceRate) * mBright, 0.0f, 1.0f);
    mLine->mBrightRate = MR::clamp((1.0f - distanceRate) * mBright, 0.0f, 1.0f);
}

namespace MR {
    void addBrightObj(BrightObjBase* pBrightObj) {
        if (!MR::isExistSceneObj(SceneObj_LensFlareDirector)) {
            MR::createSceneObj(SceneObj_LensFlareDirector);
        }

        LensFlareDirector* pDirector = ::getLensFlareDirector();
        pDirector->mBrightObjArray[pDirector->mBrightObjCount++] = pBrightObj;
    }

    void setLensFlareDrawSyncToken() {
        if (MR::isExistSceneObj(SceneObj_LensFlareDirector)) {
            ::getLensFlareDirector()->setDrawSyncToken();
        }
    }

    u32 getLensFlareDrawSyncTokenIndex() {
        return ::getLensFlareDirector()->mDrawSyncTokenIndex;
    }
};  // namespace MR

TVec2f LensFlare_FORCE_MATCH(const TVec2f& rA, const TVec2f& rB) {
    return rA - rB;
}

namespace NrvLensFlareModel {
    LensFlareModelNrvKill::LensFlareModelNrvKill() {}
    LensFlareModelNrvHide::LensFlareModelNrvHide() {}
    LensFlareModelNrvShow::LensFlareModelNrvShow() {}
    LensFlareModelNrvFadeIn::LensFlareModelNrvFadeIn() {}
    LensFlareModelNrvFadeOut::LensFlareModelNrvFadeOut() {}
    void LensFlareModelNrvFadeOut::execute(Spine* spine) const {
        LensFlareModel* host = static_cast<LensFlareModel*>(spine->mExecutor);
        host->exeFadeOut();
    }
    void LensFlareModelNrvFadeIn::execute(Spine* spine) const {
        LensFlareModel* host = static_cast<LensFlareModel*>(spine->mExecutor);
        host->exeFadeIn();
    }
    void LensFlareModelNrvShow::execute(Spine* spine) const {
        LensFlareModel* host = static_cast<LensFlareModel*>(spine->mExecutor);
        host->exeShow();
    }
    void LensFlareModelNrvHide::execute(Spine* spine) const {
        LensFlareModel* host = static_cast<LensFlareModel*>(spine->mExecutor);
        host->exeHide();
    }
    void LensFlareModelNrvKill::execute(Spine* spine) const {
        LensFlareModel* host = static_cast<LensFlareModel*>(spine->mExecutor);
        if (MR::isFirstStep(host)) {
            MR::hideModel(host);
            host->kill();
        }
    }
}
