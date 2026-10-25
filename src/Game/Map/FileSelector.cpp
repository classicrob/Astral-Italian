#include "Game/System/GameDataFunction.hpp"
#ifdef GALAXY_NATIVE
#include "Game/System/NativeCompleteSave.hpp"
#endif
#include "Game/Map/FileSelector.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/FileSelectCameraController.hpp"
#include "Game/Player/MarioAccess.hpp"
#include "Game/Map/FileSelectEffect.hpp"
#include "Game/Map/FileSelectFunc.hpp"
#include "Game/Map/FileSelectItem.hpp"
#include "Game/Map/FileSelectItemDelegator.hpp"
#include "Game/Map/FileSelectSky.hpp"
#include "Game/NPC/MiiFacePartsHolder.hpp"
#include "Game/NameObj/NameObjFactory.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Screen/BackButton.hpp"
#include "Game/Screen/BrosButton.hpp"
#include "Game/Screen/FileSelectButton.hpp"
#include "Game/Screen/FileSelectInfo.hpp"
#include "Game/Screen/InformationMessage.hpp"
#include "Game/Screen/Manual2P.hpp"
#include "Game/Screen/MiiConfirmIcon.hpp"
#include "Game/Screen/MiiSelect.hpp"
#include "Game/Screen/SysInfoWindow.hpp"
#include "Game/Screen/TitleSequenceProduct.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/UserFile.hpp"
#include "Game/Util/ActorSensorUtil.hpp"

#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/FileUtil.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MessageUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/SequenceUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <JSystem/JKernel/JKRMemArchive.hpp>
#include <cstdio>
#include <nw4r/ut/ResFont.h>
#ifdef GALAXY_NATIVE
#include <petari/launch_stage.hpp>
#include <petari/player_launch.hpp>
#include <petari/milestone.hpp>
#endif

#define USER_FILE_NUM 3

#ifdef GALAXY_NATIVE
namespace {
    // A pending --stage/--request startup opens file 1 once, creating it (Mario icon, no Mii) if it is new.
    bool sNativeStartupFileUsed = false;
    bool sNativeStartupCreate = false;
}  // namespace
#endif

namespace {
    static const s32 sIndexOrder[6] = {1, 2, 3, 4, 5, 6};
    const char* cMarioNameMessageID = "System_FileSelect_Icon000";
};  // namespace

namespace {
    inline int getItemArrayIndex(s32 id) {
        const s32* pIndexOrder = sIndexOrder;

        for (int i = 0; i < USER_FILE_NUM; i++) {
            if (id == pIndexOrder[i]) {
                return i;
            }
        }

        return -1;
    }

    MiiFacePartsHolder* getMiiFacePartsHolder() {
        return MR::getSceneObj< MiiFacePartsHolder >(SceneObj_MiiFacePartsHolder);
    }
};  // namespace

namespace NrvFileSelector {
    NEW_NERVE(FileSelectorNrvTitle, FileSelector, Title);
    NEW_NERVE(FileSelectorNrvTitleEnd, FileSelector, TitleEnd);
    NEW_NERVE(FileSelectorNrvRFLError, FileSelector, RFLError);
    NEW_NERVE(FileSelectorNrvRFLWait, FileSelector, RFLWait);
    NEW_NERVE(FileSelectorNrvRFLWaitEnd, FileSelector, RFLWaitEnd);
    NEW_NERVE(FileSelectorNrvFileSelectStart, FileSelector, FileSelectStart);
    NEW_NERVE(FileSelectorNrvFileSelect, FileSelector, FileSelect);
    NEW_NERVE(FileSelectorNrvFileConfirmStart, FileSelector, FileConfirmStart);
    NEW_NERVE(FileSelectorNrvFileConfirmMiiDeleteWarningStart, FileSelector, FileConfirmMiiDeleteWarningStart);
    NEW_NERVE(FileSelectorNrvFileConfirmMiiDeleteWarning, FileSelector, FileConfirmMiiDeleteWarning);
    NEW_NERVE(FileSelectorNrvFileConfirmMiiDeleteSave, FileSelector, FileConfirmMiiDeleteSave);
    NEW_NERVE(FileSelectorNrvFileConfirm, FileSelector, FileConfirm);
    NEW_NERVE(FileSelectorNrvDemoStartWait, FileSelector, DemoStartWait);
    NEW_NERVE(FileSelectorNrvDemo, FileSelector, Demo);
    NEW_NERVE(FileSelectorNrvCreateConfirmStart, FileSelector, CreateConfirmStart);
    NEW_NERVE(FileSelectorNrvCreateConfirm, FileSelector, CreateConfirm);
    NEW_NERVE(FileSelectorNrvCreate, FileSelector, Create);
    NEW_NERVE(FileSelectorNrvMiiTip, FileSelector, MiiTip);
    NEW_NERVE(FileSelectorNrvMiiSelectStartFirst, FileSelector, MiiSelectStart);
    NEW_NERVE(FileSelectorNrvMiiSelectStart, FileSelector, MiiSelectStart);
    NEW_NERVE(FileSelectorNrvMiiSelect, FileSelector, MiiSelect);
    NEW_NERVE(FileSelectorNrvMiiCancel, FileSelector, MiiCancel);
    NEW_NERVE(FileSelectorNrvMiiConfirmWait, FileSelector, MiiConfirmWait);
    NEW_NERVE(FileSelectorNrvMiiConfirm, FileSelector, MiiConfirm);
    NEW_NERVE(FileSelectorNrvMiiCreateWait, FileSelector, MiiCreateWait);
    NEW_NERVE(FileSelectorNrvMiiCreateDemo, FileSelector, MiiCreateDemo);
    NEW_NERVE(FileSelectorNrvMiiCaution, FileSelector, MiiCaution);
    NEW_NERVE(FileSelectorNrvMiiInfoStart, FileSelector, MiiInfoStart);
    NEW_NERVE(FileSelectorNrvMiiInfo, FileSelector, MiiInfo);
    NEW_NERVE(FileSelectorNrvDeleteConfirmStart, FileSelector, DeleteConfirmStart);
    NEW_NERVE(FileSelectorNrvDeleteConfirm, FileSelector, DeleteConfirm);
    NEW_NERVE(FileSelectorNrvDelete, FileSelector, Delete);
    NEW_NERVE(FileSelectorNrvDeleteDemo, FileSelector, DeleteDemo);
    NEW_NERVE(FileSelectorNrvManualStart, FileSelector, ManualStart);
    NEW_NERVE(FileSelectorNrvManual, FileSelector, Manual);
};  // namespace NrvFileSelector


FileSelector::FileSelector(const char* pName)
    : LiveActor(pName), mCameraController(), mSky(), mItems(), mOperationButton(), mBackButton(), mSysInfoWindow(), mSelectedItem(), mUnusedItem(), mPointedItem(), mPointingCandidate(),
      mUserFile(), mCompleteFlags(), mTitle(), mMiiSelect(), mMiiId(new RFLCreateID()), mManual(), mIsMiiSelectStartFirst(), mBgmState(), mSelectEffect(),
      mIsChildMovementStopped(), mChildActors(), mChildActorCount(), mIsTitleBgmRhythmEnabled(), mTitleBgmRhythmVolume(), mTargetTitleBgmRhythmVolume(), mTitleBgmRhythmFadeFrames() {
}

// NONMATCHING: the two TVec3f temporaries (sensor offset / sound offset) get swapped stack slots
namespace {
    inline void initFileSelectorBodySensor(LiveActor* actor) {
        MR::addHitSensorPriorBinder(actor, "Body", 8, 500.0f, TVec3f(0.0f, 0.0f, 0.0f));
    }
}

