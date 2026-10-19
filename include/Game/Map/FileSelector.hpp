#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Map/FileSelectIconID.hpp"

struct RFLCreateID;

namespace nw4r {
    namespace ut {
        class ResFont;
    };  // namespace ut
};  // namespace nw4r

class BackButton;
class FileSelectButton;
class FileSelectCameraController;
class FileSelectEffect;
class FileSelectInfo;
class FileSelectItem;
class FileSelectSky;
class Manual2P;
class MiiConfirmIcon;
class MiiSelect;
class TitleSequenceProduct;
class UserFile;
class SysInfoWindow;

template < typename T >
class DeriveActorGroup;

class FileSelector : public LiveActor {
public:
    FileSelector(const char* pName);

    virtual ~FileSelector();
    virtual void init(const JMapInfoIter& rIter);
    virtual void appear();
    virtual void kill();
    virtual void control();
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void callbackStart();
    void callbackMii();
    void callbackDelete();
    void callbackManual();
    void notifyItem(FileSelectItem* pItem, s32 msg);
    void initUserFileArray();
    void createCameraController();
    void createFileItems();
    void createOperationButton();
    void createBackButton();
    void createSysInfoWindow();
    void createFileInfo();
    void createTitle();
    void createMiiSelect();
    void createMiiConfirmIcon();
    void createManual();
    void createSelectEffect();
    FileSelectItem* createFileItem(s32 id);
    void initChildObjs(const JMapInfoIter& rIter);
    void stopMovementForLayout();
    void restartMovementForLayout();
    void updateChildObjs();
    void appearAllItems();
    void disappearAllLayout();
    bool isHiddenAllLayout() const;
    void updateFileInfo();
    void appearAllIndex();
    void disappearAllIndex();
    void invalidateSelectAll();
    void validateSelectAll();
    void initUserFile();
    void restoreUserFile();
    void checkAllComplete();
    void onPoint(FileSelectItem* pItem);
    void onSelect(FileSelectItem* pItem);
    void clearPointing();
    void setFileInfo(s32 id);
    bool checkSelectedBackButton();
    void goToNearPoint();
    void calcBasePos(f32 offset);
    void initAllItems();
    void validateRotateAllItems();
    bool isAllOtherFileNew(FileSelectItem* pItem);
    bool isUserFileComplete(s32 id) const;
    FileSelectIconID::EFellowID getUserFileFellowID(s32 id) const;
    bool isUserFileMiiIdValid(s32 id) const;
    u16 getUserFileMiiIndex(s32 id) const;
    bool isUserFileCorrupted(s32 id) const;
    bool isUserFileAppearLuigi(s32 id) const;
    bool isUserFileLuigi(s32 id) const;
    void setUserFileMario(s32 id, bool isMario) NO_INLINE;
    void storeSetMiiIdUserFile(s32 id, const FileSelectIconID& rIconId);
    void getMiiId(RFLCreateID* pCreateId, const FileSelectIconID& rIconId) const;
    void getIconId(FileSelectIconID* pIconId, s32 id) const;
    s32 getMissCount(s32 id) const;
    s32 getGrandStarNum(s32 id) const;
    void playSelectedME();
    void updateBgm();
    void updatePointingGuidance();
    void updatePointingGuidanceSelect();
    void exeTitle();
    void exeTitleEnd();
    void exeRFLError();
    void exeRFLWait();
    void exeRFLWaitEnd();
    void exeFileSelectStart();
    void exeFileSelect();
    void exeFileConfirmStart();
    void exeFileConfirmMiiDeleteWarningStart();
    void exeFileConfirmMiiDeleteWarning();
    void exeFileConfirmMiiDeleteSave();
    void exeFileConfirm();
    void exeDemoStartWait();
    void exeDemo();
    void exeCreateConfirmStart();
    void exeCreateConfirm();
    void exeCreate();
    void exeMiiTip();
    void exeMiiSelectStart();
    void exeMiiSelect();
    void exeMiiCancel();
    void exeMiiConfirmWait();
    void exeMiiConfirm();
    void exeMiiCreateWait();
    void exeMiiCreateDemo();
    void exeMiiCaution();
    void exeMiiInfoStart();
    void exeMiiInfo();
    void exeDeleteConfirmStart();
    void exeDeleteConfirm();
    void exeDelete();
    void exeDeleteDemo();
    void exeManualStart();
    void exeManual();

    /* 0x90 */ FileSelectCameraController* mCameraController;
    /* 0x94 */ FileSelectSky* mSky;
    /* 0x98 */ DeriveActorGroup< FileSelectItem >* mItems;
    /* 0x9C */ TVec3f* mItemPositions;
    /* 0xA0 */ FileSelectButton* mOperationButton;
    /* 0xA4 */ BackButton* mBackButton;
    /* 0xA8 */ SysInfoWindow* mSysInfoWindow;
    /* 0xAC */ SysInfoWindow* mSysInfoWindowMini;
    /* 0xB0 */ FileSelectItem* mSelectedItem;
    /* 0xB4 */ FileSelectItem* mUnusedItem;
    /* 0xB8 */ FileSelectItem* mPointedItem;
    /* 0xBC */ FileSelectItem* mPointingCandidate;
    /* 0xC0 */ FileSelectInfo* mFileInfo;
    /* 0xC4 */ UserFile* mUserFile;
    /* 0xC8 */ bool* mCompleteFlags;
    /* 0xCC */ TitleSequenceProduct* mTitle;
    /* 0xD0 */ MiiSelect* mMiiSelect;
    /* 0xD4 */ MiiConfirmIcon* mMiiConfirmIcon;
    /* 0xD8 */ RFLCreateID* mMiiId;
    /* 0xDC */ Manual2P* mManual;
    /* 0xE0 */ bool mIsMiiSelectStartFirst;
    /* 0xE4 */ s32 mBgmState;
    /* 0xE8 */ FileSelectEffect* mSelectEffect;
    /* 0xEC */ bool mIsChildMovementStopped;
    /* 0xF0 */ LiveActor** mChildActors;
    /* 0xF4 */ s32 mChildActorCount;
    /* 0xF8 */ bool mIsTitleBgmRhythmEnabled;
    /* 0xFC */ s32 mTitleBgmRhythmVolume;
    /* 0x100 */ s32 mTargetTitleBgmRhythmVolume;
    /* 0x104 */ s32 mTitleBgmRhythmFadeFrames;
};
