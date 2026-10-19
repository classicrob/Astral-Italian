#include "Game/System/SaveDataHandleSequence.hpp"

#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Screen/SaveIcon.hpp"
#include "Game/Screen/SysInfoWindow.hpp"
#include "Game/System/NANDErrorSequence.hpp"
#include "Game/System/NANDManager.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/SaveDataHandler.hpp"
#include "Game/System/SysConfigFile.hpp"
#include "Game/System/UserFile.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <JSystem/JKernel/JKRHeap.hpp>
#include <cstdio>

namespace {
    NEW_NERVE(SaveDataHandleSequenceNoOperation, SaveDataHandleSequence, NoOperation);
    NEW_NERVE(SaveDataHandleSequenceCheckEnableToCreate, SaveDataHandleSequence, CheckEnableToCreate);
    NEW_NERVE(SaveDataHandleSequenceSaveConfirm, SaveDataHandleSequence, SaveConfirm);
    NEW_NERVE(SaveDataHandleSequenceSave, SaveDataHandleSequence, Save);
    NEW_NERVE(SaveDataHandleSequenceSaveDoneKeyWait, SaveDataHandleSequence, SaveDoneKeyWait);
    NEW_NERVE(SaveDataHandleSequenceSaveAllWithoutKeyWait, SaveDataHandleSequence, SaveAllWithoutKeyWait);
    NEW_NERVE(SaveDataHandleSequenceWaitSaveData, SaveDataHandleSequence, WaitSaveData);
    NEW_NERVE(SaveDataHandleSequencePreLoadBanner, SaveDataHandleSequence, PreLoadBanner);
    NEW_NERVE(SaveDataHandleSequencePreLoad, SaveDataHandleSequence, PreLoad);
    NEW_NERVE(SaveDataHandleSequencePreLoadDone, SaveDataHandleSequence, PreLoadDone);
    NEW_NERVE(SaveDataHandleSequenceNoSaveConfirmRemind, SaveDataHandleSequence, NoSaveConfirmRemind);
    NEW_NERVE(SaveDataHandleSequenceErrorHandling, SaveDataHandleSequence, ErrorHandling);
};  // namespace

#ifdef GALAXY_NATIVE
namespace {
    const char* getSaveSequenceNerveName(const SaveDataHandleSequence* pSequence) {
        static const struct {
            const Nerve* mNerve;
            const char* mName;
        } cNerves[] = {
            {GET_NERVE_ANON(SaveDataHandleSequenceNoOperation), "NoOperation"},
            {GET_NERVE_ANON(SaveDataHandleSequenceCheckEnableToCreate), "CheckEnableToCreate"},
            {GET_NERVE_ANON(SaveDataHandleSequenceSaveConfirm), "SaveConfirm"},
            {GET_NERVE_ANON(SaveDataHandleSequenceSave), "Save"},
            {GET_NERVE_ANON(SaveDataHandleSequenceSaveDoneKeyWait), "SaveDoneKeyWait"},
            {GET_NERVE_ANON(SaveDataHandleSequenceSaveAllWithoutKeyWait), "SaveAllWithoutKeyWait"},
            {GET_NERVE_ANON(SaveDataHandleSequenceWaitSaveData), "WaitSaveData"},
            {GET_NERVE_ANON(SaveDataHandleSequencePreLoadBanner), "PreLoadBanner"},
            {GET_NERVE_ANON(SaveDataHandleSequencePreLoad), "PreLoad"},
            {GET_NERVE_ANON(SaveDataHandleSequencePreLoadDone), "PreLoadDone"},
            {GET_NERVE_ANON(SaveDataHandleSequenceNoSaveConfirmRemind), "NoSaveConfirmRemind"},
            {GET_NERVE_ANON(SaveDataHandleSequenceErrorHandling), "ErrorHandling"},
        };

        for (u32 i = 0; i < sizeof(cNerves) / sizeof(cNerves[0]); i++) {
            if (pSequence->isNerve(cNerves[i].mNerve)) {
                return cNerves[i].mName;
            }
        }

        return "?";
    }
}  // namespace
#endif