void FileSelector::init(const JMapInfoIter& rIter) {
    MR::connectToScene(this, 0x20, -1, -1, -1);
    initHitSensor(1);
    initFileSelectorBodySensor(this);
    initSound(4, "FileSelector", nullptr, TVec3f(0.0f));
    initUserFileArray();
    MR::invalidateClipping(this);
    createCameraController();
    createFileItems();
    createOperationButton();
    createBackButton();
    createSysInfoWindow();
    createFileInfo();
    createTitle();
    createMiiSelect();
    createMiiConfirmIcon();
    createManual();
    createSelectEffect();
    initNerve(GET_NERVE(FileSelector, FileSelectorNrvTitle));
    MR::tryRegisterDemoCast(this, rIter);
    initChildObjs(rIter);
    MR::initUseStageSwitchWriteA(this, rIter);
    mIsTitleBgmRhythmEnabled = false;
    appear();
}

void FileSelector::appear() {
    LiveActor::appear();
    MarioAccess::readyRemoteDemo();
    MR::hidePlayer();
    setNerve(GET_NERVE(FileSelector, FileSelectorNrvTitle));
}

void FileSelector::kill() {
    mBackButton->kill();
    LiveActor::kill();
    MarioAccess::fn_803B6830(false);
}

void FileSelector::callbackStart() {
    setNerve(GET_NERVE(FileSelector, FileSelectorNrvDemoStartWait));
}

void FileSelector::callbackMii() {
    disappearAllLayout();
    invalidateSelectAll();
    setNerve(GET_NERVE(FileSelector, FileSelectorNrvMiiTip));
}

void FileSelector::callbackDelete() {
    setNerve(GET_NERVE(FileSelector, FileSelectorNrvDeleteConfirmStart));
}

void FileSelector::callbackManual() {
    setNerve(GET_NERVE(FileSelector, FileSelectorNrvManualStart));
}

void FileSelector::notifyItem(FileSelectItem* pItem, s32 msg) {
    switch (msg) {
    case 0:
        onPoint(pItem);
        break;
    case 1:
        onSelect(pItem);
        break;
    default:
        break;
    }
}

// NONMATCHING: register allocation (i/offset/temp pointer registers swapped in the first loop)
// NONMATCHING: stack slot/register allocation (named offset/actor locals in the second loop shift the frame; target uses unnamed temps)
void FileSelector::control() {
    mPointingCandidate = nullptr;

    for (int i = 0; i < mItems->getObjNum(); i++) {
        LiveActor* pActor = mItems->getActor(i);
        TVec3f& rPosition = pActor->mPosition;
        rPosition = mItemPositions[i] * 0.05f + rPosition * 0.95f;
    }

    for (int i = 0; i < mItems->getObjNum(); i++) {
        TVec3f offset(0.0f, 1000.0f, 0.0f);
        LiveActor* pActor = mItems->getActor(i);
        MR::setPosition(&mSelectEffect[i], pActor->mPosition + offset);
    }

    updateBgm();
}

bool FileSelector::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    return false;
}

void FileSelector::initUserFileArray() {
    mUserFile = new UserFile[USER_FILE_NUM]();
    mCompleteFlags = new bool[USER_FILE_NUM];

    initUserFile();
}

void FileSelector::createCameraController() {
    mCameraController = new FileSelectCameraController("ファイルセレクトカメラ制御");
    mCameraController->initWithoutIter();
}

// The target retains the sky name after removal of the sky creation path.
namespace FileSelectorData {
    char sSkyActorName[] = "ファイルセレクト画面の空";
}

void FileSelector::createFileItems() {
    mItems = new DeriveActorGroup< FileSelectItem >("全ファイルアイテム保持", USER_FILE_NUM);
    mItemPositions = new TVec3f[USER_FILE_NUM]();

    calcBasePos(0.0f);
    MR::createSceneObj(SceneObj_MiiFacePartsHolder);

    FileSelectItemDelegator< FileSelector >* pDelegator = new FileSelectItemDelegator< FileSelector >(this, &FileSelector::notifyItem);

    for (int i = 0; i < USER_FILE_NUM; i++) {
        FileSelectItem* pItem = createFileItem(::sIndexOrder[i]);

        pItem->setSelectDelegator(pDelegator);
        MR::setPosition(pItem, mItemPositions[i]);
        mItems->registerActor(pItem);
    }
}

void FileSelector::createOperationButton() {
    mOperationButton = new FileSelectButton("FileSelectButton");
    mOperationButton->initWithoutIter();
    mOperationButton->setCallbackFunctor(MR::Functor(this, &FileSelector::callbackStart), MR::Functor(this, &FileSelector::callbackMii),
                                         MR::Functor(this, &FileSelector::callbackDelete), MR::Functor(this, &FileSelector::callbackManual));
}

void FileSelector::createBackButton() {
    mBackButton = new BackButtonCancelB("戻るボタン", false);
    mBackButton->initWithoutIter();
    mBackButton->waitForSelection();
    MR::connectToScene(mBackButton, 0xE, 0xD, -1, 0x51);
}

void FileSelector::createSysInfoWindow() {
    mSysInfoWindow = MR::createSysInfoWindow();
    MR::connectToSceneLayoutTemporarily(mSysInfoWindow);
    mSysInfoWindowMini = MR::createSysInfoWindowMiniExecuteWithChildren();
    MR::connectToSceneLayoutTemporarily(mSysInfoWindowMini);
}

void FileSelector::createFileInfo() {
    mFileInfo = new FileSelectInfo(12, "ファイル情報");
    mFileInfo->initWithoutIter();
}

void FileSelector::createTitle() {
    mTitle = new TitleSequenceProduct();
    mTitle->kill();
}

void FileSelector::createMiiSelect() {
    mMiiSelect = new MiiSelect("MiiSelect");
    mMiiSelect->initWithoutIter();
}

void FileSelector::createMiiConfirmIcon() {
    mMiiConfirmIcon = new MiiConfirmIcon("Mii確認用アイコン");
    mMiiConfirmIcon->initWithoutIter();
    MR::connectToScene(mMiiConfirmIcon, 0xE, 0xD, -1, 0x51);
}

void FileSelector::createManual() {
    mManual = new Manual2P("２Ｐマニュアル");
    mManual->initWithoutIter();
}

void FileSelector::createSelectEffect() {
    mSelectEffect = new FileSelectEffect[USER_FILE_NUM];

    for (int i = 0; i < USER_FILE_NUM; i++) {
        mSelectEffect[i].initWithoutIter();
        MR::setPosition(&mSelectEffect[i], mItemPositions[i]);
        MR::setScale(&mSelectEffect[i], 0.4f);
    }
}

void FileSelector::appearAllItems() {
    for (int i = 0; i < mItems->getObjNum(); i++) {
        mItems->getActor(i)->appear();
    }
}

void FileSelector::disappearAllLayout() {
    if (!MR::isDead(mOperationButton)) {
        mOperationButton->disappear();

        if (!MR::isDead(mFileInfo)) {
            mFileInfo->slideBack();
        }
    }

    if (!MR::isDead(mFileInfo)) {
        mFileInfo->disappear();
    }

    if (!mBackButton->isHidden()) {
        mBackButton->disappear();
    }
}

bool FileSelector::isHiddenAllLayout() const {
    return MR::isDead(mOperationButton) && MR::isDead(mFileInfo) && mBackButton->isHidden();
}

void FileSelector::updateFileInfo() {
    if (mPointingCandidate != nullptr) {
        if (mPointedItem != mPointingCandidate) {
            if (mPointedItem != nullptr) {
                clearPointing();
                mFileInfo->disappear();
            }

            if (mPointingCandidate->isExist()) {
                setFileInfo(mPointingCandidate->mFileIndex);
                mFileInfo->appear();
            }

            mPointedItem = mPointingCandidate;
            mPointedItem->onPointing();
        }
    } else if (mPointedItem != nullptr) {
        clearPointing();
        mFileInfo->disappear();
    }
}

