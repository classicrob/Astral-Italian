#include "Game/Map/FileSelectItem.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/Map/FileSelectIconID.hpp"
#include "Game/Map/FileSelectItemDelegator.hpp"
#include "Game/Map/FileSelectModel.hpp"
#include "Game/NPC/MiiFaceParts.hpp"
#include "Game/NPC/MiiFacePartsHolder.hpp"
#include "Game/NPC/MiiFaceRecipe.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Screen/FileSelectNumber.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"

namespace NrvFileSelectItem {
    NEW_NERVE(FileSelectItemNrvNewWait, FileSelectItem, NewWait);
    NEW_NERVE(FileSelectItemNrvExistWait, FileSelectItem, ExistWait);
    NEW_NERVE(FileSelectItemNrvFormat, FileSelectItem, Format);
    NEW_NERVE(FileSelectItemNrvChangeFellow, FileSelectItem, ChangeFellow);
    NEW_NERVE(FileSelectItemNrvChangeMii, FileSelectItem, ChangeMii);
};  // namespace NrvFileSelectItem

namespace {
    bool checkCollisionOfPointAndCylinder(const TVec3f& rVec1, const TVec3f& rVec2, const TVec3f& rVec3, f32 f1) {
        f32 length = rVec3.length();
        TVec3f vecCopy(rVec3);
        MR::normalize(&vecCopy);

        TVec3f diff(rVec1 - rVec2);

        f32 dot = vecCopy.dot(diff);
        if (dot < 0.0f || dot > length) {
            return false;
        }

        vecCopy.scale(dot);
        return !(vecCopy.distance(diff) > f1);
    }

    const char* sFellowModel[7] = {"FileSelectDataMario", "FileSelectDataLuigi", "FileSelectDataYoshi", "FileSelectDataKinopio",
                                   "FileSelectDataPeach", "FileSelectDataRosetta", "FileSelectDataTico"};

    static const Vec sDataInfoOffset = {0.0f, 2150.0f, 0.0f};
};  // namespace

namespace FileSelectItemSub {
    NEW_NERVE(ScaleControllerNrvToSmall, ScaleController, ToSmall);
    NEW_NERVE(ScaleControllerNrvSmall, ScaleController, Small);
    NEW_NERVE(ScaleControllerNrvToBig, ScaleController, ToBig);
    NEW_NERVE(ScaleControllerNrvBig, ScaleController, Big);
    NEW_NERVE(BlinkControllerNrvOpen, BlinkController, Open);
    NEW_NERVE(BlinkControllerNrvShut, BlinkController, Shut);
    NEW_NERVE(BlinkControllerNrvSleep, BlinkController, Sleep);
    NEW_NERVE(BlinkControllerNrvBlink, BlinkController, Blink);
};  // namespace FileSelectItemSub

FileSelectItem::FileSelectItem(s32 a1, bool a2, const FileSelectIconID& rID, const char* pName) : LiveActor(pName) {
    mIsNew = a2;
    mPlanetMapObj = nullptr;
    mIconID = new FileSelectIconID(rID);
    mFaceParts = nullptr;
    mNumber = nullptr;
    mUnusedVector.set(0.0f);
    mFileIndex = a1;
    mIsPointed = 0;
    mIsInvalidateSelect = false;
    mHasInvalidMiiId = 0;
    mIsComplete = 0;
    mScaleCtrl = new FileSelectItemSub::ScaleController();
    mBlinkCtrl = new FileSelectItemSub::BlinkController(this);
    mDelegator = nullptr;
    mNeedsPointerReset = 1;
    mIsDragging = 0;
    mWasDragging = 0;
    mPreviousPointerPos.x = 0.0f;
    mPreviousPointerPos.y = 0.0f;
    mRotationSpeed = 0.0f;
    mIsInvalidRotate = true;
    mUseCopyEffect = 0;
    mTurnToFrontDuration = 0;
    mTurnToFrontStep = 0;
    mPlanetMtx.identity();
    mFellowMtx.identity();
    mMiiMtx.identity();

    mModels = new FileSelectModel*[7];

    for (u32 i = 0; i < 7; i++) {
        mModels[i] = nullptr;
    }
}