SaveDataHandleSequence::SaveDataHandleSequence()
    : NerveExecutor("セーブ/ロード"), mSysConfigFile(), mCurrentUserFile(), _10(1), mBackupUserFile(), mSaveDataHandler(), mNANDErrorSequence(),
      mSysInfoWindowConfirm(), mSysInfoWindowSave(), _24(), _2C_new(), mIsConfirmRemind(), mIsSaveAndQuitMsg(), _2A(), _2B(), _2C(), _32(),
      mWorkUserFile(), mNerveForError(), mTempBuffer(), mSaveIcon() {
    mTempBuffer = new (32) u8[SaveDataHandler::getEnoughtTempBufferSize()];

    initNerve(GET_NERVE_ANON(SaveDataHandleSequenceNoOperation));
}

void SaveDataHandleSequence::initAfterResourceLoaded() {
    mSysConfigFile = new SysConfigFile();
    mCurrentUserFile = new UserFile();
    mBackupUserFile = new UserFile();
    mSaveDataHandler = new SaveDataHandler(mSysConfigFile, mCurrentUserFile);

    mSysInfoWindowConfirm = MR::createSysInfoWindowExecuteWithChildren();
    mSysInfoWindowConfirm->kill();

    mSysInfoWindowSave = MR::createSysInfoWindowMiniExecuteWithChildren();
    mSysInfoWindowSave->kill();

    mNANDErrorSequence = new NANDErrorSequence(this, mSysInfoWindowConfirm, mSysInfoWindowSave);

    mSaveIcon = new SaveIcon(mSysInfoWindowSave);
    mSaveIcon->kill();
}





void SaveDataHandleSequence::update() {
    updateNerve();

    if (mSaveDataHandler != nullptr) {
        mSaveDataHandler->update();
    }

    if (mSysInfoWindowConfirm != nullptr) {
        mSysInfoWindowConfirm->movement();
        mSysInfoWindowConfirm->calcAnim();
    }

    if (mSysInfoWindowSave != nullptr) {
        mSysInfoWindowSave->movement();
        mSysInfoWindowSave->calcAnim();
        mSaveIcon->movement();
        mSaveIcon->calcAnim();
    }

    if (isEnablePointer()) {
        MR::requestStarPointerModeSaveLoad(this);
    }
}

void SaveDataHandleSequence::draw() const {
    if (mSysInfoWindowConfirm != nullptr) {
        mSysInfoWindowConfirm->draw();
    }

    if (mSysInfoWindowSave != nullptr) {
        mSysInfoWindowSave->draw();
        mSaveIcon->draw();
    }
}

void SaveDataHandleSequence::startPreLoad() {
    setNerve(GET_NERVE_ANON(SaveDataHandleSequencePreLoadBanner));
}

void SaveDataHandleSequence::startCreateUserFile(int index) {
    UserFile* pUserFile = mCurrentUserFile;
    restoreUserFileConfigData(pUserFile, index);
    restoreUserFileGameData(pUserFile, index);
    mCurrentUserFile->resetAllData();
    mCurrentUserFile->setCreated();
    mCurrentUserFile->updateLastModified();
    mSaveDataHandler->initializeUserFileMemory(index, mCurrentUserFile);
    setNerve(GET_NERVE_ANON(SaveDataHandleSequenceSaveAllWithoutKeyWait));
}

void SaveDataHandleSequence::startDeleteUserFile(int index) {
    UserFile* pUserFile = mCurrentUserFile;
    restoreUserFileConfigData(pUserFile, index);
    restoreUserFileGameData(pUserFile, index);
    mCurrentUserFile->resetAllData();
    mSaveDataHandler->initializeUserFileMemory(index, mCurrentUserFile);
    setNerve(GET_NERVE_ANON(SaveDataHandleSequenceSaveAllWithoutKeyWait));
}



void SaveDataHandleSequence::startSaveGameData(bool waitKey, bool confirm, bool silent) {
    _2C_new = waitKey;
    mIsConfirmRemind = confirm;
    mIsSaveAndQuitMsg = silent;
    mWorkUserFile = mCurrentUserFile;
    if (confirm) {
        setNerve(GET_NERVE_ANON(SaveDataHandleSequenceSaveConfirm));
    } else {
        mWorkUserFile->setCreated();
        mWorkUserFile->updateLastModified();
        setNerve(GET_NERVE_ANON(SaveDataHandleSequenceSave));
    }
}