void FileSelector::appearAllIndex() {
    for (int i = 0; i < mItems->getObjNum(); i++) {
        static_cast< FileSelectItem* >(mItems->getActor(i))->appearIndex();
    }
}

void FileSelector::disappearAllIndex() {
    for (int i = 0; i < mItems->getObjNum(); i++) {
        static_cast< FileSelectItem* >(mItems->getActor(i))->disappearIndex();
    }
}

void FileSelector::invalidateSelectAll() {
    for (int i = 0; i < mItems->getObjNum(); i++) {
        static_cast< FileSelectItem* >(mItems->getActor(i))->invalidateSelect();
    }
}

void FileSelector::validateSelectAll() {
    for (int i = 0; i < mItems->getObjNum(); i++) {
        static_cast< FileSelectItem* >(mItems->getActor(i))->validateSelect();
    }
}

void FileSelector::initUserFile() {
    for (int i = 0; i < USER_FILE_NUM; i++) {
        GameSequenceFunction::restoreUserFile(&mUserFile[i], i + 1);
    }

    checkAllComplete();
}

void FileSelector::restoreUserFile() {
    for (int i = 0; i < USER_FILE_NUM; i++) {
        GameSequenceFunction::restoreUserFile(&mUserFile[i], i + 1);
    }

    checkAllComplete();
}

void FileSelector::checkAllComplete() {
    for (int i = 0; i < USER_FILE_NUM; i++) {
        mCompleteFlags[i] = mUserFile[i].isComplete();
    }
}

void FileSelector::onPoint(FileSelectItem* pItem) {
    if (mBackButton->isHidden() || !mBackButton->isPointing()) {
        if (mPointingCandidate != nullptr) {
            if (mPointingCandidate->mFileIndex > pItem->mFileIndex) {
                mPointingCandidate = pItem;
            }
        } else {
            mPointingCandidate = pItem;
        }
    }
}

void FileSelector::onSelect(FileSelectItem* pItem) {
    if (isNerve(GET_NERVE(FileSelector, FileSelectorNrvFileSelect))) {
        if (pItem->isNew()) {
            setNerve(GET_NERVE(FileSelector, FileSelectorNrvCreateConfirmStart));
        } else {
            setNerve(GET_NERVE(FileSelector, FileSelectorNrvFileConfirmStart));
        }
    } else {
        return;
    }

    mSelectedItem = pItem;

    MR::startActionSound(this, "SyGalaxySelected", -1, -1, -1);
    playSelectedME();
}

void FileSelector::clearPointing() {
    if (mPointedItem == nullptr) {
        return;
    }

    mPointedItem->offPointing();
    mPointedItem = nullptr;
}

namespace {
    inline OSTime getUserFileLastModified(const FileSelector* pSelector, s32 id) {
        return pSelector->mUserFile[id - 1].getLastModified();
    }

    inline bool isUserFileViewCompleteEnding(const FileSelector* pSelector, s32 id) {
        return pSelector->mUserFile[id - 1].isViewCompleteEnding();
    }

    inline bool isUserFileViewNormalEnding(const FileSelector* pSelector, s32 id) {
        return pSelector->mUserFile[id - 1].isViewNormalEnding();
    }

    inline s32 getUserFileStarPieceNum(const FileSelector* pSelector, s32 id) {
        return pSelector->mUserFile[id - 1].getStarPieceNum();
    }

    inline s32 getUserFilePowerStarNum(const FileSelector* pSelector, s32 id) {
        return pSelector->mUserFile[id - 1].getPowerStarNum();
    }

    inline void copyIconMiiName(const FileSelector* pSelector, u16* pName, s32 id) {
        FileSelectIconID iconId = FileSelectIconID();

        pSelector->getIconId(&iconId, id);
        FileSelectFunc::copyMiiName(pName, iconId);
    }
};  // namespace

void FileSelector::setFileInfo(s32 id) {
    OSCalendarTime td;
    u16 nameBuffer[12];
    wchar_t dateBuffer[32];
    wchar_t timeBuffer[32];
    wchar_t playTimeBuffer[32];

    if (static_cast< FileSelectItem* >(mItems->getActor(::getItemArrayIndex(id)))->mHasInvalidMiiId) {
        FileSelectFunc::copyMiiName(nameBuffer, FileSelectIconID());
    } else {
        ::copyIconMiiName(this, nameBuffer, id);
    }

    OSTicksToCalendarTime(::getUserFileLastModified(this, id), &td);

    MR::makeDateString(dateBuffer, ARRAY_SIZE(dateBuffer), td.year, td.mon + 1, td.mday);
    MR::makeTimeString(timeBuffer, ARRAY_SIZE(timeBuffer), td.hour, td.min);
    MR::makePlayTimeString(playTimeBuffer, ARRAY_SIZE(playTimeBuffer), mUserFile[id - 1].getCumulativePlayTime());

    FileSelectFileInfo info;
    info.mName = nameBuffer;
    info.mFileId = id;
    info.mPowerStarNum = mUserFile[id - 1].getPowerStarNum();
    info.mGrandStarNum = getGrandStarNum(id);
    info.mHas120Stars = mUserFile[id - 1].calcNormalPowerStarNum() >= 120;
    info.mIsViewed120StarsEnding = mUserFile[id - 1].isView120StarsEnding();
    info.mIsComplete = isUserFileComplete(id);
    info.mHas240Stars = mUserFile[id - 1].getPowerStarNum() >= 242;
    info.mDate = dateBuffer;
    info.mTime = timeBuffer;
    info.mMissCount = getMissCount(id);
    info.mPlayTime = playTimeBuffer;
    mFileInfo->setInfo(info);
}

bool FileSelector::checkSelectedBackButton() {
    if (mBackButton->isHidden()) {
        return false;
    }

    return mBackButton->isSelected();
}

void FileSelector::goToNearPoint() {
    calcBasePos(-16000.0f);
    mCameraController->goToNearPoint(mItemPositions[::getItemArrayIndex(mSelectedItem->mFileIndex)]);
}

// NONMATCHING: float constant registers f30/f31 swapped
void FileSelector::calcBasePos(f32 offset) {
    TVec3f pos;
    pos.x = -3000.0f;
    pos.y = offset;
    pos.z = 0.0f;

    for (int i = 0; i < USER_FILE_NUM; i++) {
        if (mSelectedItem == nullptr || mSelectedItem != mItems->getActor(i)) {
            mItemPositions[i].set(pos);
        }

        TVec3f step;
        step.x = 3000.0f;
        step.y = 0.0f;
        step.z = 0.0f;
        pos.add(step);
    }
}

FileSelectItem* FileSelector::createFileItem(s32 id) {
    FileSelectIconID iconId = FileSelectIconID();
    char* pName = new char[0x40];
    snprintf(pName, 0x40, "ファイルセレクトアイテム[%d]", id);
    FileSelectItem* pItem = new FileSelectItem(id, true, iconId, pName);
    pItem->initWithoutIter();
    return pItem;
}