void FileSelectItem::init(const JMapInfoIter& rIter) {
    MR::connectToSceneMapObjMovement(this);
    MR::createSceneObj(SceneObj_MiiFacePartsHolder);
    createNew();
    createFellows();
    createMii();
    createNumber();
    MR::initStarPointerTarget(this, 1000.0f, TVec3f(0.0f, 900.0f, 0.0f));
    MR::invalidateClipping(this);

    TVec3f soundPos;
    soundPos.setAll<f32>(0.0f);
    initSound(4, "FileSelectItem", nullptr, soundPos);

    initNerve(GET_NERVE(FileSelectItem, FileSelectItemNrvNewWait));
    MR::createCenterScreenBlur();
    makeActorAppeared();
}

void FileSelectItem::appear() {
    LiveActor::appear();

    if (mIsNew) {
        setPlanetColor();
        setNerve(GET_NERVE(FileSelectItem, FileSelectItemNrvNewWait));
    } else {
        if (mIconID->isMii()) {
            killAllModels();
            mFaceParts->makeActorAppeared();
        } else {
            appearFellowModel();
        }

        setNerve(GET_NERVE(FileSelectItem, FileSelectItemNrvExistWait));
    }
}

void FileSelectItem::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    mIsPointed = 0;
    mIsInvalidateSelect = false;
}

void FileSelectItem::makeActorDead() {
    LiveActor::makeActorDead();
    mPlanetMapObj->makeActorDead();

    for (s32 i = 0; i < 7; i++) {
        mModels[i]->makeActorDead();
    }

    mFaceParts->makeActorDead();
}

bool FileSelectItem::isNew() const {
    return isNerve(GET_NERVE(FileSelectItem, FileSelectItemNrvNewWait));
}

bool FileSelectItem::isExist() const {
    return isNerve(GET_NERVE(FileSelectItem, FileSelectItemNrvExistWait));
}

void FileSelectItem::format() {
    setNerve(GET_NERVE(FileSelectItem, FileSelectItemNrvFormat));
    deleteCompleteEffect();
    mIsNew = 1;
}

void FileSelectItem::change(const FileSelectIconID& rID, bool a2) {
    mIconID->set(rID);

    if (rID.isMii()) {
        setNerve(GET_NERVE(FileSelectItem, FileSelectItemNrvChangeMii));
    } else {
        setNerve(GET_NERVE(FileSelectItem, FileSelectItemNrvChangeFellow));
    }

    deleteCompleteEffect();
    mUseCopyEffect = 0;
    mIsNew = 0;
    mIsComplete = a2;
}

void FileSelectItem::forceChange(const FileSelectIconID& rID, bool a2) {
    deleteCompleteEffect();
    mIconID->set(rID);
    if (rID.isMii()) {
        mFaceParts->changeFaceModel(MiiFaceRecipe(RFLDataSource_Official, mIconID->getMiiIndex(), RFLResolution_256, 33));
        killAllModels();
        mFaceParts->makeActorAppeared();
    } else {
        appearFellowModel();
    }

    mIsComplete = a2;
    emitCompleteEffect();
    setNerve(GET_NERVE(FileSelectItem, FileSelectItemNrvExistWait));
    mIsNew = 0;
}

void FileSelectItem::invalidateSelect() {
    if (mIsPointed) {
        offPointing();
    }

    mIsInvalidateSelect = true;
}

void FileSelectItem::validateSelect() {
    mIsInvalidateSelect = false;
}

void FileSelectItem::appearIndex() {
    mNumber->appear();
}

void FileSelectItem::disappearIndex() {
    mNumber->disappear();
}

void FileSelectItem::copyIconID(FileSelectIconID* pId) {
    pId->set(*mIconID);
}

void FileSelectItem::setSelectDelegator(FileSelectItemDelegatorBase* pDele) {
    mDelegator = pDele;
}

