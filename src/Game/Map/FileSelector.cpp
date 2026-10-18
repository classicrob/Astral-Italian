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

// Missing from shared include/Game/System/GameSequenceFunction.hpp (defined in its .cpp); local decl only.
namespace GameSequenceFunction {
    void restoreUserFile(UserFile*, int);
    bool isSuccessSaveDataHandleSequence();
    void startCreateUserFileSequence(int);
    void startDeleteUserFileSequence(int);
    void requestChangeStageAfterFileSelect();
}

namespace GameDataFunction {
    typedef wchar_t* WideStr;
    void setUserName(const WideStr);
}

namespace MR {
    void startSystemWipeCircleWithCaptureScreen(s32);
}

namespace StarPointerUtil {
    bool sub_8005E720(LayoutActor*, bool);
    void sub_8005E790(LayoutActor*);
    void sub_8005E940(LayoutActor*);
    TVec2f* getDefaultButtonOffsetVec2();
    void addStarPointerMovePosition(const char*, TVec3f*, const TVec3f&);
    void addStarPointerMovePositionFromPane(LayoutActor*, const char*, TVec2f*);
    void setStarPointerMovePosition(const char*, TVec2f*);
    void setConnectionMovePositionRight2Way(const char*, const char*);
    void setConnectionMovePositionRight(const char*, const char*);
    void setConnectionMovePositionDown(const char*, const char*);
}
extern "C" void fn_8005EA60(const char*);
extern "C" void fn_8005EAA0(const char*);
extern "C" void fn_8005EAE0(const char*, const char*);
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

#define USER_FILE_NUM 3

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


void FileSelector_FORCE_MATCH_SDATA2() {
    (void)1.0f;
    (void)0.0f;
}

extern "C" void fn_8004B340(NameObj*);
extern "C" void fn_803B6830(int);
extern "C" void fn_8045E970(BackButton*);
extern "C" bool fn_8045E9D0(BackButton*);
extern "C" void fn_80486AA0();
extern "C" bool fn_804A2BB0(SysInfoWindow*);
extern "C" void fn_80486AF0();
extern "C" void fn_80060290();
extern "C" bool fn_800602A0();
extern "C" bool fn_804A54D0(TitleSequenceProduct*);
extern "C" void fn_8024CFE0(FileSelectItem*);
extern "C" void fn_8024D010(FileSelectItem*);
extern "C" void fn_804D5F60(s32, s32, s32*);
extern "C" void fn_804D5E70(s32);
extern "C" void fn_804D4700();
extern "C" void fn_804D3DF0();

FileSelector::FileSelector(const char* pName)
    : LiveActor(pName), mCameraController(), mSky(), mItems(), mOperationButton(), mBackButton(), mSysInfoWindow(), _B0(), _B4(), _B8(), _BC(),
      mUserFile(), _C8(), mTitle(), mMiiSelect(), mMiiId(new RFLCreateID()), mManual(), mIsMiiSelectStartFirst(), mBgmState(), mSelectEffect(),
      _EC(), _F0(), _F4(), _F8(), _FC(), _100(), _104() {
}

// NONMATCHING: the two TVec3f temporaries (sensor offset / sound offset) get swapped stack slots
void FileSelector::init(const JMapInfoIter& rIter) {
    MR::connectToScene(this, 0x20, -1, -1, -1);
    initHitSensor(1);
    MR::addHitSensorPriorBinder(this, "Body", 8, 500.0f, TVec3f(0.0f, 0.0f, 0.0f));
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
    _F8 = false;
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
    fn_803B6830(nullptr);
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
void FileSelector::control() {
    _BC = nullptr;

    for (int i = 0; i < mItems->getObjNum(); i++) {
        LiveActor* pActor = mItems->getActor(i);
        TVec3f& rPosition = pActor->mPosition;
        rPosition = _9C[i] * 0.05f + rPosition * 0.95f;
    }

    for (int i = 0; i < mItems->getObjNum(); i++) {
        MR::setPosition(&mSelectEffect[i], mItems->getActor(i)->mPosition + TVec3f(0.0f, 1000.0f, 0.0f));
    }

    updateBgm();
}

bool FileSelector::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    return false;
}