namespace {
    inline void initFileItem(FileSelector* pSelector, FileSelectItem* pItem) {
        s32 id = pItem->mFileIndex;

        if (!pSelector->mUserFile[id - 1].isCreated()) {
            return;
        }

        u32 tempIconId;
        RFLCreateID tempMiiId;
        FileSelectIconID iconId = FileSelectIconID();

        if (pSelector->mUserFile[id - 1].getIconId(&tempIconId)) {
            iconId.setFellowID(pSelector->getUserFileFellowID(id));
        } else if (pSelector->mUserFile[id - 1].getMiiId(&tempMiiId)) {
            if (::getMiiFacePartsHolder()->isError()) {
                pItem->mHasInvalidMiiId = true;
            } else if (pSelector->isUserFileMiiIdValid(id)) {
                u32 miiIndex = pSelector->getUserFileMiiIndex(id);

                iconId.setMiiIndex(miiIndex);
            } else {
                pItem->mHasInvalidMiiId = true;
            }
        }

        pItem->forceChange(iconId, pSelector->mCompleteFlags[id - 1]);
    }
};  // namespace

void FileSelector::initAllItems() {
    for (int i = 0; i < USER_FILE_NUM; i++) {
        ::initFileItem(this, static_cast< FileSelectItem* >(mItems->getActor(i)));
    }
}

void FileSelector::validateRotateAllItems() {
    for (int i = 0; i < mItems->getObjNum(); i++) {
        static_cast< FileSelectItem* >(mItems->getActor(i))->validateRotate();
    }
}

void FileSelector::stopMovementForLayout() {
    if (!mIsChildMovementStopped) {
        MR::incrementPauseBlurDelay();

        if (mSky != nullptr) {
            MR::requestMovementOff(mSky);
        }

        if (mSelectedItem != nullptr) {
            MR::requestMovementOff(mSelectedItem);
        }

        mIsChildMovementStopped = true;
    }
}

void FileSelector::restartMovementForLayout() {
    if (mIsChildMovementStopped) {
        MR::decrementPauseBlurDelay();

        if (mSky != nullptr) {
            MR::requestMovementOn(mSky);
        }

        if (mSelectedItem != nullptr) {
            MR::requestMovementOn(mSelectedItem);
        }

        mIsChildMovementStopped = false;
    }
}

void FileSelector::initChildObjs(const JMapInfoIter& rIter) {
    mChildActorCount = MR::getChildObjNum(rIter);
    mChildActors = new LiveActor*[mChildActorCount];

    for (int i = 0; i < mChildActorCount; i++) {
        mChildActors[i] = static_cast< LiveActor* >(NameObjFactory::initChildObj(rIter, i));
    }
}

// NONMATCHING: register allocation (loop counter / actor pointer registers swapped)
void FileSelector::updateChildObjs() {
    f32 degToRad;
    f32 radToDeg;
    LiveActor* pActor;

    degToRad = PI / 180.0f;
    radToDeg = 180.0f / PI;

    TPos3f mtxA;
    mtxA.identity();
    mtxA.setRotate(35.0f * degToRad, 0.0f, 345.0f * degToRad);
    mtxA.invert(mtxA);

    TPos3f mtxB;
    mtxB.identity();
    mtxB.setRotate(335.0f * degToRad, 0.0f, 0.0f);
    mtxB.concat(mtxA);

    for (int i = 0; i < mChildActorCount; i++) {
        TPos3f mtx;
        mtx.identity();

        TVec3f rot(mChildActors[i]->mRotation);
        rot.scale(degToRad);
        mtx.setRotate(rot.x, rot.y, rot.z);
        mtx.concat(mtxB, mtx);

        pActor = mChildActors[i];
        mtx.getEulerXYZ(pActor->mRotation);
        mChildActors[i]->mRotation.scale(radToDeg);
    }
}

FileSelectIconID::EFellowID FileSelector::getUserFileFellowID(s32 id) const {
    u32 iconId;

    if (mUserFile[id - 1].getIconId(&iconId)) {
        FileSelectIconID::EFellowID fellowId = FileSelectIconID::Mario;

        if (iconId <= 7) {
            fellowId = static_cast< FileSelectIconID::EFellowID >(iconId - 1);
        }

        return fellowId;
    }

    return FileSelectIconID::Mario;
}

bool FileSelector::isUserFileMiiIdValid(s32 id) const {
    RFLCreateID createId;
    u16 index;

    if (mUserFile[id - 1].getMiiId(&createId)) {
        if (RFLSearchOfficialData(&createId, &index) == TRUE) {
            return RFLIsAvailableOfficialData(index) == TRUE;
        }
    }

    return false;
}

u16 FileSelector::getUserFileMiiIndex(s32 id) const {
    RFLCreateID createId;
    u16 index;

    if (mUserFile[id - 1].getMiiId(&createId)) {
        if (RFLSearchOfficialData(&createId, &index)) {
            return index;
        }
    }

    return 0;
}





// NONMATCHING: register allocation (result flag / index offset registers swapped)
bool FileSelector::isUserFileComplete(s32 id) const {
    return mUserFile[id - 1].calcNormalPowerStarNum() >= 120 && mUserFile[id - 1].getPowerStarNum() >= 240;
}

void FileSelector::storeSetMiiIdUserFile(s32 id, const FileSelectIconID& rIconId) {
    if (rIconId.isFellow()) {
        u32 fellowId = rIconId.getFellowID() + 1;

        GameSequenceFunction::storeMiiOrIconIdUserFileSequence(id, nullptr, &fellowId);
    } else {
        getMiiId(mMiiId, rIconId);
        GameSequenceFunction::storeMiiOrIconIdUserFileSequence(id, mMiiId, nullptr);
    }

    GameSequenceFunction::startSaveAllUserFileSequence();
}

void FileSelector::getMiiId(RFLCreateID* pCreateId, const FileSelectIconID& rIconId) const {
    RFLAdditionalInfo additionalInfo;
    u16 index = rIconId.getMiiIndex();

    if (RFLGetAdditionalInfoWithSanitizedNames(&additionalInfo, RFLDataSource_Official, nullptr, index) == RFLErrcode_Success) {
        *pCreateId = additionalInfo.createID;
    }
}

void FileSelector::getIconId(FileSelectIconID* pIconId, s32 id) const {
    u32 iconId;
    RFLCreateID miiId;

    if (mUserFile[id - 1].getIconId(&iconId)) {
        FileSelectIconID::EFellowID fellowId = getUserFileFellowID(id);

        pIconId->setFellowID(fellowId);
    } else if (mUserFile[id - 1].getMiiId(&miiId) && isUserFileMiiIdValid(id)) {
        u16 miiIndex = getUserFileMiiIndex(id);

        pIconId->setMiiIndex(miiIndex);
    } else {
        pIconId->setFellowID(FileSelectIconID::Mario);
    }
}

s32 FileSelector::getMissCount(s32 id) const {
    if (!mCompleteFlags[id - 1]) {
        return -1;
    }

    return mUserFile[id - 1].getPlayerMissNum();
}

s32 FileSelector::getGrandStarNum(s32 id) const {
    for (s32 i = 6; i > 0; i--) {
        if (mUserFile[id - 1].hasGrandStar(i)) {
            return i;
        }
    }

    return 0;
}

bool FileSelector::isAllOtherFileNew(FileSelectItem* pItem) {
    for (int i = 0; i < USER_FILE_NUM; i++) {
        FileSelectItem* pOther = static_cast< FileSelectItem* >(mItems->getActor(i));

        if (pOther != pItem && mUserFile[pOther->mFileIndex - 1].isCreated()) {
            return false;
        }
    }

    GameDataFunction::resetSysConfigCounters();
    return true;
}

void FileSelector::playSelectedME() {
    switch (MR::getRandom(0L, 4L)) {
    case 0:
        MR::startSystemME("ME_ASTRO_DOME_SELECT1");
        break;
    case 1:
        MR::startSystemME("ME_ASTRO_DOME_SELECT2");
        break;
    case 2:
        MR::startSystemME("ME_ASTRO_DOME_SELECT3");
        break;
    case 3:
        MR::startSystemME("ME_ASTRO_DOME_SELECT4");
        break;
    default:
        break;
    }
}

