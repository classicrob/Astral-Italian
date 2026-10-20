#pragma once
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace PetariNative::App::LaunchStage {

// Values copied from SMG2 ScenarioData, not a mission-count or progress table.
struct Scenario {
    int number = 0;                       // Actual ScenarioNo; gaps are permitted.
    std::uint32_t powerStarMask = 0;      // Raw PowerStarId BITMASK, not a star ID.
    std::string powerStarType;
    std::string comet;
    std::string name;                     // ScenarioName data/message identifier.
};

struct CatalogGalaxy {
    std::string stage;
    std::string englishName;              // Empty if no verified English label is available.
    int world = 0;
    std::vector<Scenario> scenarios;
    std::string provenance;               // Resource/table origin, supplied by the data reader.
};

// Read-only compatibility view for the Home menu. Pointer lifetime ends on the
// next successful installCatalog; publish/read on the game thread only.
struct Galaxy {
    const char* stage;
    const char* alias;
    const char* name;
    int dome;                            // Legacy member name: SMG2 world, NOT an SMG1 dome.
    int missions;                        // Number of actual scenario rows, NOT maximum ScenarioNo.
    const char* comets;                   // Legacy single-digit display tags only.
    const char* hidden;
};

struct Request {
    std::string stage;                    // Canonical internal name after resolveRequest.
    int scenario = 1;
};

enum class StartupStatus { Idle, PendingCatalog, PendingFileLoad, Applied, Failed };
// Configure once before boot; syntax errors leave the request unconfigured.
// Catalog membership is checked when the real game data is published.
bool configureStartupRequest(const std::string& command, std::string* error);
bool configureStartupRequest(const std::string& name, int number, std::string* error);
StartupStatus startupStatus();
const std::string& startupError();
const Request* pendingStartupRequest();
// Retained canonical selected target after resolution, including after dispatch.
const Request* resolvedStartupRequest();
int startupPlacedScenario();              // 0 before dispatch; can differ for hidden/green stars.
// Called by the normal file-load adapter only, after dispatch or refusal.
void finishStartupRequest(bool applied, const std::string& error = {}, int placedScenario = 0);
void failStartupRequest(const std::string& error);

// No built-in/fallback galaxy table. An engine/data reader must publish a full
// validated SMG2 inventory before resolution or listing. Failure leaves the old
// catalog intact. This API never reads/writes player progress or save storage.
bool installCatalog(std::vector<CatalogGalaxy> catalog, std::string* error);
bool catalogReady();
const std::vector<CatalogGalaxy>& catalog();

// Player-facing entries with verified English names; find/resolve also retain
// nameless internal records for explicit test launches.
const Galaxy* galaxies(int* count);
const char* domeName(int world);           // Legacy function name; prints "World 1".."World S".
const Galaxy* find(const std::string& name);
// Maps a Home-menu row index to the actual ScenarioNo (0 for an invalid index).
int scenarioNumberAt(const Galaxy& galaxy, int index);
const Scenario* scenario(const std::string& stage, int number);

// Internal names, generated English-name aliases, or English labels. ASCII case,
// punctuation, spaces and a trailing "galaxy" are ignored; ambiguity fails.
bool resolve(const std::string& name, int number, std::string* stage, std::string* error);
bool resolveRequest(const std::string& name, int number, Request* request, std::string* error);
// Shared headless command grammar: galaxy <name, optionally quoted> scenario <N>.
// Parses syntax only; resolveRequest performs catalog membership validation.
bool parseRequest(const std::string& command, Request* request, std::string* error);
void printList(std::FILE* out);

}  // namespace PetariNative::App::LaunchStage