void SaveDataHandleSequence::startWaitSaveData() {
    setNerve(GET_NERVE_ANON(SaveDataHandleSequenceWaitSaveData));
}

void SaveDataHandleSequence::startLoadUserFile(int userFileIndex) {
    UserFile* userFile = mCurrentUserFile;
    _10 = userFileIndex;
    restoreUserFileConfigData(userFile, userFileIndex);
    restoreUserFileGameData(userFile, userFileIndex);

    _24 = 2;

    setNerve(GET_NERVE_ANON(SaveDataHandleSequenceNoOperation));
}

void SaveDataHandleSequence::startReloadUserFile() {
    startLoadUserFile(_10);
}

void SaveDataHandleSequence::storeMiiOrIconId(int userFileIndex, const void* pMiiId, const u32* pIconId) {
    restoreUserFile(mCurrentUserFile, userFileIndex);
    mCurrentUserFile->setMiiOrIconId(pMiiId, pIconId);
    mCurrentUserFile->updateLastModified();
    mSaveDataHandler->storeUserFile(mCurrentUserFile);
}

void SaveDataHandleSequence::exeSave() {
    if (MR::isFirstStep(this)) {
        prepareSaveAll(true);
        mSaveDataHandler->storeUserFile(mWorkUserFile);
    }

    if (tryNoSave()) {
        return;
    }

    if (_2A) {
        bool isErr = true;
        executeSaveFinish(&isErr, GET_NERVE_ANON(SaveDataHandleSequenceSaveConfirm));
        return;
    }

    if (_2C_new) {
        bool isErr = false;
        if (trySave(&isErr, GET_NERVE_ANON(SaveDataHandleSequenceSaveConfirm)) && !isErr) {
            setNerve(GET_NERVE_ANON(SaveDataHandleSequenceSaveDoneKeyWait));
        }
    } else {
        bool isErr = false;
        if (trySave(&isErr, GET_NERVE_ANON(SaveDataHandleSequenceSaveConfirm)) && !isErr) {
            _24 = 2;
            setNerve(GET_NERVE_ANON(SaveDataHandleSequenceNoOperation));
        }
    }
}

void SaveDataHandleSequence::startSaveAll() {
    setNerve(GET_NERVE_ANON(SaveDataHandleSequenceSaveAllWithoutKeyWait));
}





bool SaveDataHandleSequence::tryNANDErrorSequence(s32 code) {
    if (mNANDErrorSequence->tryNoRecoverErroSequence(code)) {
        mNerveForError = GET_NERVE_ANON(SaveDataHandleSequenceNoOperation);

        setNerve(GET_NERVE_ANON(SaveDataHandleSequenceErrorHandling));

        return true;
    }

    return false;
}

bool SaveDataHandleSequence::isActive() const {
    return !isNerve(GET_NERVE_ANON(SaveDataHandleSequenceNoOperation));
}

bool SaveDataHandleSequence::isPermitToReset() const {
    if (mNANDErrorSequence != nullptr) {
        return mNANDErrorSequence->isPermitToReset();
    }

    return true;
}

void SaveDataHandleSequence::prepareReset() {
    if (mNANDErrorSequence != nullptr) {
        mNANDErrorSequence->prepareReset();
    }
}