void FileSelectItem::onPointing() {
    if (!mIsInvalidateSelect) {
        if (isNerve(GET_NERVE(FileSelectItem, FileSelectItemNrvNewWait))) {
            MR::startActionSound(this, "FileUnUsedPoint", -1, -1, -1);
        } else {
            MR::startActionSound(this, "FilePoint", -1, -1, -1);
        }

        mNumber->onSelectIn();
        mIsPointed = 1;
        mScaleCtrl->setNerve(GET_NERVE_DIRECT(FileSelectItemSub, ScaleControllerNrvToBig));
        MR::tryRumblePadWeak(this, WPAD_CHAN0);
    }
}

void FileSelectItem::offPointing() {
    if (!mIsInvalidateSelect) {
        mNumber->onSelectOut();
        mIsPointed = 0;
        mScaleCtrl->setNerve(GET_NERVE_DIRECT(FileSelectItemSub, ScaleControllerNrvToSmall));
    }
}

void FileSelectItem::validateRotate() {
    mIsInvalidRotate = false;
}

void FileSelectItem::invalidateRotate() {
    mIsInvalidRotate = true;
    mNeedsPointerReset = 1;
    mRotationSpeed = 0.0f;
}

void FileSelectItem::turnToFront(s32 angle) {
    mTurnToFrontDuration = angle;
    mTurnToFrontStep = 0;
}

void FileSelectItem::requestRotateRefresh() {
    mNeedsPointerReset = 1;
}

void FileSelectItem::setPlanetColor() {
    killAllModels();
    mPlanetMapObj->makeActorAppeared();
    MR::startBrk(mPlanetMapObj, "FileSelectDataPlanet");
    MR::setBrkFrameAndStop(mPlanetMapObj, static_cast<f32>(mFileIndex - 1));
}

void FileSelectItem::exeFormat() {
    if (MR::isFirstStep(this)) {
    }

    if (MR::isLessStep(this, 40)) {
        MR::startActionSound(this, "SyLvFileSelMorphblur", -1, -1, -1);
    }

    if (MR::isStep(this, 40)) {
        MR::startActionSound(this, "SyFileSelMorphDummy", -1, -1, -1);
        emitVanish();
        setPlanetColor();
        MR::tryRumblePadStrong(this, WPAD_CHAN0);
        MR::shakeCameraNormal();
    }

    if (MR::isGreaterEqualStep(this, 40)) {
        mRotation.y = 0.0f;
        mRotationSpeed = 0.0f;
    }

    if (MR::isStep(this, 60)) {
        setNerve(GET_NERVE(FileSelectItem, FileSelectItemNrvNewWait));
    }
}

void FileSelectItem::exeChangeFellow() {
    if (MR::isFirstStep(this)) {
    }

    if (MR::isLessStep(this, 40)) {
        MR::startActionSound(this, "SyLvFileSelMorphblur", -1, -1, -1);
    }

    if (MR::isStep(this, 40)) {
        MR::startActionSound(this, "SyFileSelMorphMario", -1, -1, -1);
        appearFellowModel();
        emitCompleteEffect();

        if (mUseCopyEffect) {
            emitCopy();
        } else {
            emitOpen();
        }

        MR::tryRumblePadStrong(this, WPAD_CHAN0);
        MR::shakeCameraNormal();
    }

    if (MR::isGreaterEqualStep(this, 40)) {
        mRotation.y = 0.0f;
        mRotationSpeed = 0.0f;
    }

    if (MR::isStep(this, 150)) {
        setNerve(GET_NERVE(FileSelectItem, FileSelectItemNrvExistWait));
    }
}

void FileSelectItem::exeChangeMii() {
    if (MR::isFirstStep(this)) {
    }

    if (MR::isLessStep(this, 40)) {
        MR::startActionSound(this, "SyLvFileSelMorphblur", -1, -1, -1);
    }

    if (MR::isStep(this, 39)) {
        mFaceParts->changeFaceModel(MiiFaceRecipe(RFLDataSource_Official, mIconID->getMiiIndex(), RFLResolution_256, 33));
    }

    if (MR::isStep(this, 40)) {
        MR::startActionSound(this, "SyFileSelMorphMario", -1, -1, -1);
        killAllModels();
        mFaceParts->makeActorAppeared();
        emitCompleteEffect();

        if (mUseCopyEffect) {
            emitCopy();
        } else {
            emitOpen();
        }

        MR::tryRumblePadStrong(this, WPAD_CHAN0);
        MR::shakeCameraNormal();
    }

    if (MR::isGreaterEqualStep(this, 40)) {
        mRotation.y = 0.0f;
        mRotationSpeed = 0.0f;
    }

    if (MR::isStep(this, 150)) {
        setNerve(GET_NERVE(FileSelectItem, FileSelectItemNrvExistWait));
    }
}

