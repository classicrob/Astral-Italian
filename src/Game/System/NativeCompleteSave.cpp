#ifdef GALAXY_NATIVE
#include "Game/System/NativeCompleteSave.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/GameDataGalaxyStorage.hpp"
#include "Game/System/GameDataHolder.hpp"
#include "Game/System/GameDataTicoFatStorage.hpp"
#include "Game/System/GameEventFlagTable.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
#include "Game/System/NANDManager.hpp"
#include "Game/System/SaveDataFileAccessor.hpp"
#include "Game/System/SaveDataHandleSequence.hpp"
#include "Game/System/SaveDataHandler.hpp"
#include "Game/System/UserFile.hpp"
#include <petari/complete_save_paths.hpp>
#include <petari/host_allocation.hpp>
#include <petari/platform/nand.hpp>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace NativeCompleteSave {
namespace {
enum class State { Idle, Saving, Reloading, Done };
State sState = State::Idle;
bool sNewFile = false;
int sPolls = 0;
using Records = std::map<std::string, std::vector<u8>>;
Records sStored;
std::filesystem::path sUser;

bool enabled(const char* name) {
    const char* value = std::getenv(name);
    return value && std::string(value) == "1";
}
bool creating() { return enabled("ASTRAL_DEV_COMPLETE_SAVE"); }
bool checking() { return creating() || enabled("ASTRAL_VERIFY_COMPLETE_SAVE"); }
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
void checkRoot() {
    const auto nand = PetariNative::Platform::NAND::hostPath("/");
    require(!nand.empty() && nand.filename() == "NAND", "NAND is not mounted at a user directory");
    sUser = nand.parent_path();
    require(PetariNative::CompleteSave::isIsolatedUser(sUser),
            "use the separate ~/Library/Application Support/AstralItalian-unlocked user directory");
}

Records records(SaveDataHandler& handler) {
    require(SaveDataHandler::isCorrectFileHeader(handler._18), "invalid in-memory game save header");
    SaveDataFileAccessor accessor(handler._18);
    const auto* header = accessor.getHeader();
    require(header->mFileSize <= 0x8000 && header->mUserFileInfoNum <= 32,
            "invalid game save directory extent");
    Records result;
    for (u32 i = 0; i < header->mUserFileInfoNum; ++i) {
        const auto* info = accessor.getFileInfo(i);
        const u32 end = i + 1 < header->mUserFileInfoNum ? accessor.getFileInfo(i + 1)->mOffset : header->mFileSize;
        require(std::memchr(info->mName, 0, sizeof(info->mName)) &&
                info->mOffset >= sizeof(SaveDataFileHeader) + header->mUserFileInfoNum * sizeof(SaveDataFileInfo) &&
                info->mOffset < end && end <= header->mFileSize, "invalid game save record extent");
        require(result.emplace(info->mName, std::vector<u8>(handler._18 + info->mOffset, handler._18 + end)).second,
                "duplicate game save record");
    }
    return result;
}

const char* const sProgressFlags[] = {
    "オープニング実行", "ノーマルエンディング実行", "ノーマルエンディング後デモ",
    "スター120個エンディング実行", "スター120個エンディング後デモ", "最終ギャラクシー出現",
    "グリーンスター出現デモ", "グランドギャラクシーマップ初プレイ", "クッパ最終戦直前デモ",
    "でしゃばりルイージ出現開始", "ルイージプレイ済", "コメット解説", "コメットメダル解説",
};

void progressFlags(bool apply) {
    for (const char* flag : sProgressFlags) {
        require(GameEventFlagTable::isExist(flag), "unknown SMG2 progression flag");
        if (apply) GameDataFunction::onGameEventFlag(flag);
        require(GameDataFunction::isOnGameEventFlag(flag), "progression flag failed readback");
    }
    for (int world = 1; world <= 7; ++world) {
        char name[64];
        snprintf(name, sizeof(name), "ワールド%d初プレイ", world);
        require(GameEventFlagTable::isExist(name), "unknown SMG2 world flag");
        if (apply) GameDataFunction::onGameEventFlag(name);
        require(GameDataFunction::isOnGameEventFlag(name), "world did not unlock");
    }
    for (int i = 0; i < GameEventFlagTable::getTableSize(); ++i) {
        const char* name = GameEventFlagTable::getFlag(i)->mName;
        if (std::string(name).rfind("IsOpenScenario", 0) == 0) {
            if (apply) GameDataFunction::onGameEventFlag(name);
            require(GameDataFunction::isOnGameEventFlag(name), "scenario did not unlock");
        }
    }
}

int stars(GameDataHolder& holder, bool apply) {
    auto* all = holder.mAllGalaxyStorage;
    require(all != nullptr, "game galaxy save storage unavailable");
    int count = 0;
    for (int i = 0; i < all->getGalaxyNum(); ++i) {
        auto* galaxy = all->getGalaxyStorage(i);
        require(galaxy && galaxy->mGalaxyName && galaxy->mScenarioAccessorNum > 0 &&
                galaxy->mScenarioAccessorNum <= 32 && galaxy->mScenarioAccessors, "invalid game star storage");
        if (apply) {
            GameDataFunction::setGalaxyStateOpened(galaxy->mGalaxyName);
            GameDataFunction::onGalaxyFlagTicoCoin(galaxy->mGalaxyName);
            GameDataFunction::onGalaxyFlagComet(galaxy->mGalaxyName);
        }
        require(GameDataFunction::isGalaxyStateOpened(galaxy->mGalaxyName), "galaxy is not open");
        require(GameDataFunction::isOnGalaxyFlagTicoCoin(galaxy->mGalaxyName), "comet medal not saved");
        for (int star = 1; star <= galaxy->mScenarioAccessorNum; ++star) {
            if (apply) {
                GameDataFunction::setPowerStar(galaxy->mGalaxyName, star);
                GameDataFunction::onGalaxyScenarioFlagAlreadyVisited(galaxy->mGalaxyName, star);
            }
            require(GameDataFunction::hasPowerStar(galaxy->mGalaxyName, star) &&
                    !GameDataFunction::hasBronzeStar(galaxy->mGalaxyName, star), "star not complete");
            ++count;
        }
    }
    require(count == 242 && count == GameDataFunction::getPowerStarNumMax() &&
            count == GameDataFunction::calcCurrentPowerStarNum(), "SMG2 star inventory/readback is not 242");
    return count;
}

void unlock(GameDataHolder& holder, bool apply) {
    stars(holder, apply);
    progressFlags(apply);
    for (int world = 1; world <= 8; ++world) {
        for (int bit = 0; bit < 8; ++bit) {
            if (apply) GameDataFunction::onWorldMapPointFlag(world, bit);
            require(GameDataFunction::isOnWorldMapPointFlag(world, bit), "world checkpoint not complete");
        }
    }
    require(holder.mTicoFatStorage, "missing Hungry Luma save storage");
    for (int i = 0; i < 48; ++i) {
        if (apply) holder.mTicoFatStorage->addValue(i, 0x7FFF);
        require(holder.mTicoFatStorage->getValue(i) == 0x7FFF, "Hungry Luma route not paid");
    }
    for (int i = 1; i <= 6; ++i) require(GameDataFunction::hasGrandStar(i), "missing world Grand Star");
    require(GameDataFunction::calcLastEnteredWorldNo() >= 7, "World S is not open");
    if (apply) {
        GameDataFunction::clearFaceShipEventValues();
        GameDataFunction::setCurrentWorldId(1);
        GameDataFunction::addPlayerLeft(99);
    }
}
} // namespace