bool SaveDataHandleSequence::isPreparedReset() const {
    if (mNANDErrorSequence != nullptr && !mNANDErrorSequence->isPreparedReset()) {
        return false;
    }

    if (isNerve(GET_NERVE_ANON(SaveDataHandleSequenceNoOperation)) || isNerve(GET_NERVE_ANON(SaveDataHandleSequenceSaveConfirm)) ||
        isNerve(GET_NERVE_ANON(SaveDataHandleSequenceSaveDoneKeyWait)) || isNerve(GET_NERVE_ANON(SaveDataHandleSequenceNoSaveConfirmRemind))) {
        return true;
    }

    if (isNerve(GET_NERVE_ANON(SaveDataHandleSequencePreLoadBanner)) || isNerve(GET_NERVE_ANON(SaveDataHandleSequencePreLoad)) ||
        isNerve(GET_NERVE_ANON(SaveDataHandleSequencePreLoadDone)) || isNerve(GET_NERVE_ANON(SaveDataHandleSequenceSave)) ||
        isNerve(GET_NERVE_ANON(SaveDataHandleSequenceSaveAllWithoutKeyWait)) ||
        isNerve(GET_NERVE_ANON(SaveDataHandleSequenceWaitSaveData))) {
        return false;
    }

    if (isNerve(GET_NERVE_ANON(SaveDataHandleSequenceCheckEnableToCreate)) || isNerve(GET_NERVE_ANON(SaveDataHandleSequenceErrorHandling))) {
        return mNANDErrorSequence->isPreparedReset();
    }

    return false;
}

void SaveDataHandleSequence::restoreFromReset() {
    mNANDErrorSequence->restoreFromReset();

    if (!MR::isDead(mSysInfoWindowConfirm)) {
        mSysInfoWindowConfirm->forceKill();
    }

    if (!MR::isDead(mSysInfoWindowSave)) {
        mSysInfoWindowSave->forceKill();
    }

    mSaveIcon->kill();
    _24 = 3;

    setNerve(GET_NERVE_ANON(SaveDataHandleSequenceNoOperation));
}

bool SaveDataHandleSequence::isInitializedGameDataHolder() const {
    return mCurrentUserFile != nullptr;
}

void SaveDataHandleSequence::restoreUserFile(UserFile* pUserFile, int index) {
    restoreUserFileConfigData(pUserFile, index);
    restoreUserFileGameData(pUserFile, index);
}

void SaveDataHandleSequence::restoreUserFile(UserFile* pUserFile, int index, bool isPlayerMario) {
    restoreUserFileConfigData(pUserFile, index);
    restoreUserFileGameData(pUserFile, index);
    pUserFile->setLastLoadedMario(isPlayerMario);
}

void SaveDataHandleSequence::backupCurrentUserFile() {
    mCurrentUserFile->makeGameDataBinary(mTempBuffer, SaveDataHandler::getEnoughtTempBufferSize());
    mBackupUserFile->loadFromGameDataBinary(mCurrentUserFile->getGameDataName(), mTempBuffer, SaveDataHandler::getEnoughtTempBufferSize());
    mCurrentUserFile->makeConfigDataBinary(mTempBuffer, SaveDataHandler::getEnoughtTempBufferSize());
    mBackupUserFile->loadFromConfigDataBinary(mCurrentUserFile->getConfigDataName(), mTempBuffer, SaveDataHandler::getEnoughtTempBufferSize());
}

void SaveDataHandleSequence::backupCurrentGameData() {
    mBackupUserFile->makeGameDataBinary(mTempBuffer, SaveDataHandler::getEnoughtTempBufferSize());
    mCurrentUserFile->loadFromGameDataBinary(mBackupUserFile->getGameDataName(), mTempBuffer, SaveDataHandler::getEnoughtTempBufferSize());
}

u8* SaveDataHandleSequence::clearTempBuffer(s32) {
    MR::zeroMemory(mTempBuffer, SaveDataHandler::getEnoughtTempBufferSize());
    return mTempBuffer;
}

void SaveDataHandleSequence::loadTempBufferAsGameData() {
    u32 size = SaveDataHandler::getEnoughtTempBufferSize();
    const char* name = mCurrentUserFile->getGameDataName();
    mCurrentUserFile->loadFromGameDataBinary(name, mTempBuffer, size);
}

void SaveDataHandleSequence::exeNoOperation() {
    _2C_new = false;
    mIsConfirmRemind = false;
    mIsSaveAndQuitMsg = false;
    _2B = false;
}

void SaveDataHandleSequence::exeCheckEnableToCreate() {
    if (MR::isFirstStep(this)) {
        mNANDErrorSequence->startCheckEnableToCreate();
    }

    mNANDErrorSequence->update();

    if (!mNANDErrorSequence->mIsDead) {
        return;
    }

    syncNoSaveFlagsFromErrorSequence();
    _24 = 2;
    setNerve(GET_NERVE_ANON(SaveDataHandleSequenceNoOperation));
}