// NONMATCHING: scheduling; target loads mScaleCtrl->getScale() before the mPlanetMapObj / mFaceParts pointer (ours hoists the pointer loads)
void FileSelectItem::control() {
    updatePointing();
    updateRotate();

    TPos3f mtx;
    if (mRotation.x == 0.0f && mRotation.z == 0.0f) {
        MR::makeMtxTransRotateY(mtx, this);
    } else {
        MR::makeMtxTR(mtx, this);
    }

    TVec3f yDir;
    mtx.getYDir(yDir);

    TVec3f trans;
    mtx.getTrans(trans);
    mPlanetMtx.set(mtx);
    mPlanetMtx.setTrans(trans + yDir * 30.0f * 30.0f);

    mFellowMtx.set(mtx);
    mMiiMtx.set(mtx);

    mScaleCtrl->updateNerve();

    mPlanetMapObj->mScale.setAll< f32 >(30.0f * mScaleCtrl->getScale());

    for (s32 i = 0; i < 7; i++) {
        mModels[i]->mScale.setAll< f32 >(30.0f * mScaleCtrl->getScale());
    }

    mFaceParts->mScale.setAll< f32 >(27.0f * mScaleCtrl->getScale());

    mBlinkCtrl->updateNerve();

    TVec3f newPos;
    TVec3f screenPos;
    newPos.add(mPosition, ::sDataInfoOffset);
    MR::calcScreenPosition(&screenPos, newPos);
    mNumber->setTrans(screenPos);
}

void FileSelectItem::createNew() {
    mPlanetMapObj = MR::createPartsModelMapObj(this, "ニューフェイス", "FileSelectDataPlanet", mPlanetMtx);
    mPlanetMapObj->mScale.set(30.0f);
    mPlanetMapObj->makeActorDead();
}

void FileSelectItem::createFellows() {
    for (u32 i = 0; i < 7; i++) {
        mModels[i] = new FileSelectModel(::sFellowModel[i], mFellowMtx, "キャラフェイス");
    }
}

void FileSelectItem::createMii() {
    if (mIsNew || !mIconID->isMii()) {
        mFaceParts = MiiFacePartsHolder::createPartsFromDefault("Miiフェイス", 0);
    } else {
        mFaceParts = MiiFacePartsHolder::createPartsFromReceipe("Miiフェイス",
                                                                MiiFaceRecipe(RFLDataSource_Official, mIconID->getMiiIndex(), RFLResolution_256, 33));
    }

    mFaceParts->initFixedPosition(mMiiMtx, TVec3f(0.0f, 0.0f, 0.0f), TVec3f(0.0f, 0.0f, 0.0f));
    mFaceParts->mScale.set(27.0f);
    mFaceParts->initEffectKeeper(0, "FileSelectDataMii", false);
    MR::invalidateClipping(mFaceParts);
    mFaceParts->makeActorDead();
}

void FileSelectItem::createNumber() {
    mNumber = new FileSelectNumber("ファイル番号");
    mNumber->initWithoutIter();
    mNumber->setNumber(mFileIndex);
}

void FileSelectItem::updatePointing() {
    if (mIsInvalidateSelect) {
        if (MR::isStarPointerPointing1PWithoutCheckZ(this, nullptr, false, false)) {
            if (mDelegator != nullptr) {
                mDelegator->notify(this, 2);
            }
        }
    } else if (MR::isStarPointerPointingFileSelect(this)) {
        if (mDelegator != nullptr) {
            mDelegator->notify(this, 0);
        }
    }

    if (!mIsInvalidateSelect && mIsPointed && MR::testDPDMenuPadDecideTrigger()) {
        if (mDelegator != nullptr) {
            mDelegator->notify(this, 1);
            mScaleCtrl->setNerve(GET_NERVE_DIRECT(FileSelectItemSub, ScaleControllerNrvToSmall));
        }
    }
}