void checkSelection(int file, bool isNew) {
    if (!checking()) return;
    PetariNative::HostAllocationScope hostAllocations;
    try {
        checkRoot();
        require(file == 1, "unlocked development save must use file 1 of its separate directory");
        if (creating()) {
            require(isNew, "refusing to replace an existing file; use a new unlocked user directory");
            require(!std::filesystem::exists(sUser / "complete-save.json"), "unlocked save receipt already exists");
            sNewFile = true;
        } else {
            require(!isNew, "no unlocked file exists; create it first");
        }
    } catch (const std::exception& error) {
        std::fprintf(stderr, "ASTRAL COMPLETE SAVE FAILED: %s\n", error.what());
        std::_Exit(2);
    }
}

bool beforeStageLaunch() {
    if (!checking() || sState == State::Done) return true;
    PetariNative::HostAllocationScope hostAllocations;
    try {
        checkRoot();
        auto* sequence = GameDataFunction::getSaveDataHandleSequence();
        require(sequence && sequence->_10 == 1, "unexpected selected save file");
        auto* file = sequence->getCurrentUserFile();
        auto* handler = sequence->mSaveDataHandler;
        require(file && handler && file->isCreated() && !file->mIsGameDataCorrupted && !file->mIsConfigDataCorrupted,
                "normal game loader rejected file");
        require(++sPolls < 18000, "save/reload timed out");
        if (sequence->isActive() || !handler->isDone()) return false;
        if (sState == State::Idle) {
            if (!creating()) {
                unlock(*file->mGameDataHolder, false);
                std::fprintf(stderr, "ASTRAL COMPLETE SAVE VERIFIED: file=1 stars=242 worlds=7 grandStars=6\n");
                sState = State::Done;
                return true;
            }
            require(sNewFile, "creation did not select a fresh file");
            const auto before = records(*handler);
            unlock(*file->mGameDataHolder, true);
            handler->storeUserFile(file);
            sStored = records(*handler);
            auto otherBefore = before, otherAfter = sStored;
            otherBefore.erase("user1"); otherAfter.erase("user1");
            require(otherBefore == otherAfter, "another save slot/config changed during unlock");
            handler->requestSaveSaveData();
            sState = State::Saving;
            std::fprintf(stderr, "ASTRAL COMPLETE SAVE: writing file 1 through SaveDataHandler\n");
            return false;
        }
        require(handler->getLastResultCode().isSuccess(), "game NAND operation failed");
        if (sState == State::Saving) {
            handler->requestLoadGameData();
            sState = State::Reloading;
            return false;
        }
        require(handler->requestVerifyAfterLoadGameDataFile(), "game save header/checksum/size verification failed");
        require(records(*handler) == sStored, "persisted save records differ after reload");
        sequence->startLoadUserFile(1);
        require(!file->mIsGameDataCorrupted && !file->mIsConfigDataCorrupted, "reloaded file is corrupt");
        unlock(*file->mGameDataHolder, false);
        std::ofstream receipt(sUser / "complete-save.json");
        receipt << "{\"game\":\"SB4E01\",\"file\":1,\"stars\":242,\"worlds\":7,\"grand_stars\":6,"
                   "\"game_save_reload_verified\":true,\"other_slots_unchanged\":true}\n";
        receipt.close();
        require(!receipt.fail(), "cannot write unlocked save receipt");
        std::fprintf(stderr, "ASTRAL COMPLETE SAVE RELOADED: file=1 stars=242 worlds=7 grandStars=6 otherSlots=unchanged\n");
        sState = State::Done;
        return true;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "ASTRAL COMPLETE SAVE FAILED: %s\n", error.what());
        std::_Exit(2);
    }
}
} // namespace NativeCompleteSave
#endif