void SaveDataHandleSequence::exeSaveConfirm() {
    bool b = _2C || _2B;

    if (b) {
        mWorkUserFile->setCreated();
        mWorkUserFile->updateLastModified();
        setNerve(GET_NERVE_ANON(SaveDataHandleSequenceSave));
    } else {
        bool isSelectedYes = false;
        const char* const pSystemMessageId = mIsConfirmRemind ? "System_Save07" : "System_Save00";

        if (!tryConfirm(pSystemMessageId, &isSelectedYes)) {
            return;
        }

        if (isSelectedYes) {
            mWorkUserFile->setCreated();
            mWorkUserFile->updateLastModified();
            setNerve(GET_NERVE_ANON(SaveDataHandleSequenceSave));
        } else if (mIsConfirmRemind) {
            setNerve(GET_NERVE_ANON(SaveDataHandleSequenceNoOperation));
            _24 = 3;
        } else {
            setNerve(GET_NERVE_ANON(SaveDataHandleSequenceNoSaveConfirmRemind));
        }
    }
}



void SaveDataHandleSequence::exeSaveDoneKeyWait() {
    if (!tryProcessDoneKeyWait("System_Save02")) {
        return;
    }

    _24 = 2;

    setNerve(GET_NERVE_ANON(SaveDataHandleSequenceNoOperation));
}

void SaveDataHandleSequence::exeSaveAllWithoutKeyWait() {
    if (MR::isFirstStep(this)) {
        prepareSaveAll(false);
    }

    if (!tryNoSave()) {
        if (_2A) {
            bool isErr = true;
            executeSaveFinish(&isErr, GET_NERVE_ANON(SaveDataHandleSequenceSaveAllWithoutKeyWait));
            return;
        }

        bool isErr = false;
        if (trySave(&isErr, GET_NERVE_ANON(SaveDataHandleSequenceSaveAllWithoutKeyWait)) && !isErr) {
            _24 = 2;
            setNerve(GET_NERVE_ANON(SaveDataHandleSequenceNoOperation));
        }
    }
}




void SaveDataHandleSequence::exeWaitSaveData() {
    if (MR::isFirstStep(this)) {
        prepareSaveAll(true);
    }
    if (tryNoSave()) {
        return;
    }
    if (_2A) {
        bool error = true;
        executeSaveFinish(&error, GET_NERVE_ANON(SaveDataHandleSequenceSaveAllWithoutKeyWait));
        return;
    }
    bool error = false;
    if (trySave(&error, GET_NERVE_ANON(SaveDataHandleSequenceWaitSaveData)) && !error) {
        _24 = 2;
        setNerve(GET_NERVE_ANON(SaveDataHandleSequenceNoOperation));
    }
}

void SaveDataHandleSequence::exePreLoadBanner() {
    if (MR::isFirstStep(this)) {
        mSaveDataHandler->requestLoadBanner();
        _32 = true;
    }
    if (mSaveDataHandler->isDone()) {
        NANDResultCode code = mSaveDataHandler->getLastResultCode();
        if (code.isSuccess() || code.isSaveDataCorrupted()) {
            setNerve(GET_NERVE_ANON(SaveDataHandleSequencePreLoad));
        } else if (code.isNoExistFile()) {
            _32 = false;
            setNerve(GET_NERVE_ANON(SaveDataHandleSequencePreLoad));
        } else {
            if (tryNANDErrorSequence(code.getCode())) {
                return;
            }
        }
    }
}

