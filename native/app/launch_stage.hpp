#pragma once

#include <petari/launch_stage.hpp>
#include <filesystem>

class ScenarioDataParser;
class JMapInfo;

namespace PetariNative::App::LaunchStage {
// Frontend setup before OSInit/game boot. Reads English messages, never a save.
// Call for --stage/list, a headless --request, or a catalog-consuming inventory.
bool configureCatalog(const std::filesystem::path& extractedDisc, std::string* error);
bool catalogConfigured();
void requestCatalogListing();
bool catalogListingRequested();

// ONE shared native hook, after GameSystem::mDataHolder->init(). The parser and
// world-order table must be fully loaded. Failure is explicit; caller must not
// continue a requested launch/inventory as if publication succeeded.
bool onGameDataReady(const ScenarioDataParser& parser, const JMapInfo& worldOrder, std::string* error);

// Opt-in PETARI_TEST_RETURN_TO_SHIP=<positive frames>[:<nonnegative start index>].
// One normal ship request after consecutive ready/action frames in the applied
// startup target. Call on the game thread before entering HostAllocationScope.
void testReturnToShipFrame();

// Normal after-file-load destination seam only. Explicit startup requests may
// override ANY selected file's initial destination, including the tutorial;
// false eligibility consumes/refuses the one-shot without modifying progress.
// Returns true only when the canonical stage-change request was dispatched.
bool onFileLoaded(bool canOverrideDestination);
}  // namespace PetariNative::App::LaunchStage