namespace {
    struct PointerSweepTriangle {
        TVec3f mVertexA;
        TVec3f mVertexB;
        TVec3f mVertexC;
        TVec3f mEdgeAB;
        TVec3f mEdgeBC;
        TVec3f mEdgeCA;
        TVec3f mNormal;

        void set(const TVec3f& rCameraPosition, const TVec3f& rCurrentPoint, const TVec3f& rPreviousPoint) {
            mVertexA = rCameraPosition;
            mVertexB = rCurrentPoint;
            mVertexC = rPreviousPoint;

            mEdgeAB = rCurrentPoint - rCameraPosition;
            mEdgeBC = rPreviousPoint - rCurrentPoint;
            mEdgeCA = rCameraPosition - rPreviousPoint;
            mNormal.cross(mEdgeBC, mEdgeAB);
            MR::normalize(&mNormal);
        }

        PointerSweepTriangle(const TVec3f& rA, const TVec3f& rB, const TVec3f& rC) {
            set(rA, rB, rC);
        }

        bool contains(const TVec3f& rPoint) const {
            bool containsPoint;
            TVec3f cross;
            cross.cross(rPoint - mVertexA, mEdgeAB);

            if (cross.dot(mNormal) < 0.0f) {
                containsPoint = false;
            } else {
                cross.cross(rPoint - mVertexB, mEdgeBC);

                if (cross.dot(mNormal) < 0.0f) {
                    containsPoint = false;
                } else {
                    cross.cross(rPoint - mVertexC, mEdgeCA);

                    containsPoint = !(cross.dot(mNormal) < 0.0f);
                }
            }

            return containsPoint;
        }

        void project(TVec3f* pResult, const TVec3f& rCenter, f32 distance) const {
            TVec3f offset(mNormal);
            offset.scale(distance);
            *pResult = rCenter - offset;
        }
    };

    inline bool checkCollisionOfPointAndTriangle(const TVec3f& rCenter, const TVec3f& rCameraPosition, const TVec3f& rCurrentPoint,
                                                 const TVec3f& rPreviousPoint) {
        PointerSweepTriangle triangle(rCameraPosition, rCurrentPoint, rPreviousPoint);

        TVec3f delta(rCenter - rCameraPosition);
        f32 planeDistance = triangle.mNormal.dot(delta);

        bool containsPoint;
        bool collides;

        if (MR::abs(planeDistance) >= 900.0f) {
            collides = false;
        } else {
            TVec3f projected;
            triangle.project(&projected, rCenter, planeDistance);
            containsPoint = triangle.contains(projected);
            if (containsPoint) {
                collides = true;
            } else if (triangle.mVertexA.distance(rCenter) <= 900.0f) {
                collides = true;
            } else if (triangle.mVertexB.distance(rCenter) <= 900.0f) {
                collides = true;
            } else if (triangle.mVertexC.distance(rCenter) <= 900.0f) {
                collides = true;
            } else if (::checkCollisionOfPointAndCylinder(rCenter, triangle.mVertexA, triangle.mEdgeAB, 900.0f)) {
                collides = true;
            } else if (::checkCollisionOfPointAndCylinder(rCenter, triangle.mVertexB, triangle.mEdgeBC, 900.0f)) {
                collides = true;
            } else {
                collides = ::checkCollisionOfPointAndCylinder(rCenter, triangle.mVertexC, triangle.mEdgeCA, 900.0f);
            }
        }

        return collides;
    }

};  // namespace

