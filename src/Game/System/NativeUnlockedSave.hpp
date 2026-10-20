#pragma once

#ifdef GALAXY_NATIVE
#include <petari/unlocked_save.hpp>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

class GameDataHolder;
class SaveDataHandleSequence;
class ScenarioDataParser;
namespace PetariNative::App::LaunchStage { struct CatalogGalaxy; }

namespace NativeUnlockedSave {
// Called only after the shared level-select catalog publication and game data
// initialization. No SMG1 inventory, scenario-count fallback, or unlock flags.
PetariNative::UnlockedSave::Inventory resolveInventory(
    const std::vector<PetariNative::App::LaunchStage::CatalogGalaxy>&,
    ScenarioDataParser&, GameDataHolder&);
PetariNative::UnlockedSave::Snapshot readSnapshot(
    const GameDataHolder&, const PetariNative::UnlockedSave::Inventory&);

// Requires a supported, independently derived progression policy in inventory.
// The resolver currently declares all policies unsupported: no guessed flags.
// Uses the existing current-file star setter, never writes save-format bytes.
void applyStars(SaveDataHandleSequence&, const PetariNative::UnlockedSave::Inventory&,
                PetariNative::UnlockedSave::Profile);

// Asynchronous actual-game persistence adapter. Caller pumps the NORMAL save
// sequence/NAND worker, then poll(); no private event loop or fake completion.
// This transports a selected holder; it does NOT certify progression semantics.
// The frontend must also verify the complete plan/getters and produce a bound
// receipt before Python can publish. No frontend or hook is installed here.
class Readback {
public:
    enum class State { Idle, LoadingSeed, Ready, Saving, Reloading, Complete, Failed };
    Readback(SaveDataHandleSequence&, int slot);
    void beginLoad();
    void beginSave();
    void poll();
    State state() const { return mState; }
    const std::string& error() const { return mError; }
    const PetariNative::UnlockedSave::RoundTrip& result() const { return mResult; }

private:
    using Records = std::map<std::string, std::vector<std::uint8_t>>;
    Records records() const;
    void loadSelected();
    void fail(const std::string&);
    SaveDataHandleSequence& mSequence;
    int mSlot;
    State mState = State::Idle;
    std::string mError;
    Records mBefore, mStored;
    std::vector<std::uint8_t> mSerialized;
    PetariNative::UnlockedSave::RoundTrip mResult{};
};
} // namespace NativeUnlockedSave
#endif