void FileSelector::initUserFileArray() {
    mUserFile = new UserFile[USER_FILE_NUM]();
    _C8 = new bool[USER_FILE_NUM];

    initUserFile();
}

void FileSelector::createCameraController() {
    mCameraController = new FileSelectCameraController("ファイルセレクトカメラ制御");
    mCameraController->initWithoutIter();
}

void FileSelector::createFileItems() {
    mItems = new DeriveActorGroup< FileSelectItem >("全ファイルアイテム保持", USER_FILE_NUM);
    _9C = new TVec3f[USER_FILE_NUM]();

    calcBasePos(0.0f);
    MR::createSceneObj(SceneObj_MiiFacePartsHolder);

    FileSelectItemDelegator< FileSelector >* pDelegator = new FileSelectItemDelegator< FileSelector >(this, &FileSelector::notifyItem);

    for (int i = 0; i < USER_FILE_NUM; i++) {
        FileSelectItem* pItem = createFileItem(::sIndexOrder[i]);

        pItem->setSelectDelegator(pDelegator);
        MR::setPosition(pItem, _9C[i]);
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
    fn_8045E970(mBackButton);
    MR::connectToScene(mBackButton, 0xE, 0xD, -1, 0x51);
}

void FileSelector::createSysInfoWindow() {
    mSysInfoWindow = MR::createSysInfoWindow();
    fn_8004B340(mSysInfoWindow);
    mSysInfoWindowMini = MR::createSysInfoWindowMiniExecuteWithChildren();
    fn_8004B340(mSysInfoWindowMini);
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
        MR::setPosition(&mSelectEffect[i], _9C[i]);
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
    if (_BC != nullptr) {
        if (_B8 != _BC) {
            if (_B8 != nullptr) {
                clearPointing();
                mFileInfo->disappear();
            }

            if (_BC->isExist()) {
                setFileInfo(_BC->_140);
                mFileInfo->appear();
            }

            _B8 = _BC;
            _B8->onPointing();
        }
    } else if (_B8 != nullptr) {
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
        _C8[i] = mUserFile[i].isComplete();
    }
}

void FileSelector::onPoint(FileSelectItem* pItem) {
    if (mBackButton->isHidden() || !mBackButton->isPointing()) {
        if (_BC != nullptr) {
            if (_BC->_140 > pItem->_140) {
                _BC = pItem;
            }
        } else {
            _BC = pItem;
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

    _B0 = pItem;

    MR::startActionSound(this, "SyGalaxySelected", -1, -1, -1);
    playSelectedME();
}

void FileSelector::clearPointing() {
    if (_B8 == nullptr) {
        return;
    }

    _B8->offPointing();
    _B8 = nullptr;
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

    if (static_cast< FileSelectItem* >(mItems->getActor(::getItemArrayIndex(id)))->_146) {
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

    return fn_8045E9D0(mBackButton);
}

void FileSelector::goToNearPoint() {
    calcBasePos(-16000.0f);
    mCameraController->goToNearPoint(_9C[::getItemArrayIndex(_B0->_140)]);
}

// NONMATCHING: float constant registers f30/f31 swapped
void FileSelector::calcBasePos(f32 offset) {
    TVec3f pos;
    pos.x = -3000.0f;
    pos.y = offset;
    pos.z = 0.0f;

    for (int i = 0; i < USER_FILE_NUM; i++) {
        if (_B0 == nullptr || _B0 != mItems->getActor(i)) {
            _9C[i].set(pos);
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
        s32 id = pItem->_140;

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
                pItem->_146 = true;
            } else if (pSelector->isUserFileMiiIdValid(id)) {
                u32 miiIndex = pSelector->getUserFileMiiIndex(id);

                iconId.setMiiIndex(miiIndex);
            } else {
                pItem->_146 = true;
            }
        }

        pItem->forceChange(iconId, pSelector->_C8[id - 1]);
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
    if (!_EC) {
        fn_80486AA0();

        if (mSky != nullptr) {
            MR::requestMovementOff(mSky);
        }

        if (_B0 != nullptr) {
            MR::requestMovementOff(_B0);
        }

        _EC = true;
    }
}

void FileSelector::restartMovementForLayout() {
    if (_EC) {
        fn_80486AF0();

        if (mSky != nullptr) {
            MR::requestMovementOn(mSky);
        }

        if (_B0 != nullptr) {
            MR::requestMovementOn(_B0);
        }

        _EC = false;
    }
}

void FileSelector::initChildObjs(const JMapInfoIter& rIter) {
    _F4 = MR::getChildObjNum(rIter);
    _F0 = new LiveActor*[_F4];

    for (int i = 0; i < _F4; i++) {
        _F0[i] = static_cast< LiveActor* >(NameObjFactory::initChildObj(rIter, i));
    }
}

// NONMATCHING: register allocation (loop counter / actor pointer registers swapped)
void FileSelector::updateChildObjs() {
    f32 degToRad = PI / 180.0f;
    f32 radToDeg = 180.0f / PI;

    TPos3f mtxA;
    mtxA.identity();
    mtxA.setRotate(35.0f * degToRad, 0.0f, 345.0f * degToRad);
    mtxA.invert(mtxA);

    TPos3f mtxB;
    mtxB.identity();
    mtxB.setRotate(335.0f * degToRad, 0.0f, 0.0f);
    mtxB.concat(mtxA);

    for (int i = 0; i < _F4; i++) {
        TPos3f mtx;
        mtx.identity();

        TVec3f rot(_F0[i]->mRotation);
        rot.scale(degToRad);
        mtx.setRotate(rot.x, rot.y, rot.z);
        mtx.concat(mtxB, mtx);

        LiveActor* pActor = _F0[i];
        mtx.getEulerXYZ(pActor->mRotation);
        pActor->mRotation.scale(radToDeg);
    }
}

FileSelectIconID::EFellowID FileSelector::getUserFileFellowID(s32 id) const {
    u32 iconId;

    if (mUserFile[id - 1].getIconId(&iconId)) {
        FileSelectIconID::EFellowID fellowId = FileSelectIconID::Mario;

        if (iconId <= 5) {
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
    bool result = false;

    if (mUserFile[id - 1].calcNormalPowerStarNum() >= 120) {
        if (mUserFile[id - 1].getPowerStarNum() >= 240) {
            result = true;
        }
    }

    return result;
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

    if (RFLGetAdditionalInfo(&additionalInfo, RFLDataSource_Official, nullptr, index) == RFLErrcode_Success) {
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
    if (!_C8[id - 1]) {
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

        if (pOther != pItem && mUserFile[pOther->_140 - 1].isCreated()) {
            return false;
        }
    }

    fn_804D4700();
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

void FileSelector_FORCE_MATCH() {
    MR::startSystemME("ME_ASTRO_DOME_CALCEL1");
    MR::startSystemME("ME_ASTRO_DOME_CALCEL2");
    MR::startSystemME("ME_ASTRO_DOME_CALCEL3");
    MR::startSystemME("ME_ASTRO_DOME_CALCEL4");
    MR::startSystemME("ME_ASTRO_DOME_CALCEL5");
}

// NONMATCHING: register allocation in the smoothing step
void FileSelector::updateBgm() {
    if (_F8) {
        if (mCameraController != nullptr) {
            s32 state = 0;

            if (mCameraController->isToOrAtFarPoint()) {
                _100 = 0;
                state = 0;
            } else if (mCameraController->isToOrAtNearPoint()) {
                _100 = 100;
                state = 1;
            }

            if (mBgmState != state) {
                _104 = 60;
                mBgmState = state;
            }

            if (_104 > 0) {
                s32 frame = _104;
                _104 = frame - 1;
                _FC += (_100 - _FC) / frame;
            }

            MR::startSystemLevelSE("SE_SY_LV_TITLE_BGM", -1, -1);
            MR::startSystemLevelSE("SE_SY_LV_TITLE_BGM_RHYTHM", _FC, -1);
        }
    }
}

// NONMATCHING: register allocation (loop counter / actor registers swapped in the second loop)
void FileSelector::updatePointingGuidance() {
    TVec2f screenPos;
    TVec2f pos;
    TVec2f size;

    if (StarPointerUtil::sub_8005E720(reinterpret_cast< LayoutActor* >(this), true)) {
        for (int i = 0; i < mItems->getObjNum(); i++) {
            LiveActor* pActor = mItems->getActor(i);
            size.x = 0.0f;
            size.y = 0.0f;
            pos.x = 0.0f;
            pos.y = 0.0f;
            StarPointerUtil::addStarPointerMovePosition(pActor->mName, reinterpret_cast< TVec3f* >(&pos),
                                                        *reinterpret_cast< const TVec3f* >(&size));
        }

        LiveActor* pActor;
        for (int i = 0; i < mItems->getObjNum() - 1; i++) {
            pActor = mItems->getActor(i);
            StarPointerUtil::setConnectionMovePositionRight2Way(pActor->mName, mItems->getActor(i + 1)->mName);
        }

        LiveActor* pFirst = mItems->getActor(0);
        LiveActor* pLast = mItems->getActor(mItems->getObjNum() - 1);
        StarPointerUtil::setConnectionMovePositionRight2Way(pLast->mName, pFirst->mName);
        fn_8005EA60(pFirst->mName);
        fn_8005EAA0(pLast->mName);
    }

    for (int i = 0; i < mItems->getObjNum(); i++) {
        LiveActor* pActor = mItems->getActor(i);
        MR::calcScreenPosition(&screenPos, pActor->mPosition);
        StarPointerUtil::setStarPointerMovePosition(pActor->mName, &screenPos);
    }

    StarPointerUtil::sub_8005E790(reinterpret_cast< LayoutActor* >(this));
}

void FileSelector::updatePointingGuidanceSelect() {
    if (StarPointerUtil::sub_8005E720(reinterpret_cast< LayoutActor* >(this), true)) {
        StarPointerUtil::addStarPointerMovePositionFromPane(mBackButton, "BoxButton", StarPointerUtil::getDefaultButtonOffsetVec2());
        StarPointerUtil::addStarPointerMovePositionFromPane(mOperationButton, "PicMii", StarPointerUtil::getDefaultButtonOffsetVec2());
        StarPointerUtil::addStarPointerMovePositionFromPane(mOperationButton, "PicDelete", StarPointerUtil::getDefaultButtonOffsetVec2());
        StarPointerUtil::addStarPointerMovePositionFromPane(mOperationButton, "BoxStartButton", StarPointerUtil::getDefaultButtonOffsetVec2());
        StarPointerUtil::setConnectionMovePositionDown("PicMii", "BoxButton");
        StarPointerUtil::setConnectionMovePositionDown("PicDelete", "BoxStartButton");
        StarPointerUtil::setConnectionMovePositionRight("PicMii", "PicDelete");
        StarPointerUtil::setConnectionMovePositionRight("BoxButton", "BoxStartButton");
        fn_8005EAE0("BoxButton", "BoxStartButton");
    }

    StarPointerUtil::sub_8005E940(mBackButton);
    StarPointerUtil::sub_8005E940(mOperationButton);
    StarPointerUtil::sub_8005E790(reinterpret_cast< LayoutActor* >(this));
}

void FileSelector::exeTitle() {
    if (MR::isFirstStep(this)) {
        mTitle->appear();
        MR::deactivateDefaultGameLayout();
        MR::startStarPointerModeTitle(this);
        MR::resetCameraMan();
        fn_80060290();
        _F8 = false;
    }

    mTitle->updateNerve();

    if (fn_804A54D0(mTitle)) {
        _F8 = true;
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

    if (mCameraController->isAtFarPoint() && fn_800602A0()) {
        validateRotateAllItems();
        setNerve(GET_NERVE(FileSelector, FileSelectorNrvFileSelectStart));
    }
}

void FileSelector::exeRFLError() {
    if (MR::isFirstStep(this)) {
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
        validateSelectAll();
        appearAllIndex();
        MR::activeStarPointerGuidance();
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
        _B0->turnToFront(40);
        fn_8024CFE0(_B0);
    }

    if (mCameraController->isAtNearPoint()) {
        if (_B0->_146) {
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
        s32 arg = 1;
        fn_804D5F60(_B0->_140, 0, &arg);
    }

    if (!GameSequenceFunction::isActiveSaveDataHandleSequence()) {
        _B0->_146 = false;
        restoreUserFile();
        setNerve(GET_NERVE(FileSelector, FileSelectorNrvFileConfirm));
    }
}

void FileSelector::exeFileConfirm() {
    if (MR::isFirstStep(this)) {
        mOperationButton->appear();
        mBackButton->appear();

        if (MR::isDead(mFileInfo)) {
            setFileInfo(_B0->_140);
            mFileInfo->appear();
        }

        mFileInfo->slide();
        fn_8024D010(_B0);
    }

    if (MR::isStep(this, 90)) {
        _B0->validateRotate();
    }

    if (checkSelectedBackButton()) {
        _B0->validateRotate();
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
        disappearAllLayout();
        MR::stopSystemSE("SE_SY_LV_TITLE_BGM", 30);
        MR::stopSystemSE("SE_SY_LV_TITLE_BGM_RHYTHM", 30);
        _F8 = false;
    }

    if (isHiddenAllLayout()) {
        setNerve(GET_NERVE(FileSelector, FileSelectorNrvDemo));
    }
}

void FileSelector::exeDemo() {
    if (MR::isFirstStep(this)) {
        fn_804D5E70(_B0->_140);
        MR::stopSystemSE("SE_SY_LV_TITLE_BGM", 30);
        MR::stopSystemSE("SE_SY_LV_TITLE_BGM_RHYTHM", 30);
        _F8 = false;
    }

    FileSelectIconID iconId;
    getIconId(&iconId, _B0->_140);
    wchar_t name[12];

    if (iconId.isMii()) {
        FileSelectFunc::copyMiiName(reinterpret_cast< u16* >(name), iconId);
    } else {
        MR::copyString(name, MR::getGameMessageDirect(cMarioNameMessageID), 12);
    }

    GameDataFunction::setUserName(name);
    fn_804D3DF0();
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
        GameSequenceFunction::startCreateUserFileSequence(_B0->_140);
    }

    if (GameSequenceFunction::isActiveSaveDataHandleSequence()) {
        return;
    }

    if (GameSequenceFunction::isSuccessSaveDataHandleSequence()) {
        restoreUserFile();
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

            _B0->copyIconID(&iconId);
            mMiiSelect->prohibitIcon(iconId);
        } else {
            mIsMiiSelectStartFirst = true;

            mMiiSelect->admitIcon();
        }

        mMiiSelect->validateAllSpecialMii();

        if (!mUserFile[_B0->_140 - 1].isView120StarsEnding()) {
            mMiiSelect->invalidateSpecialMii(static_cast< FileSelectIconID::EFellowID >(5));
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
        if (fn_804A2BB0(mSysInfoWindow)) {
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
        storeSetMiiIdUserFile(_B0->_140, iconId);

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

        s32 id = _B0->_140;
        FileSelectIconID iconId = FileSelectIconID();

        mMiiSelect->getSelectedID(&iconId);
        _B0->change(iconId, _C8[id - 1]);
        setFileInfo(id);
    }

    if (!_B0->isExist()) {
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
        isAllOtherFileNew(_B0);
        GameSequenceFunction::startDeleteUserFileSequence(_B0->_140);
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
        _B0->format();
    }

    if (_B0->isNew()) {
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