void FileSelectItem::updateRotate() {
    const s32 duration = mTurnToFrontDuration;
    if (duration > 0) {
        mRotationSpeed = 0.0f;

        mTurnToFrontStep++;
        const s32 step = mTurnToFrontStep;
        f32 rate = static_cast< f32 >(step) / duration;
        rate *= rate;
        if (rate > 1.0f) {
            rate = 1.0f;
        }

        if (rate < 0.0f) {
            rate = 0.0f;
        }

        if (step >= duration) {
            mTurnToFrontStep = 0;
            mTurnToFrontDuration = 0;
        }

        f32 angle = MR::repeat(mRotation.y, -180.0f, 360.0f);
        mRotation.y = angle - angle * rate;
        FileSelectItemSub::BlinkController* pBlink = mBlinkCtrl;
        pBlink->open();
        pBlink->setNerve(GET_NERVE_DIRECT(FileSelectItemSub, BlinkControllerNrvOpen));
        return;
    }

    if (mIsInvalidRotate) {
        return;
    }

    if (MR::isStarPointerInScreen(0)) {
        TVec3f center = mPosition + TVec3f(0.0f, 900.0f, 0.0f);
        TVec2f screenPos(*MR::getStarPointerScreenPosition(0));

        if (mNeedsPointerReset) {
            mPreviousPointerPos = screenPos;
            mWasDragging = mIsDragging;
            mNeedsPointerReset = 0;
        }

        TVec3f cameraPosition = MR::getCamPos();
        TVec3f cameraToCenter = center - cameraPosition;

        f32 pointerDistance = (900.0f + cameraToCenter.length());
        if (JGeometry::TUtil< f32 >::sqrt(screenPos.squareDist(mPreviousPointerPos)) < 2.0f) {
            TVec3f pointerPosition;
            MR::calcWorldPositionFromScreen(&pointerPosition, screenPos, pointerDistance);
            TVec3f pointerDirection = pointerPosition - cameraPosition;
            MR::normalize(&pointerDirection);
            TVec3f crossDirection = cameraToCenter.cross(pointerDirection);

            if (crossDirection.length() < 900.0f) {
                mIsDragging = 1;
            }
        } else {
            TVec3f currentPoint;
            MR::calcWorldPositionFromScreen(&currentPoint, screenPos, pointerDistance);
            TVec3f previousPoint;
            MR::calcWorldPositionFromScreen(&previousPoint, mPreviousPointerPos, pointerDistance);
            bool collides = ::checkCollisionOfPointAndTriangle(center, cameraPosition, currentPoint, previousPoint);

            if (collides) {
                mIsDragging = 1;
            }
        }

        if (mIsDragging) {
            TVec2f delta = screenPos - mPreviousPointerPos;
            mRotationSpeed += 0.03f * delta.x / mScale.x;
            mRotationSpeed = MR::clamp(mRotationSpeed, -25.0f, 25.0f);
        }

        mWasDragging = mIsDragging;
        mPreviousPointerPos = screenPos;
    } else {
        mNeedsPointerReset = 1;
    }

    mRotationSpeed *= 0.98f;

    if (mRotationSpeed >= 0.0f && mRotationSpeed < 0.5f) {
        mRotationSpeed = 0.5f;
    }

    if (mRotationSpeed < 0.0f && mRotationSpeed > -0.5f) {
        mRotationSpeed = -0.5f;
    }

    mRotation.y = MR::repeat(mRotation.y + mRotationSpeed, 0.0f, 360.0f);
    mIsDragging = 0;
}

void FileSelectItem::playPointedME() {
    switch (MR::getRandom(0l, 5l)) {
    case 0:
        MR::startSystemME("ME_ASTRO_DOME_HIT_GALAXY1");

        break;
    case 1:
        MR::startSystemME("ME_ASTRO_DOME_HIT_GALAXY2");

        break;
    case 2:
        MR::startSystemME("ME_ASTRO_DOME_HIT_GALAXY3");

        break;
    case 3:
        MR::startSystemME("ME_ASTRO_DOME_HIT_GALAXY4");

        break;
    case 4:
        MR::startSystemME("ME_ASTRO_DOME_HIT_GALAXY5");

        break;
    }
}