// The target also retains the five unused cancelled-selection music names.
namespace FileSelectorData {
    char sCancelME1[] = "ME_ASTRO_DOME_CALCEL1";
    char sCancelME2[] = "ME_ASTRO_DOME_CALCEL2";
    char sCancelME3[] = "ME_ASTRO_DOME_CALCEL3";
    char sCancelME4[] = "ME_ASTRO_DOME_CALCEL4";
    char sCancelME5[] = "ME_ASTRO_DOME_CALCEL5";
}

// NONMATCHING: register allocation in the smoothing step
void FileSelector::updateBgm() {
    if (mIsTitleBgmRhythmEnabled) {
        if (mCameraController != nullptr) {
            s32 state = 0;

            if (mCameraController->isToOrAtFarPoint()) {
                mTargetTitleBgmRhythmVolume = 0;
                state = 0;
            } else if (mCameraController->isToOrAtNearPoint()) {
                mTargetTitleBgmRhythmVolume = 100;
                state = 1;
            }

            if (mBgmState != state) {
                mTitleBgmRhythmFadeFrames = 60;
                mBgmState = state;
            }

            s32 target;
            s32 volume;
            s32 frame = mTitleBgmRhythmFadeFrames;
            if (frame > 0) {
                volume = mTitleBgmRhythmVolume;
                target = mTargetTitleBgmRhythmVolume;
                mTitleBgmRhythmFadeFrames = frame - 1;
                mTitleBgmRhythmVolume = volume + (target - volume) / frame;
            }

            MR::startSystemLevelSE("SE_SY_LV_TITLE_BGM", -1, -1);
            MR::startSystemLevelSE("SE_SY_LV_TITLE_BGM_RHYTHM", mTitleBgmRhythmVolume, -1);
        }
    }
}

// NONMATCHING: register allocation (loop counter / actor registers swapped in the second loop)
void FileSelector::updatePointingGuidance() {
    TVec2f screenPos;
    TVec2f pos;
    TVec2f size;

    if (StarPointerUtil::beginMovePositionRegistration(this, true)) {
        for (int i = 0; i < mItems->getObjNum(); i++) {
            LiveActor* pActor = mItems->getActor(i);
            size.x = 0.0f;
            size.y = 0.0f;
            pos.x = 0.0f;
            pos.y = 0.0f;
            StarPointerUtil::addStarPointerMovePosition(pActor->mName, &pos, size);
        }

        LiveActor* pActor;
        for (int i = 0; i < mItems->getObjNum() - 1; i++) {
            pActor = mItems->getActor(i);
            StarPointerUtil::setConnectionMovePositionRight2Way(pActor->mName, mItems->getActor(i + 1)->mName);
        }

        LiveActor* pFirst = mItems->getActor(0);
        LiveActor* pLast = mItems->getActor(mItems->getObjNum() - 1);
        StarPointerUtil::setConnectionMovePositionRight2Way(pLast->mName, pFirst->mName);
        StarPointerUtil::setFirstMovePosition(pFirst->mName);
        StarPointerUtil::setLastMovePosition(pLast->mName);
    }

    for (int i = 0; i < mItems->getObjNum(); i++) {
        LiveActor* pActor = mItems->getActor(i);
        MR::calcScreenPosition(&screenPos, pActor->mPosition);
        StarPointerUtil::setStarPointerMovePosition(pActor->mName, &screenPos);
    }

    StarPointerUtil::endMovePositionRegistration(this);
}

void FileSelector::updatePointingGuidanceSelect() {
    if (StarPointerUtil::beginMovePositionRegistration(this, true)) {
        StarPointerUtil::addStarPointerMovePositionFromPane(mBackButton, "BoxButton", StarPointerUtil::getDefaultButtonOffsetVec2());
        StarPointerUtil::addStarPointerMovePositionFromPane(mOperationButton, "PicMii", StarPointerUtil::getDefaultButtonOffsetVec2());
        StarPointerUtil::addStarPointerMovePositionFromPane(mOperationButton, "PicDelete", StarPointerUtil::getDefaultButtonOffsetVec2());
        StarPointerUtil::addStarPointerMovePositionFromPane(mOperationButton, "BoxStartButton", StarPointerUtil::getDefaultButtonOffsetVec2());
        StarPointerUtil::setConnectionMovePositionDown("PicMii", "BoxButton");
        StarPointerUtil::setConnectionMovePositionDown("PicDelete", "BoxStartButton");
        StarPointerUtil::setConnectionMovePositionRight("PicMii", "PicDelete");
        StarPointerUtil::setConnectionMovePositionRight("BoxButton", "BoxStartButton");
        StarPointerUtil::setFirstAndLastMovePosition("BoxButton", "BoxStartButton");
    }

    StarPointerUtil::refreshMovePositionsFromLayout(mBackButton);
    StarPointerUtil::refreshMovePositionsFromLayout(mOperationButton);
    StarPointerUtil::endMovePositionRegistration(this);
}

#ifdef GALAXY_NATIVE
bool FileSelector::isUserFileCorrupted(s32 id) const {
    return mUserFile[id - 1].mIsGameDataCorrupted || mUserFile[id - 1].mIsConfigDataCorrupted;
}
#endif

void FileSelector::exeTitle() {
    if (MR::isFirstStep(this)) {
#ifdef GALAXY_NATIVE
        petari_milestone("FileSelector.Title");
#endif
        mTitle->appear();
        MR::deactivateDefaultGameLayout();
        MR::startStarPointerModeTitle(this);
        MR::resetCameraMan();
        MR::requestLoadStationedArchiveOthers();
        mIsTitleBgmRhythmEnabled = false;
    }

#ifdef GALAXY_NATIVE
    // An opened file or a pending --stage startup skips the title ("Press A and B").
    if (PetariNative::PlayerLaunch::openFile > 0 ||
        (!sNativeStartupFileUsed && PetariNative::App::LaunchStage::startupStatus() == PetariNative::App::LaunchStage::StartupStatus::PendingFileLoad)) {
        mTitle->kill();
    } else {
        mTitle->updateNerve();
    }
#else
    mTitle->updateNerve();
#endif

    if (mTitle->isBgmStartStep()) {
        mIsTitleBgmRhythmEnabled = true;
    }

    if (!mTitle->isActive()) {
        mCameraController->appear();

        if (::getMiiFacePartsHolder()->isInitEnd()) {
            if (::getMiiFacePartsHolder()->isError()) {
                setNerve(GET_NERVE(FileSelector, FileSelectorNrvRFLError));
            } else {
                setNerve(GET_NERVE(FileSelector, FileSelectorNrvTitleEnd));
            }
        } else {
            setNerve(GET_NERVE(FileSelector, FileSelectorNrvRFLWait));
        }
    }
}

void FileSelector::exeTitleEnd() {
    if (MR::isFirstStep(this)) {
#ifdef GALAXY_NATIVE
        petari_milestone("FileSelector.TitleEnd");
#endif
        if (MR::isValidSwitchA(this)) {
            MR::onSwitchA(this);
        }

        mCameraController->goToFarPoint();
        calcBasePos(0.0f);
        appearAllItems();
        initAllItems();
        mMiiSelect->collectValidMiiIndex();
        invalidateSelectAll();
        MR::startStarPointerModeFileSelect(this);
        updateChildObjs();
    }

    if (mCameraController->isAtFarPoint() && MR::isEndStationedArchiveLoader()) {
        validateRotateAllItems();
        setNerve(GET_NERVE(FileSelector, FileSelectorNrvFileSelect));
    }
}

