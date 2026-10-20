#ifdef GALAXY_NATIVE
#include "NativeUnlockedSave.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/GameDataGalaxyStorage.hpp"
#include "Game/System/GameDataHolder.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
#include "Game/System/NANDManager.hpp"
#include "Game/System/SaveDataFileAccessor.hpp"
#include "Game/System/SaveDataHandleSequence.hpp"
#include "Game/System/SaveDataHandler.hpp"
#include "Game/System/ScenarioDataParser.hpp"
#include "Game/System/UserFile.hpp"
#include "Game/Util/SceneUtil.hpp"
#include <petari/launch_stage.hpp>
#include <cstring>
#include <stdexcept>

namespace Save = PetariNative::UnlockedSave;
namespace {
void require(bool condition, const char* reason) {
    if (!condition) throw std::runtime_error(reason);
}

GameDataSomeGalaxyStorage& storage(const GameDataHolder& holder, const std::string& name) {
    require(holder.mAllGalaxyStorage != nullptr, "missing game star storage");
    GameDataSomeGalaxyStorage* found = nullptr;
    for (int i = 0; i < holder.mAllGalaxyStorage->getGalaxyNum(); ++i) {
        auto* entry = holder.mAllGalaxyStorage->getGalaxyStorage(i);
        require(entry && entry->mGalaxyName, "invalid game galaxy storage");
        if (name == entry->mGalaxyName) {
            require(!found, "duplicate game galaxy storage");
            found = entry;
        }
    }
    require(found != nullptr, "catalog galaxy missing from actual save storage");
    return *found;
}

std::vector<std::uint8_t> serialize(GameDataHolder& holder) {
    std::vector<std::uint8_t> bytes(SaveDataHandler::getEnoughtTempBufferSize(), 0);
    const auto used = holder.makeFileBinary(bytes.data(), static_cast<u32>(bytes.size()));
    require(used > 0 && used <= bytes.size(), "game holder serialization failed");
    return bytes;
}
} // namespace