void FileSelectItem::playPointedNotUsingME() {
    switch (MR::getRandom(0l, 5l)) {
    case 0:
        MR::startSystemME("ME_ASTRO_DOME_HIT_GALAXY_N1");

        break;
    case 1:
        MR::startSystemME("ME_ASTRO_DOME_HIT_GALAXY_N2");

        break;
    case 2:
        MR::startSystemME("ME_ASTRO_DOME_HIT_GALAXY_N3");

        break;
    case 3:
        MR::startSystemME("ME_ASTRO_DOME_HIT_GALAXY_N4");

        break;
    case 4:
        MR::startSystemME("ME_ASTRO_DOME_HIT_GALAXY_N5");

        break;
    }
}

void FileSelectItem::appearFellowModel() {
    killAllModels();
    mModels[mIconID->getFellowID()]->makeActorAppeared();
}

void FileSelectItem::killAllModels() {
    mPlanetMapObj->makeActorDead();

    for (s32 i = 0; i < 7; i++) {
        mModels[i]->makeActorDead();
    }

    mFaceParts->makeActorDead();
}

void FileSelectItem::emitOpen() {
    if (!MR::isDead(mPlanetMapObj)) {
        MR::emitEffect(mPlanetMapObj, "Open");
    }

    for (s32 i = 0; i < 7; i++) {
        if (!MR::isDead(mModels[i])) {
            mModels[i]->emitOpen();
        }
    }

    if (!MR::isDead(mFaceParts)) {
        MR::emitEffect(mFaceParts, "Open");
    }
}

void FileSelectItem::emitVanish() {
    if (!MR::isDead(mPlanetMapObj)) {
        MR::emitEffect(mPlanetMapObj, "Vanish");
    }

    for (s32 i = 0; i < 7; i++) {
        if (!MR::isDead(mModels[i])) {
            mModels[i]->emitVanish();
        }
    }

    if (!MR::isDead(mFaceParts)) {
        MR::emitEffect(mFaceParts, "Vanish");
    }
}

void FileSelectItem::emitCopy() {
    if (!MR::isDead(mPlanetMapObj)) {
        MR::emitEffect(mPlanetMapObj, "Copy");
    }

    for (s32 i = 0; i < 7; i++) {
        if (!MR::isDead(mModels[i])) {
            mModels[i]->emitCopy();
        }
    }

    if (!MR::isDead(mFaceParts)) {
        MR::emitEffect(mFaceParts, "Copy");
    }
}

void FileSelectItem::emitCompleteEffect() {
    if (!mIsComplete) {
        return;
    }

    for (s32 i = 0; i < 7; i++) {
        if (!MR::isDead(mModels[i])) {
            mModels[i]->emitCompleteEffect();
        }
    }

    if (!MR::isDead(mFaceParts)) {
        MR::emitEffect(mFaceParts, "Complete");
    }
}

void FileSelectItem::deleteCompleteEffect() {
    for (s32 i = 0; i < 7; i++) {
        mModels[i]->deleteCompleteEffect();
    }

    MR::deleteEffect(mFaceParts, "Complete");
}

namespace FileSelectItemSub {

    ScaleController::ScaleController() : NerveExecutor("ファイルセレクタアイコンサイズ管理") {
        mScale = 1.0f;
        initNerve(GET_NERVE_DIRECT(FileSelectItemSub, ScaleControllerNrvSmall));
    }

    void ScaleController::exeToSmall() {
        if (MR::isLessEqualStep(this, 30)) {
            f32 v = (getNerveStep() / 30.0f);
            mScale += (v * (1.0f - mScale));
        }

        MR::setNerveAtStep(this, GET_NERVE_DIRECT(FileSelectItemSub, ScaleControllerNrvSmall), 30);
    }

    void ScaleController::exeToBig() {
        if (MR::isLessEqualStep(this, 30)) {
            f32 v = (getNerveStep() / 30.0f);
            mScale += v * (1.2f - mScale);
        }

        MR::setNerveAtStep(this, GET_NERVE_DIRECT(FileSelectItemSub, ScaleControllerNrvBig), 30);
    }

