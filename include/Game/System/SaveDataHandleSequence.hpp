#pragma once

#include "Game/System/NerveExecutor.hpp"

class GameDataHolder;
class NANDErrorSequence;
class SaveDataHandler;
class SaveIcon;
class SysConfigFile;
class SysInfoWindow;
class UserFile;

namespace MR {
    class FunctorBase;
};  // namespace MR

class SaveDataHandleSequence : public NerveExecutor {
public:
    /// @brief Creates a new `SaveDataHandleSequence`.
    SaveDataHandleSequence();

    void initAfterResourceLoaded();
    void registerFunctorOnSaveSuccess(const MR::FunctorBase&);
    void registerFunctorJustBeforeSave(const MR::FunctorBase&);
    void update();
    void draw() const;
    void startPreLoad();
    void startCreateUserFile(int);
    void startDeleteUserFile(int);
    void startSave(bool, bool);
    void startSaveBackup(bool, bool);
    void startSaveAll();
    void startSaveTotalMailSize();
    void startLoad(int, bool);
    void storeMiiOrIconId(int, const void*, const u32*);
    void storeCopyUserFile(int, int);
    bool tryNANDErrorSequence(s32);
    bool isActive() const;
    bool isPermitToReset() const;
    void prepareReset();
    bool isPreparedReset() const;
    void restoreFromReset();
    bool isInitializedGameDataHolder() const;
    void restoreUserFile(UserFile*, int);
    void restoreUserFile(UserFile*, int, bool);
    void backupCurrentUserFile();
    void exeNoOperation();
    void exeCheckEnableToCreate();
    void exeSaveConfirm();
    void exeSave();
    void exeSaveWindowDisappear();
    void exeSaveDoneKeyWait();
    void exeSaveAllWithoutKeyWait();
    void exeSaveAllWithoutKeyWaitDisappear();
    void exeSaveAllWithoutWindow();
    void exePreLoad();
    void exePreLoadDone();
    void exeNoSaveConfirmRemind();
    void exeErrorHandling();
    GameDataHolder* getHolder();
    SysConfigFile* getSysConfigFile();
    UserFile* getCurrentUserFile();
    UserFile* getBackupUserFile();
    void restoreUserFileConfigData(UserFile*, int);
    void restoreUserFileGameData(UserFile*, int, bool);
    void restoreSysConfigFile(SysConfigFile*);
    bool trySave();
    bool trySaveWindowDisappear(bool*, const Nerve*);
    bool trySaveWithoutWindow(bool*, const Nerve*);
    bool tryConfirm(const char*, bool*);
    bool tryProcessDoneKeyWait(const char*);
    bool tryNoSave();
    bool isEnablePointer() const;
    bool executeSaveFinish(bool*, const Nerve*);
    void syncNoSaveFlagsFromErrorSequence();

    /* 0x08 */ SysConfigFile* mSysConfigFile;
    /* 0x0C */ UserFile* mCurrentUserFile;
    /* 0x10 */ s32 _10;  // SMG2: initialized to 1
    /* 0x14 */ UserFile* mBackupUserFile;
    /* 0x18 */ SaveDataHandler* mSaveDataHandler;
    /* 0x1C */ NANDErrorSequence* mNANDErrorSequence;
    /* 0x20 */ SysInfoWindow* mSysInfoWindowConfirm;
    /* 0x24 */ SysInfoWindow* mSysInfoWindowSave;
    /* 0x28 */ s32 _24;
    /* 0x2C */ bool _2C_new;  // SMG2
    /* 0x2D */ bool mIsConfirmRemind;
    /* 0x2E */ bool mIsSaveAndQuitMsg;
    /* 0x2F */ bool _2A;
    /* 0x30 */ bool _2B;
    /* 0x31 */ bool _2C;
    /* 0x32 */ bool _32;  // SMG2
    /* 0x34 */ UserFile* mWorkUserFile;
    /* 0x38 */ const Nerve* mNerveForError;
    /* 0x3C */ u8* mTempBuffer;
    /* 0x40 */ SaveIcon* mSaveIcon;
    // SMG1 only (no longer in SMG2's layout; kept so old call sites compile)
    MR::FunctorBase* mOnSaveSuccessFunc;
    MR::FunctorBase* mJustBeforeSaveFunc;
};