namespace NativeUnlockedSave {
Save::Inventory resolveInventory(
    const std::vector<PetariNative::App::LaunchStage::CatalogGalaxy>& catalog,
    ScenarioDataParser& parser, GameDataHolder& holder) {
    require(!catalog.empty() && holder.mAllGalaxyStorage, "catalog/game data not ready");
    Save::Inventory result{false, GameDataFunction::getPowerStarNumMax(), {}, {}};
    std::set<std::string> galaxies;
    for (const auto& galaxy : catalog) {
        require(!galaxy.stage.empty() && galaxies.insert(galaxy.stage).second, "duplicate catalog galaxy");
        auto* data = parser.getScenarioData(galaxy.stage.c_str());
        require(data != nullptr && galaxy.world == static_cast<int>(data->getWorldNo()),
                "catalog/parser world mismatch");
        GalaxyStatusAccessor accessor(data);
        std::uint32_t unionMask = 0;
        std::set<int> scenarioNumbers;
        for (const auto& row : galaxy.scenarios) {
            require(row.number > 0 && scenarioNumbers.insert(row.number).second, "invalid actual ScenarioNo");
            s32 mask = 0;
            require(data->getValueU32("PowerStarId", row.number, &mask) &&
                    static_cast<std::uint32_t>(mask) == row.powerStarMask, "catalog/parser star mask mismatch");
            unionMask |= row.powerStarMask;
        }
        if (!unionMask) continue; // Non-star scene, not a missing star record.
        require(galaxy.world >= 1 && galaxy.world <= 7, "star galaxy outside validated SMG2 worlds");
        auto& saved = storage(holder, galaxy.stage);
        require(saved.mScenarioAccessorNum > 0 && saved.mScenarioAccessorNum <= 32 &&
                saved.mScenarioAccessors != nullptr && saved.mScenarioNum == saved.mScenarioAccessorNum,
                "invalid star storage extent");
        const auto allowed = saved.mScenarioAccessorNum == 32 ? UINT32_MAX :
                             ((std::uint32_t{1} << saved.mScenarioAccessorNum) - 1);
        require(unionMask == allowed, "mask IDs do not cover exactly the allocated save records");
        require(saved.mScenarioAccessorNum == accessor.getPowerStarNum(), "engine storage/maximum disagreement");
        for (int id = 1; id <= saved.mScenarioAccessorNum; ++id) {
            require(scenarioNumbers.count(id), "star type has no canonical ScenarioNo record");
            Save::Star star{{galaxy.stage, id}, galaxy.world, accessor.isGreenStar(id), true, {}};
            for (const auto& row : galaxy.scenarios) {
                if ((row.powerStarMask & (std::uint32_t{1} << (id - 1))) == 0) continue;
                require(accessor.isExistPowerStarAtScenario(row.number, id), "engine rejected star association");
                star.scenarios.push_back(row.number);
            }
            require(!star.scenarios.empty(), "star has no actual scenario association");
            if (accessor.isHiddenStar(id) || star.green) {
                const int placed = MR::getPlacedHiddenStarScenarioNo(galaxy.stage.c_str(), id);
                require(scenarioNumbers.count(placed) && accessor.isExistPowerStarAtScenario(placed, id),
                        "hidden/green helper fallback lacks actual star membership");
            }
            result.stars.push_back(std::move(star));
        }
    }
    // Reverse coverage: no allocated game save record may disappear from a
    // partial catalog, even if its reported total happens to be plausible.
    int allocated = 0;
    for (int i = 0; i < holder.mAllGalaxyStorage->getGalaxyNum(); ++i) {
        auto* entry = holder.mAllGalaxyStorage->getGalaxyStorage(i);
        require(entry && entry->mGalaxyName && galaxies.count(entry->mGalaxyName), "catalog omits saved galaxy");
        allocated += entry->mScenarioAccessorNum;
    }
    require(result.gameMaximum == 242 && allocated == 242 && result.stars.size() == 242,
            "SB4E01 must resolve all 242 actual game save identities");
    result.complete = true;
    // World-map point/path progression and Luigi/Master Luma rules have not yet
    // been independently established for SMG2. Fail before any star mutation.
    for (auto profile : {Save::Profile::Worlds1To6, Save::Profile::Postgame, Save::Profile::AllStars})
        result.profiles[profile] = {false, {}};
    return result;
}

Save::Snapshot readSnapshot(const GameDataHolder& holder, const Save::Inventory& inventory) {
    require(inventory.complete && inventory.gameMaximum == 242 && inventory.stars.size() == 242,
            "snapshot requires complete SB4E01 inventory");
    Save::Snapshot result{{}, holder.calcCurrentPowerStarNum(), {}};
    for (const auto& star : inventory.stars) {
        auto& saved = storage(holder, star.key.first);
        require(star.engineValidated && star.key.second >= 1 && star.key.second <= saved.mScenarioAccessorNum &&
                saved.mScenarioAccessors != nullptr,
                "unvalidated or out-of-bounds snapshot identity");
        const auto* record = saved.getScenarioAccessor(star.key.second);
        if (record->testFlag(1))
            require(result.owned.emplace(star.key, record->testFlag(2)).second, "duplicate snapshot identity");
    }
    require(result.gameOwnedCount == static_cast<int>(result.owned.size()), "game owned-count mismatch");
    return result;
}

void applyStars(SaveDataHandleSequence& sequence, const Save::Inventory& inventory, Save::Profile profile) {
    const auto plan = Save::makePlan(inventory, profile); // Unsupported policies refuse here.
    require(&sequence == GameDataFunction::getSaveDataHandleSequence(), "not the game's active save sequence");
    auto* file = sequence.getCurrentUserFile();
    require(file && file->isCreated() && !file->mIsGameDataCorrupted && !file->mIsConfigDataCorrupted,
            "selected seed is missing or corrupt");
    const auto before = readSnapshot(*file->mGameDataHolder, inventory);
    for (const auto& entry : before.owned)
        require(plan.stars.count(entry.first), "seed has out-of-profile stars; lower-progress seed required");
    for (const auto& star : plan.stars)
        GameDataFunction::setPowerStar(star.first.c_str(), star.second);
    // This function only applies stars. It never claims world/character unlocks,
    // emits a receipt, or substitutes star count for a progression readback.
}

Readback::Readback(SaveDataHandleSequence& sequence, int slot) : mSequence(sequence), mSlot(slot) {
    require(slot > 0, "invalid selected slot");
}

void Readback::fail(const std::string& reason) { mError = reason; mState = State::Failed; }

Readback::Records Readback::records() const {
    auto* handler = mSequence.mSaveDataHandler;
    require(handler && handler->_18, "missing validated save data");
    SaveDataFileAccessor accessor(handler->_18);
    const auto* header = accessor.getHeader();
    // The canonical handler validates version/size/checksum first. Bound the
    // record directory before using its accessor; no authored save byte layout.
    require(SaveDataHandler::isCorrectFileHeader(handler->_18), "invalid game file header");
    require(header->mFileSize >= sizeof(SaveDataFileHeader) &&
            header->mUserFileInfoNum <= (header->mFileSize - sizeof(SaveDataFileHeader)) / sizeof(SaveDataFileInfo),
            "invalid game record directory");
    Records out;
    const auto directoryEnd = sizeof(SaveDataFileHeader) + header->mUserFileInfoNum * sizeof(SaveDataFileInfo);
    for (u32 i = 0; i < header->mUserFileInfoNum; ++i) {
        const auto* info = accessor.getFileInfo(i);
        require(std::memchr(info->mName, 0, sizeof(info->mName)) != nullptr, "unterminated game record name");
        const auto end = i + 1 < header->mUserFileInfoNum ? accessor.getFileInfo(i + 1)->mOffset : header->mFileSize;
        require(info->mOffset >= directoryEnd && info->mOffset < end && end <= header->mFileSize,
                "invalid game record extent");
        require(out.emplace(info->mName, std::vector<std::uint8_t>(handler->_18 + info->mOffset,
                                                                handler->_18 + end)).second,
                "duplicate game record name");
    }
    return out;
}

void Readback::beginLoad() {
    require(mState == State::Idle && !mSequence.isActive() && mSequence.mSaveDataHandler &&
            mSequence.mSaveDataHandler->isDone(), "save sequence is busy/uninitialized");
    mSequence.mSaveDataHandler->requestLoadGameData();
    mState = State::LoadingSeed;
}

void Readback::loadSelected() {
    const auto all = records();
    const auto suffix = std::to_string(mSlot);
    require(all.count("user" + suffix) && all.count("config" + suffix), "selected slot does not exist");
    mSequence.startLoadUserFile(mSlot); // Real UserFile/GameDataHolder/config loaders.
    const auto* file = mSequence.getCurrentUserFile();
    require(file && file->isCreated() && !file->mIsGameDataCorrupted && !file->mIsConfigDataCorrupted,
            "regular game loader rejected selected file");
}

void Readback::beginSave() {
    require(mState == State::Ready && !mSequence.isActive(), "seed is not ready for save");
    auto* file = mSequence.getCurrentUserFile();
    require(file && !file->mIsGameDataCorrupted && !file->mIsConfigDataCorrupted &&
            file->getGameDataName() == std::string("user") + std::to_string(mSlot), "selected file changed");
    mSerialized = serialize(*file->mGameDataHolder);
    auto* handler = mSequence.mSaveDataHandler;
    handler->storeUserFile(file); // Canonical serializers; preserve the seed's config/icon, no reset.
    mStored = records();
    auto unrelated = mStored;
    auto original = mBefore;
    const auto selected = "user" + std::to_string(mSlot);
    unrelated.erase(selected);
    original.erase(selected);
    require(unrelated == original, "serialization altered config or unrelated game records");
    handler->requestSaveSaveData(); // Actual game's persistent NAND writer.
    mState = State::Saving;
}

void Readback::poll() {
    if (mState != State::LoadingSeed && mState != State::Saving && mState != State::Reloading) return;
    try {
        auto* handler = mSequence.mSaveDataHandler;
        if (!handler->isDone()) return;
        require(handler->getLastResultCode().isSuccess(), "game NAND operation failed");
        if (mState == State::Saving) {
            mResult.saveCompleted = true;
            handler->requestLoadGameData();
            mState = State::Reloading;
            return;
        }
        require(handler->requestVerifyAfterLoadGameDataFile(), "game header/version/size/checksum checks failed");
        loadSelected();
        if (mState == State::LoadingSeed) {
            mBefore = records();
            mState = State::Ready;
            return;
        }
        auto* file = mSequence.getCurrentUserFile();
        mResult.headerValid = mResult.versionValid = mResult.checksumValid = true;
        mResult.regularLoaderSucceeded = true;
        mResult.gameDataCorrupted = file->mIsGameDataCorrupted;
        mResult.configDataCorrupted = file->mIsConfigDataCorrupted;
        mResult.bytesEqual = records() == mStored && serialize(*file->mGameDataHolder) == mSerialized;
        mResult.unrelatedSlotsUnchanged = mResult.bytesEqual; // Stronger all-record equality above.
        require(mResult.bytesEqual, "regular reload differs from stored holder/records");
        mState = State::Complete;
    } catch (const std::exception& error) {
        fail(error.what());
    }
}
} // namespace NativeUnlockedSave
#endif