    BlinkController::BlinkController(FileSelectItem* pItem) : NerveExecutor("ファイルセレクタアイコン瞬き管理") {
        mItem = pItem;
        mBlinkWaitFrames = 0;
        mSleepFrames = 0;
        initNerve(GET_NERVE_DIRECT(FileSelectItemSub, BlinkControllerNrvOpen));
    }

    void BlinkController::exeOpen() {
        if (MR::isFirstStep(this)) {
            mBlinkWaitFrames = MR::getRandom(180l, 300l);
            mSleepFrames = 0;
        }

        f32 v2 = MR::abs(mItem->mRotationSpeed);

        if (v2 > 10.0f) {
            mSleepFrames += 2;
        } else {
            if (v2 > 6.0f) {
                mSleepFrames++;
            } else {
                mSleepFrames = 0;
            }
        }

        if (mSleepFrames > 180) {
            setNerve(GET_NERVE_DIRECT(FileSelectItemSub, BlinkControllerNrvSleep));
        } else {
            if (MR::isGreaterEqualStep(this, mBlinkWaitFrames)) {
                setNerve(GET_NERVE_DIRECT(FileSelectItemSub, BlinkControllerNrvShut));
            }
        }
    }

    void BlinkController::exeShut() {
        if (MR::isFirstStep(this)) {
            shut();
        }

        if (MR::isGreaterEqualStep(this, 10)) {
            open();
            setNerve(GET_NERVE_DIRECT(FileSelectItemSub, BlinkControllerNrvOpen));
        }
    }

    void BlinkController::exeSleep() {
        if (MR::isFirstStep(this)) {
            sleep();
            mSleepFrames = 0;
        }

        mSleepFrames++;

        if (MR::abs(mItem->mRotationSpeed) > 2.0f) {
            mSleepFrames = 0;
        }

        if (mSleepFrames > 60) {
            setNerve(GET_NERVE_DIRECT(FileSelectItemSub, BlinkControllerNrvBlink));
        }
    }

    void BlinkController::exeBlink() {
        if (MR::isFirstStep(this) && mItem->mIconID->isFellow()) {
            mItem->mModels[mItem->mIconID->getFellowID()]->blink();
        }

        if (getNerveStep() % 8 < 4) {
            mItem->mFaceParts->changeExpressionBlink();

        } else {
            mItem->mFaceParts->changeExpressionNormal();
        }

        if (mItem->mIconID->isFellow()) {
            FileSelectIconID::EFellowID id = mItem->mIconID->getFellowID();

            if (mItem->mModels[id]->isOpen()) {
                setNerve(GET_NERVE_DIRECT(FileSelectItemSub, BlinkControllerNrvOpen));
                return;
            }
        }

        if (mItem->mIconID->isMii()) {
            if (MR::isGreaterEqualStep(this, 40)) {
                mItem->mFaceParts->changeExpressionNormal();
                setNerve(GET_NERVE_DIRECT(FileSelectItemSub, BlinkControllerNrvOpen));
            }
        }
    }

    void BlinkController::shut() {
        if (!mItem->mIsNew) {
            if (mItem->mIconID->isMii()) {
                mItem->mFaceParts->changeExpressionBlink();
            } else {
                mItem->mModels[mItem->mIconID->getFellowID()]->blinkOnce();
            }
        }
    }

    void BlinkController::open() {
        if (!mItem->mIsNew) {
            if (mItem->mIconID->isMii()) {
                mItem->mFaceParts->changeExpressionNormal();
            } else {
                mItem->mModels[mItem->mIconID->getFellowID()]->open();
            }
        }
    }

    void BlinkController::sleep() {
        if (!mItem->mIsNew) {
            if (mItem->mIconID->isMii()) {
                mItem->mFaceParts->changeExpressionBlink();
            } else {
                mItem->mModels[mItem->mIconID->getFellowID()]->close();
            }
        }
    }

    void ScaleController::exeBig() {
        mScale = 1.2f;
    }

    void ScaleController::exeSmall() {
        mScale = 1.0f;
    }
};  // namespace FileSelectItemSub

void FileSelectItem::exeExistWait() {
}

void FileSelectItem::exeNewWait() {
}

FileSelectItem::~FileSelectItem() {
}