void FileSelector::exeRFLError() {
    if (MR::isFirstStep(this)) {
#ifdef GALAXY_NATIVE
        petari_milestone("FileSelector.RFLError");
#endif
        mSysInfoWindowMini->appear("System_FileSelect018", SysInfoWindow::Type_Key, SysInfoWindow::TextPos_Center, SysInfoWindow::MessageType_System);
        MR::showPaneRecursive(mSysInfoWindowMini, "BG");
        stopMovementForLayout();
    }

    if (MR::isDead(mSysInfoWindowMini)) {
        restartMovementForLayout();
        setNerve(GET_NERVE(FileSelector, FileSelectorNrvTitleEnd));
    }
}

void FileSelector::exeRFLWait() {
    if (MR::isFirstStep(this)) {
        mSysInfoWindowMini->appear("System_FileSelect017", SysInfoWindow::Type_Blocking, SysInfoWindow::TextPos_Center,
                                   SysInfoWindow::MessageType_System);
        MR::showPaneRecursive(mSysInfoWindowMini, "BG");
        stopMovementForLayout();
    }

    if (::getMiiFacePartsHolder()->isInitEnd() && mSysInfoWindowMini->isWait()) {
        setNerve(GET_NERVE(FileSelector, FileSelectorNrvRFLWaitEnd));
    }
}

void FileSelector::exeRFLWaitEnd() {
    if (MR::isFirstStep(this)) {
        mSysInfoWindowMini->disappear();
        restartMovementForLayout();
    }

    if (MR::isDead(mSysInfoWindowMini)) {
        setNerve(GET_NERVE(FileSelector, FileSelectorNrvTitleEnd));
    }
}

void FileSelector::exeFileSelectStart() {
    if (MR::isFirstStep(this)) {
#ifdef GALAXY_NATIVE
        petari_milestone("FileSelector.FileSelectStart");
#endif
        mCameraController->goToFarPoint();
        calcBasePos(0.0f);
        validateRotateAllItems();
    }

    if (mCameraController->isAtFarPoint()) {
        setNerve(GET_NERVE(FileSelector, FileSelectorNrvFileSelect));
    }
}

void FileSelector::exeFileSelect() {
    if (MR::isFirstStep(this)) {
#ifdef GALAXY_NATIVE
        petari_milestone("FileSelector.FileSelect");
#endif
        validateSelectAll();
        appearAllIndex();
        MR::activeStarPointerGuidance();
#ifdef GALAXY_NATIVE
        s32 openFile = PetariNative::PlayerLaunch::openFile;
        PetariNative::PlayerLaunch::openFile = 0;
        namespace Launch = PetariNative::App::LaunchStage;
        const bool startupFile = openFile <= 0 && !sNativeStartupFileUsed && Launch::startupStatus() == Launch::StartupStatus::PendingFileLoad;
        if (startupFile) {
            sNativeStartupFileUsed = true;
            openFile = 1;
        }
        const int openIndex = openFile > 0 ? ::getItemArrayIndex(openFile) : -1;
        FileSelectItem* pOpen = openIndex >= 0 ? static_cast<FileSelectItem*>(mItems->getActor(openIndex)) : nullptr;
        if (startupFile && pOpen != nullptr) {
            NativeCompleteSave::checkSelection(openFile, pOpen->isNew());
        }
        if (startupFile && pOpen != nullptr && pOpen->isNew() && !isUserFileCorrupted(openFile)) {
            petari_milestone("FileSelector.StartupCreate");
            mSelectedItem = pOpen;
            sNativeStartupCreate = true;
            setNerve(GET_NERVE(FileSelector, FileSelectorNrvCreate));
            return;
        }
        if (pOpen != nullptr && !pOpen->isNew() && !isUserFileCorrupted(openFile) && !pOpen->mHasInvalidMiiId) {
            petari_milestone("FileSelector.OpenFile");
            mSelectedItem = pOpen;
            MR::requestFileSelectGuidance();
            updatePointingGuidance();
            setNerve(GET_NERVE(FileSelector, FileSelectorNrvDemoStartWait));
            return;
        }
#endif
    } else {
        updateFileInfo();
    }

    MR::requestFileSelectGuidance();
    updatePointingGuidance();
}

void FileSelector::exeFileConfirmStart() {
    if (MR::isFirstStep(this)) {
        goToNearPoint();
        invalidateSelectAll();
        disappearAllIndex();
        mSelectedItem->turnToFront(40);
        mSelectedItem->invalidateRotate();
    }

    if (mCameraController->isAtNearPoint()) {
        if (mSelectedItem->mHasInvalidMiiId) {
            setNerve(GET_NERVE(FileSelector, FileSelectorNrvFileConfirmMiiDeleteWarningStart));
        } else {
            setNerve(GET_NERVE(FileSelector, FileSelectorNrvFileConfirm));
        }
    }
}

void FileSelector::exeFileConfirmMiiDeleteWarningStart() {
    if (MR::isFirstStep(this)) {
        disappearAllLayout();
    }

    if (isHiddenAllLayout()) {
        setNerve(GET_NERVE(FileSelector, FileSelectorNrvFileConfirmMiiDeleteWarning));
    }
}

void FileSelector::exeFileConfirmMiiDeleteWarning() {
    if (MR::isFirstStep(this)) {
        mSysInfoWindowMini->appear("System_FileSelect009", SysInfoWindow::Type_Key, SysInfoWindow::TextPos_Center, SysInfoWindow::MessageType_System);
        MR::showPaneRecursive(mSysInfoWindowMini, "BG");
        stopMovementForLayout();
    }

    if (MR::isDead(mSysInfoWindowMini)) {
        restartMovementForLayout();
        setNerve(GET_NERVE(FileSelector, FileSelectorNrvFileConfirmMiiDeleteSave));
    }
}

void FileSelector::exeFileConfirmMiiDeleteSave() {
    if (MR::isFirstStep(this)) {
        u32 arg = 1;
        GameSequenceFunction::storeMiiOrIconIdAndSaveAllUserFileSequence(mSelectedItem->mFileIndex, nullptr, &arg);
    }

    if (!GameSequenceFunction::isActiveSaveDataHandleSequence()) {
        mSelectedItem->mHasInvalidMiiId = false;
        restoreUserFile();
        setNerve(GET_NERVE(FileSelector, FileSelectorNrvFileConfirm));
    }
}

void FileSelector::exeFileConfirm() {
    if (MR::isFirstStep(this)) {
        mOperationButton->appear();
        mBackButton->appear();

        if (MR::isDead(mFileInfo)) {
            setFileInfo(mSelectedItem->mFileIndex);
            mFileInfo->appear();
        }

        mFileInfo->slide();
        mSelectedItem->requestRotateRefresh();
    }

    if (MR::isStep(this, 90)) {
        mSelectedItem->validateRotate();
    }

    if (checkSelectedBackButton()) {
        mSelectedItem->validateRotate();
        disappearAllLayout();
        clearPointing();
        mMiiSelect->clearPage();
        setNerve(GET_NERVE(FileSelector, FileSelectorNrvFileSelectStart));
        return;
    }

    updatePointingGuidanceSelect();
}

void FileSelector::exeDemoStartWait() {
    if (MR::isFirstStep(this)) {
#ifdef GALAXY_NATIVE
        petari_milestone("FileSelector.DemoStartWait");
#endif
        disappearAllLayout();
        MR::stopSystemSE("SE_SY_LV_TITLE_BGM", 30);
        MR::stopSystemSE("SE_SY_LV_TITLE_BGM_RHYTHM", 30);
        mIsTitleBgmRhythmEnabled = false;
    }

#ifdef GALAXY_NATIVE
    // The startup file's icon save may still be running (inactive on every normal path).
    if (GameSequenceFunction::isActiveSaveDataHandleSequence()) {
        return;
    }
#endif
    if (isHiddenAllLayout()) {
        setNerve(GET_NERVE(FileSelector, FileSelectorNrvDemo));
    }
}