void SaveDataHandleSequence::exePreLoad() {
    if (MR::isFirstStep(this)) {
        mSaveDataHandler->requestLoadGameData();
    }
    if (mSaveDataHandler->isDone()) {
        NANDResultCode code = mSaveDataHandler->getLastResultCode();
        if (code.isSuccess()) {
            if (_32) {
                setNerve(GET_NERVE_ANON(SaveDataHandleSequencePreLoadDone));
            } else {
                mNerveForError = GET_NERVE_ANON(SaveDataHandleSequenceNoOperation);
                mNANDErrorSequence->startRemoveFile();
                setNerve(GET_NERVE_ANON(SaveDataHandleSequenceErrorHandling));
            }
        } else if (code.isNoExistFile()) {
            setNerve(GET_NERVE_ANON(SaveDataHandleSequenceCheckEnableToCreate));
        } else if (code.isSaveDataCorrupted()) {
            mNerveForError = GET_NERVE_ANON(SaveDataHandleSequenceNoOperation);
            mNANDErrorSequence->startRemoveFile();
            setNerve(GET_NERVE_ANON(SaveDataHandleSequenceErrorHandling));
        } else {
            if (tryNANDErrorSequence(code.getCode())) {
                return;
            }
        }
    }
}

void SaveDataHandleSequence::exePreLoadDone() {
    if (!mSaveDataHandler->requestVerifyAfterLoadGameDataFile()) {
        mNerveForError = GET_NERVE_ANON(SaveDataHandleSequenceNoOperation);
        mNANDErrorSequence->startRemoveFile();

        setNerve(GET_NERVE_ANON(SaveDataHandleSequenceErrorHandling));
    } else {
        restoreSysConfigFile();
        _24 = 2;

        setNerve(GET_NERVE_ANON(SaveDataHandleSequenceNoOperation));
    }
}

void SaveDataHandleSequence::exeNoSaveConfirmRemind() {
    bool isSelectedYes = false;

    if (!tryConfirm("System_ConfirmNoSave", &isSelectedYes)) {
        return;
    }

    if (isSelectedYes) {
        _24 = 2;

        setNerve(GET_NERVE_ANON(SaveDataHandleSequenceNoOperation));
    } else {
        setNerve(GET_NERVE_ANON(SaveDataHandleSequenceSaveConfirm));
    }
}

void SaveDataHandleSequence::exeErrorHandling() {
    mNANDErrorSequence->update();

    if (!mNANDErrorSequence->mIsDead) {
        return;
    }

    syncNoSaveFlagsFromErrorSequence();
    setNerve(mNerveForError);

    mNerveForError = nullptr;
}


SysConfigFile* SaveDataHandleSequence::getSysConfigFile() {
    return mSysConfigFile;
}

UserFile* SaveDataHandleSequence::getCurrentUserFile() {
    return mCurrentUserFile;
}


void SaveDataHandleSequence::restoreUserFileConfigData(UserFile* pUserFile, int index) {
    char dataName[16];
    snprintf(dataName, sizeof(dataName), "config%1d", index);

    mSaveDataHandler->restoreGameDataFile(dataName, mTempBuffer, SaveDataHandler::getEnoughtTempBufferSize());
    pUserFile->loadFromConfigDataBinary(dataName, mTempBuffer, SaveDataHandler::getEnoughtTempBufferSize());
}

void SaveDataHandleSequence::restoreUserFileGameData(UserFile* pUserFile, int index) {
    char dataName[16];
    snprintf(dataName, sizeof(dataName), "%s%1d", "user", index);

    mSaveDataHandler->restoreGameDataFile(dataName, mTempBuffer, SaveDataHandler::getEnoughtTempBufferSize());
    pUserFile->loadFromGameDataBinary(dataName, mTempBuffer, SaveDataHandler::getEnoughtTempBufferSize());
}

void SaveDataHandleSequence::restoreSysConfigFile() {
    const char* pName = "sysconf";
    mSaveDataHandler->restoreGameDataFile(pName, mTempBuffer, SaveDataHandler::getEnoughtTempBufferSize());
    mSysConfigFile->loadFromDataBinary(mTempBuffer, SaveDataHandler::getEnoughtTempBufferSize());
}

void SaveDataHandleSequence::prepareSaveAll(bool isWithBeforeSave) {
    if (isWithBeforeSave) {
        GameDataFunction::storeSharedPlayerLeft();
    }

    mSysConfigFile->mChunk->backupCounters();
    mSaveDataHandler->storeSysConfigFile(mSysConfigFile);
}

bool SaveDataHandleSequence::trySave(bool* pIsErr, const Nerve* pNerve) {
    if (MR::isFirstStep(this)) {
        mSaveDataHandler->requestSaveSaveData();
    }

    if (mSaveDataHandler->isDone()) {
        executeSaveFinish(pIsErr, pNerve);
        return true;
    }

    return false;
}



bool SaveDataHandleSequence::tryConfirm(const char* pSystemMessageId, bool* pIsSelectedYes) {
    if (MR::isFirstStep(this)) {
        _24 = 1;

        mSysInfoWindowConfirm->appear(pSystemMessageId, SysInfoWindow::Type_YesNo, SysInfoWindow::TextPos_Center, SysInfoWindow::MessageType_System);
        MR::startSystemSE("SE_SY_SAVE_CONFIRM_INFO");
    }

    if (MR::isDead(mSysInfoWindowConfirm)) {
        *pIsSelectedYes = mSysInfoWindowConfirm->isSelectedYes();

        return true;
    }

    return false;
}

bool SaveDataHandleSequence::tryProcessDoneKeyWait(const char* pSystemMessageId) {
    if (MR::isFirstStep(this)) {
        mSysInfoWindowSave->appear(pSystemMessageId, SysInfoWindow::Type_Key, SysInfoWindow::TextPos_Center, SysInfoWindow::MessageType_System);
    }

    if (MR::isDead(mSysInfoWindowSave)) {
        return true;
    }

    return false;
}

bool SaveDataHandleSequence::tryNoSave() {
    bool b = _2C || _2B;

    if (b) {
        setNerve(GET_NERVE_ANON(SaveDataHandleSequenceNoOperation));

        _24 = 2;

        return true;
    }

    return false;
}

bool SaveDataHandleSequence::isEnablePointer() const {
    if (isNerve(GET_NERVE_ANON(SaveDataHandleSequenceNoSaveConfirmRemind)) || isNerve(GET_NERVE_ANON(SaveDataHandleSequenceErrorHandling))) {
        return true;
    }

    if (isNerve(GET_NERVE_ANON(SaveDataHandleSequenceSaveConfirm))) {
        bool isRemind = false;
        if (_2C || _2B) {
            isRemind = true;
        }

        if (!isRemind) {
            return true;
        }
    }

    if (isNerve(GET_NERVE_ANON(SaveDataHandleSequenceCheckEnableToCreate)) && mNANDErrorSequence->isEnablePointer()) {
        return true;
    }

    return false;
}

void SaveDataHandleSequence::executeSaveFinish(bool* pIsErr, const Nerve* pNerveForError) {
    NANDResultCode resultCode = mSaveDataHandler->getLastResultCode();

    if (_2A) {
        resultCode = NANDResultCode(NAND_RESULT_AUTHENTICATION);
    }

    if (resultCode.isSuccess()) {
        if (!mIsSaveAndQuitMsg) {
            MR::startSystemSE("SE_SY_SAVE_SUCCESS");
        }

        *pIsErr = false;
        _2A = false;
        return;
    }

    *pIsErr = true;

    if (!mIsSaveAndQuitMsg) {
        MR::startSystemSE("SE_SY_SAVE_FAILURE");
    }

    if (resultCode.isSaveDataCorrupted()) {
        mNerveForError = pNerveForError;

        mNANDErrorSequence->startRemoveFile();
        setNerve(GET_NERVE_ANON(SaveDataHandleSequenceErrorHandling));
        return;
    }

    if (!tryNANDErrorSequence(resultCode.getCode())) {
        mNerveForError = GET_NERVE_ANON(SaveDataHandleSequenceNoOperation);

        mNANDErrorSequence->startErrorToWiiMenu("NAND_12");
        setNerve(GET_NERVE_ANON(SaveDataHandleSequenceErrorHandling));
    }
}

void SaveDataHandleSequence::syncNoSaveFlagsFromErrorSequence() {
    _2C = mNANDErrorSequence->isResultNoSave();

    bool isResultFileCorrupted = mNANDErrorSequence->isResultFileCorrupted();

    _2A = isResultFileCorrupted;
    _2B = isResultFileCorrupted;
}