void FileSelector::exeDemo() {
    if (MR::isFirstStep(this)) {
        GameSequenceFunction::startLoadUserFileSequence(mSelectedItem->mFileIndex);
        MR::stopSystemSE("SE_SY_LV_TITLE_BGM", 30);
        MR::stopSystemSE("SE_SY_LV_TITLE_BGM_RHYTHM", 30);
        mIsTitleBgmRhythmEnabled = false;
    }

    FileSelectIconID iconId;
    getIconId(&iconId, mSelectedItem->mFileIndex);
    wchar_t name[12];

    if (iconId.isMii()) {
        FileSelectFunc::copyMiiName(reinterpret_cast< u16* >(name), iconId);
    } else {
        MR::copyString(name, MR::getGameMessageDirect(cMarioNameMessageID), 12);
    }

    GameDataFunction::setUserName(name);
#ifdef GALAXY_NATIVE
    if (!NativeCompleteSave::beforeStageLaunch()) return;
#endif
    GameDataFunction::resetPlayerLeftAndTryOnDimensionBigCastleGalaxyComet();
    GameSequenceFunction::requestChangeStageAfterFileSelect();
    MR::startSystemWipeCircleWithCaptureScreen(90);
}

void FileSelector::exeCreateConfirmStart() {
    if (MR::isFirstStep(this)) {
        goToNearPoint();
        invalidateSelectAll();
        disappearAllIndex();
    }

    if (mCameraController->isAtNearPoint()) {
        setNerve(GET_NERVE(FileSelector, FileSelectorNrvCreateConfirm));
    }
}

void FileSelector::exeCreateConfirm() {
    if (MR::isFirstStep(this)) {
        mSysInfoWindow->appear("System_FileSelect001", SysInfoWindow::Type_YesNo, SysInfoWindow::TextPos_Center, SysInfoWindow::MessageType_System);
        MR::showPaneRecursive(mSysInfoWindow, "BG");
        mSysInfoWindow->setYesNoSelectorSE("SE_SY_BUTTON_CURSOR_ON", "SE_SY_FILE_SEL_NEW_FILE", "SE_SY_TALK_SELECT_NO");
        stopMovementForLayout();
    }

    if (mSysInfoWindow->isDisappear()) {
        restartMovementForLayout();
    }

    if (MR::isDead(mSysInfoWindow)) {
        mSysInfoWindow->resetYesNoSelectorSE();

        if (mSysInfoWindow->isSelectedYes()) {
            setNerve(GET_NERVE(FileSelector, FileSelectorNrvCreate));
        } else {
            clearPointing();
            setNerve(GET_NERVE(FileSelector, FileSelectorNrvFileSelectStart));
        }
    }
}

void FileSelector::exeCreate() {
    if (MR::isFirstStep(this)) {
#ifdef GALAXY_NATIVE
        petari_milestone("FileSelector.Create");
#endif
        GameSequenceFunction::startCreateUserFileSequence(mSelectedItem->mFileIndex);
    }

    if (GameSequenceFunction::isActiveSaveDataHandleSequence()) {
        return;
    }

    if (GameSequenceFunction::isSuccessSaveDataHandleSequence()) {
        restoreUserFile();
#ifdef GALAXY_NATIVE
        if (sNativeStartupCreate) {
            // Startup file: Mario icon, saved like a confirmed icon choice, then loaded (MiiCreateWait/FileConfirm skipped).
            sNativeStartupCreate = false;
            FileSelectIconID iconId;
            iconId.setFellowID(FileSelectIconID::Mario);
            storeSetMiiIdUserFile(mSelectedItem->mFileIndex, iconId);
            setNerve(GET_NERVE(FileSelector, FileSelectorNrvDemoStartWait));
            return;
        }
#endif
        setNerve(GET_NERVE(FileSelector, FileSelectorNrvMiiSelectStartFirst));
    } else {
        clearPointing();
        restartMovementForLayout();
        setNerve(GET_NERVE(FileSelector, FileSelectorNrvFileSelectStart));
    }
}











void FileSelector::exeMiiTip() {
    MR::isFirstStep(this);

    if (mBackButton->isHidden() && MR::isDead(mFileInfo)) {
        setNerve(GET_NERVE(FileSelector, FileSelectorNrvMiiSelectStart));
    }
}

void FileSelector::exeMiiSelectStart() {
    if (MR::isFirstStep(this)) {
        if (isNerve(GET_NERVE(FileSelector, FileSelectorNrvMiiSelectStart))) {
            FileSelectIconID iconId = FileSelectIconID();

            mSelectedItem->copyIconID(&iconId);
            mMiiSelect->prohibitIcon(iconId);
        } else {
            mIsMiiSelectStartFirst = true;

            mMiiSelect->admitIcon();
        }

        mMiiSelect->validateAllSpecialMii();

        if (!mUserFile[mSelectedItem->mFileIndex - 1].isView120StarsEnding()) {
            mMiiSelect->invalidateSpecialMii(FileSelectIconID::Rosetta);
        }

        mMiiSelect->appear();
        stopMovementForLayout();
    }

    if (mMiiSelect->isAppearing()) {
        return;
    }

    setNerve(GET_NERVE(FileSelector, FileSelectorNrvMiiSelect));
}

void FileSelector::exeMiiSelect() {
#ifdef GALAXY_NATIVE
    if (MR::isFirstStep(this)) {
        petari_milestone("FileSelector.MiiSelect");
    }
#endif
    if (MR::isFirstStep(this) && !mIsMiiSelectStartFirst) {
        mBackButton->appear();
    }

    if (mMiiSelect->isDummySelected()) {
        MR::startActionSound(this, "SyFileSelMiiSelected", -1, -1, -1);
        mMiiSelect->disappear();

        if (!mBackButton->isHidden()) {
            mBackButton->disappear();
        }

        setNerve(GET_NERVE(FileSelector, FileSelectorNrvMiiInfoStart));
        return;
    }

    if (mMiiSelect->isSelected()) {
        MR::startActionSound(this, "SyFileSelMiiSelected", -1, -1, -1);
        mMiiSelect->disappear();

        if (!mBackButton->isHidden()) {
            mBackButton->disappear();
        }

        setNerve(GET_NERVE(FileSelector, FileSelectorNrvMiiConfirmWait));
        return;
    }

    if (checkSelectedBackButton()) {
        mMiiSelect->disappear();
        mBackButton->disappear();
        restartMovementForLayout();
        setNerve(GET_NERVE(FileSelector, FileSelectorNrvMiiCancel));
    }
}

void FileSelector::exeMiiCancel() {
    if (!MR::isDead(mMiiSelect)) {
        return;
    }

    if (!mBackButton->isHidden()) {
        return;
    }

    setNerve(GET_NERVE(FileSelector, FileSelectorNrvFileConfirm));
}

void FileSelector::exeMiiConfirmWait() {
    if (!MR::isDead(mMiiSelect)) {
        return;
    }

    if (!mBackButton->isHidden()) {
        return;
    }

    setNerve(GET_NERVE(FileSelector, FileSelectorNrvMiiConfirm));
}

void FileSelector::exeMiiConfirm() {
    if (MR::isFirstStep(this)) {
        if (mIsMiiSelectStartFirst) {
            mSysInfoWindow->appear("System_FileSelect013", SysInfoWindow::Type_YesNo, SysInfoWindow::TextPos_Bottom,
                                   SysInfoWindow::MessageType_System);
            MR::showPaneRecursive(mSysInfoWindow, "BG");
        } else {
            mSysInfoWindow->appear("System_FileSelect005", SysInfoWindow::Type_YesNo, SysInfoWindow::TextPos_Bottom,
                                   SysInfoWindow::MessageType_System);
            MR::showPaneRecursive(mSysInfoWindow, "BG");
        }

        u16 name[12];
        FileSelectIconID iconId = FileSelectIconID();

        mMiiSelect->getSelectedID(&iconId);
        FileSelectFunc::copyMiiName(name, iconId);
        mMiiConfirmIcon->appear(mMiiSelect->getSelectedMiiTexMap(), reinterpret_cast< wchar_t* >(name));
        mSysInfoWindow->setYesNoSelectorSE("SE_SY_BUTTON_CURSOR_ON", "SE_SY_FILE_SEL_MII_CHANGE", "SE_SY_TALK_SELECT_NO");
    }

    if (!mMiiConfirmIcon->isDisappear() && mSysInfoWindow->isDisappear()) {
        mMiiConfirmIcon->disappear();
    }

    if (mSysInfoWindow->isDisappear()) {
        if (mSysInfoWindow->isYesButtonSelected()) {
            restartMovementForLayout();
        }
    }

    if (MR::isDead(mSysInfoWindow)) {
        mSysInfoWindow->resetYesNoSelectorSE();

        if (mSysInfoWindow->isSelectedYes()) {
            setNerve(GET_NERVE(FileSelector, FileSelectorNrvMiiCreateWait));
            return;
        }

        if (mIsMiiSelectStartFirst) {
            setNerve(GET_NERVE(FileSelector, FileSelectorNrvMiiSelectStartFirst));
            return;
        }

        setNerve(GET_NERVE(FileSelector, FileSelectorNrvMiiSelectStart));
    }
}

void FileSelector::exeMiiCreateWait() {
    if (MR::isFirstStep(this)) {
        FileSelectIconID iconId = FileSelectIconID();

        mMiiSelect->getSelectedID(&iconId);
        storeSetMiiIdUserFile(mSelectedItem->mFileIndex, iconId);

        mIsMiiSelectStartFirst = false;
    }

    if (GameSequenceFunction::isActiveSaveDataHandleSequence()) {
        return;
    }

    setNerve(GET_NERVE(FileSelector, FileSelectorNrvMiiCreateDemo));
}

void FileSelector::exeMiiCreateDemo() {
    if (MR::isFirstStep(this)) {
        restoreUserFile();

        s32 id = mSelectedItem->mFileIndex;
        FileSelectIconID iconId = FileSelectIconID();

        mMiiSelect->getSelectedID(&iconId);
        mSelectedItem->change(iconId, mCompleteFlags[id - 1]);
        setFileInfo(id);
    }

    if (!mSelectedItem->isExist()) {
        return;
    }

    FileSelectIconID iconId = FileSelectIconID();

    mMiiSelect->getSelectedID(&iconId);

    if (iconId.isMii()) {
        setNerve(GET_NERVE(FileSelector, FileSelectorNrvMiiCaution));
    } else {
        setNerve(GET_NERVE(FileSelector, FileSelectorNrvFileConfirm));
    }
}

void FileSelector::exeMiiCaution() {
    if (MR::isFirstStep(this)) {
        mSysInfoWindowMini->appear("System_FileSelect006", SysInfoWindow::Type_Key, SysInfoWindow::TextPos_Center, SysInfoWindow::MessageType_System);
        MR::showPaneRecursive(mSysInfoWindowMini, "BG");
        stopMovementForLayout();
    }

    if (MR::isDead(mSysInfoWindowMini)) {
        restartMovementForLayout();
        setNerve(GET_NERVE(FileSelector, FileSelectorNrvFileConfirm));
    }
}

void FileSelector::exeMiiInfoStart() {
    if (!MR::isDead(mMiiSelect)) {
        return;
    }

    if (!mBackButton->isHidden()) {
        return;
    }

    setNerve(GET_NERVE(FileSelector, FileSelectorNrvMiiInfo));
}

void FileSelector::exeMiiInfo() {
    if (MR::isFirstStep(this)) {
        mSysInfoWindowMini->appear("System_FileSelect015", SysInfoWindow::Type_Key, SysInfoWindow::TextPos_Center, SysInfoWindow::MessageType_System);
        MR::showPaneRecursive(mSysInfoWindowMini, "BG");
    }

    if (!MR::isDead(mSysInfoWindowMini)) {
        return;
    }

    if (mIsMiiSelectStartFirst) {
        setNerve(GET_NERVE(FileSelector, FileSelectorNrvMiiSelectStartFirst));
    } else {
        setNerve(GET_NERVE(FileSelector, FileSelectorNrvMiiSelectStart));
    }
}

void FileSelector::exeDeleteConfirmStart() {
    if (MR::isFirstStep(this)) {
        disappearAllLayout();
    }

    if (isHiddenAllLayout()) {
        setNerve(GET_NERVE(FileSelector, FileSelectorNrvDeleteConfirm));
    }
}

void FileSelector::exeDeleteConfirm() {
    if (MR::isFirstStep(this)) {
        mSysInfoWindow->setYesNoSelectorSE("SE_SY_BUTTON_CURSOR_ON", "SE_SY_FILE_SEL_DELETE", "SE_SY_TALK_SELECT_NO");
        mSysInfoWindow->appear("System_FileSelect007", SysInfoWindow::Type_YesNo, SysInfoWindow::TextPos_Center, SysInfoWindow::MessageType_System);
        MR::showPaneRecursive(mSysInfoWindow, "BG");
        stopMovementForLayout();
    }

    if (mSysInfoWindow->isDisappear()) {
        restartMovementForLayout();
    }

    if (MR::isDead(mSysInfoWindow)) {
        mSysInfoWindow->resetYesNoSelectorSE();

        if (mSysInfoWindow->isSelectedYes()) {
            setNerve(GET_NERVE(FileSelector, FileSelectorNrvDelete));
        } else {
            mOperationButton->shiftSelect();
            setNerve(GET_NERVE(FileSelector, FileSelectorNrvFileConfirm));
        }
    }
}

void FileSelector::exeDelete() {
    if (MR::isFirstStep(this)) {
        isAllOtherFileNew(mSelectedItem);
        GameSequenceFunction::startDeleteUserFileSequence(mSelectedItem->mFileIndex);
    }

    if (GameSequenceFunction::isActiveSaveDataHandleSequence()) {
        return;
    }

    if (GameSequenceFunction::isSuccessSaveDataHandleSequence()) {
        setNerve(GET_NERVE(FileSelector, FileSelectorNrvDeleteDemo));
    } else {
        clearPointing();
        setNerve(GET_NERVE(FileSelector, FileSelectorNrvFileSelectStart));
    }
}

void FileSelector::exeDeleteDemo() {
    if (MR::isFirstStep(this)) {
        mSelectedItem->format();
    }

    if (mSelectedItem->isNew()) {
        initUserFile();
        clearPointing();
        setNerve(GET_NERVE(FileSelector, FileSelectorNrvFileSelectStart));
    }
}


void FileSelector::exeManualStart() {
    if (MR::isFirstStep(this)) {
        disappearAllLayout();
    }

    if (isHiddenAllLayout()) {
        setNerve(GET_NERVE(FileSelector, FileSelectorNrvManual));
    }
}

void FileSelector::exeManual() {
    if (MR::isFirstStep(this)) {
        mManual->appear();
        stopMovementForLayout();
    }

    if (mManual->isClosed()) {
        restartMovementForLayout();
        setNerve(GET_NERVE(FileSelector, FileSelectorNrvFileConfirm));
    }
}


FileSelector::~FileSelector() {
}
