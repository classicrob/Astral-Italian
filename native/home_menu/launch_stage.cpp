#include "petari/launch_stage.hpp"

#include <algorithm>
#include <charconv>
#include <cstring>
#include <limits>
#include <memory>
#include <set>
#include <sstream>
#include <utility>

namespace PetariNative::App::LaunchStage {
namespace {

struct Catalog {
    std::vector<CatalogGalaxy> records;
    std::vector<std::string> aliases, comets, hidden;
    std::vector<Galaxy> views;
};

std::unique_ptr<Catalog> sCatalog;
StartupStatus sStartupStatus = StartupStatus::Idle;
Request sStartupRaw, sStartupResolved;
std::string sStartupError;
int sPlacedScenario = 0;

void resolvePendingStartup() {
    if (sStartupStatus != StartupStatus::PendingCatalog || !sCatalog) return;
    if (resolveRequest(sStartupRaw.stage, sStartupRaw.scenario, &sStartupResolved, &sStartupError))
        sStartupStatus = StartupStatus::PendingFileLoad;
    else
        sStartupStatus = StartupStatus::Failed;
}

bool fail(std::string* error, const std::string& message) {
    if (error) *error = message;
    return false;
}

bool asciiAlnum(unsigned char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
}

char lower(unsigned char c) { return c >= 'A' && c <= 'Z' ? static_cast<char>(c + ('a' - 'A')) : static_cast<char>(c); }

std::string key(const std::string& text) {
    std::string out;
    for (unsigned char c : text) if (asciiAlnum(c)) out += lower(c);
    if (out.size() >= 6 && out.compare(out.size() - 6, 6, "galaxy") == 0) out.resize(out.size() - 6);
    return out;
}

std::string alias(const std::string& text) {
    std::string out;
    bool separator = false;
    for (unsigned char c : text) {
        if (asciiAlnum(c)) {
            if (separator && !out.empty()) out += '-';
            out += lower(c);
            separator = false;
        } else if (c != '\'') {
            separator = true;
        }
    }
    if (out.size() > 7 && out.compare(out.size() - 7, 7, "-galaxy") == 0) out.resize(out.size() - 7);
    return out;
}

bool validStage(const std::string& stage) {
    return !stage.empty() && stage.size() < 64 && std::all_of(stage.begin(), stage.end(), [](unsigned char c) { return asciiAlnum(c); });
}

// Canonical internal names take precedence. A friendly name shared by multiple
// records is refused rather than silently selecting the first galaxy.
const Galaxy* lookup(const std::string& name, bool* ambiguous) {
    *ambiguous = false;
    if (!sCatalog) return nullptr;
    const std::string wanted = key(name);
    if (wanted.empty()) return nullptr;
    for (const Galaxy& g : sCatalog->views) if (key(g.stage) == wanted) return &g;
    const Galaxy* result = nullptr;
    for (const Galaxy& g : sCatalog->views) {
        if (key(g.alias) != wanted && key(g.name) != wanted) continue;
        if (result) { *ambiguous = true; return nullptr; }
        result = &g;
    }
    return result;
}

std::string numbers(const CatalogGalaxy& galaxy) {
    std::string out;
    for (const Scenario& s : galaxy.scenarios) {
        if (!out.empty()) out += ' ';
        out += std::to_string(s.number);
    }
    return out;
}

}  // namespace

bool installCatalog(std::vector<CatalogGalaxy> records, std::string* error) {
    if (records.empty()) return fail(error, "SMG2 stage catalog is empty");
    std::set<std::string> stages;
    for (CatalogGalaxy& g : records) {
        if (!validStage(g.stage) || key(g.stage).empty()) return fail(error, "invalid internal stage name: " + g.stage);
        if (!stages.insert(key(g.stage)).second) return fail(error, "duplicate internal stage: " + g.stage);
        if (g.world < 0 || g.provenance.empty() || g.scenarios.empty()) return fail(error, "incomplete SMG2 catalog record: " + g.stage);
        std::sort(g.scenarios.begin(), g.scenarios.end(), [](const Scenario& a, const Scenario& b) { return a.number < b.number; });
        int previous = 0;
        for (const Scenario& s : g.scenarios) {
            if (s.number <= 0 || s.number == previous) return fail(error, "invalid or duplicate ScenarioNo in " + g.stage);
            previous = s.number;
        }
        if (g.scenarios.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) return fail(error, "too many scenario rows");
    }
    std::stable_sort(records.begin(), records.end(), [](const CatalogGalaxy& a, const CatalogGalaxy& b) { return a.world < b.world; });
    auto next = std::make_unique<Catalog>();
    next->records = std::move(records);
    const auto count = next->records.size();
    next->aliases.resize(count);
    next->comets.resize(count);
    next->hidden.resize(count);
    next->views.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        const CatalogGalaxy& g = next->records[i];
        next->aliases[i] = alias(g.englishName.empty() ? g.stage : g.englishName);
        for (const Scenario& s : g.scenarios) {
            // These two strings support the old menu's single-digit badges;
            // actual membership and printList use complete integer ScenarioNo.
            if (s.number > 9) continue;
            if (!s.comet.empty()) next->comets[i] += static_cast<char>('0' + s.number);
            if (s.powerStarType == "Hidden") next->hidden[i] += static_cast<char>('0' + s.number);
        }
        next->views.push_back({g.stage.c_str(), next->aliases[i].c_str(), g.englishName.empty() ? g.stage.c_str() : g.englishName.c_str(),
                               g.world, static_cast<int>(g.scenarios.size()), next->comets[i].c_str(), next->hidden[i].c_str()});
    }
    sCatalog.swap(next);
    resolvePendingStartup();
    if (error) error->clear();
    return true;
}

bool catalogReady() { return sCatalog != nullptr; }

bool configureStartupRequest(const std::string& name, int number, std::string* error) {
    if (sStartupStatus != StartupStatus::Idle) return fail(error, "a startup galaxy request is already configured");
    if (number <= 0 || key(name).empty() || name.find('\0') != std::string::npos)
        return fail(error, "a galaxy name and positive ScenarioNo are required");
    sStartupRaw = {name, number};
    sStartupStatus = StartupStatus::PendingCatalog;
    resolvePendingStartup();
    if (sStartupStatus == StartupStatus::Failed) return fail(error, sStartupError);
    if (error) error->clear();
    return true;
}

bool configureStartupRequest(const std::string& command, std::string* error) {
    Request request;
    return parseRequest(command, &request, error) && configureStartupRequest(request.stage, request.scenario, error);
}

StartupStatus startupStatus() { return sStartupStatus; }
const std::string& startupError() { return sStartupError; }
const Request* pendingStartupRequest() { return sStartupStatus == StartupStatus::PendingFileLoad ? &sStartupResolved : nullptr; }
const Request* resolvedStartupRequest() { return sStartupResolved.stage.empty() ? nullptr : &sStartupResolved; }
int startupPlacedScenario() { return sPlacedScenario; }

void finishStartupRequest(bool applied, const std::string& error, int placedScenario) {
    if (sStartupStatus != StartupStatus::PendingFileLoad) return;
    sStartupStatus = applied ? StartupStatus::Applied : StartupStatus::Failed;
    sStartupError = applied ? std::string() : (error.empty() ? "startup galaxy request was refused" : error);
    sPlacedScenario = applied ? (placedScenario > 0 ? placedScenario : sStartupResolved.scenario) : 0;
}

void failStartupRequest(const std::string& error) {
    if (sStartupStatus == StartupStatus::PendingCatalog || sStartupStatus == StartupStatus::PendingFileLoad) {
        sStartupStatus = StartupStatus::Failed;
        sStartupError = error;
    }
}

const std::vector<CatalogGalaxy>& catalog() {
    static const std::vector<CatalogGalaxy> empty;
    return sCatalog ? sCatalog->records : empty;
}

const Galaxy* galaxies(int* count) {
    if (count) *count = sCatalog ? static_cast<int>(sCatalog->views.size()) : 0;
    return sCatalog && !sCatalog->views.empty() ? sCatalog->views.data() : nullptr;
}

const char* domeName(int world) {
    static const char* const names[] = {"Other stages", "World 1", "World 2", "World 3", "World 4", "World 5", "World 6", "World S"};
    return world >= 0 && world < 8 ? names[world] : "Other worlds";
}

const Galaxy* find(const std::string& name) {
    bool ambiguous = false;
    return lookup(name, &ambiguous);
}

int scenarioNumberAt(const Galaxy& galaxy, int index) {
    for (const CatalogGalaxy& g : catalog()) {
        if (g.stage == galaxy.stage && index >= 0 && static_cast<std::size_t>(index) < g.scenarios.size()) return g.scenarios[index].number;
    }
    return 0;
}

const Scenario* scenario(const std::string& stage, int number) {
    for (const CatalogGalaxy& g : catalog()) {
        if (g.stage != stage) continue;
        for (const Scenario& s : g.scenarios) if (s.number == number) return &s;
    }
    return nullptr;
}

bool resolve(const std::string& name, int number, std::string* stage, std::string* error) {
    if (!sCatalog) return fail(error, "SMG2 stage catalog has not been loaded");
    bool ambiguous = false;
    const Galaxy* g = lookup(name, &ambiguous);
    if (!g) return fail(error, (ambiguous ? "ambiguous galaxy \"" : "unknown galaxy \"") + name + "\" (--stage list shows available names)");
    if (!scenario(g->stage, number)) {
        const auto& record = sCatalog->records[static_cast<std::size_t>(g - sCatalog->views.data())];
        return fail(error, std::string(g->name) + " has ScenarioNo " + numbers(record) + "; scenario " + std::to_string(number) + " does not exist");
    }
    if (stage) *stage = g->stage;
    if (error) error->clear();
    return true;
}

bool resolveRequest(const std::string& name, int number, Request* request, std::string* error) {
    std::string stage;
    if (!resolve(name, number, &stage, error)) return false;
    if (request) *request = {std::move(stage), number};
    return true;
}

bool parseRequest(const std::string& command, Request* request, std::string* error) {
    std::istringstream input(command);
    std::vector<std::string> words;
    std::string word;
    while (input >> word) words.push_back(word);
    if (words.size() < 4 || words.front() != "galaxy" || words[words.size() - 2] != "scenario")
        return fail(error, "expected: galaxy <name> scenario <positive ScenarioNo>");
    int number = 0;
    const std::string& last = words.back();
    const auto parsed = std::from_chars(last.data(), last.data() + last.size(), number);
    if (parsed.ec != std::errc() || parsed.ptr != last.data() + last.size() || number <= 0)
        return fail(error, "scenario must be a positive integer ScenarioNo");
    std::string name;
    for (std::size_t i = 1; i + 2 < words.size(); ++i) { if (!name.empty()) name += ' '; name += words[i]; }
    if (name.size() >= 2 && name.front() == '"' && name.back() == '"') name = name.substr(1, name.size() - 2);
    if (name.empty() || name.find('"') != std::string::npos || key(name).empty()) return fail(error, "invalid galaxy name in request");
    if (request) *request = {std::move(name), number};
    if (error) error->clear();
    return true;
}

void printList(std::FILE* out) {
    if (!out) return;
    if (!sCatalog) { std::fputs("SMG2 stage catalog has not been loaded.\n", out); return; }
    std::fputs("SMG2 galaxies (alias, internal stage, actual ScenarioNo; C=comet H=hidden G=green):\n", out);
    int world = -1;
    for (std::size_t i = 0; i < sCatalog->records.size(); ++i) {
        const CatalogGalaxy& g = sCatalog->records[i];
        if (g.world != world) { world = g.world; std::fprintf(out, "\n%s\n", domeName(world)); }
        std::fprintf(out, "  %s  %s  %s [", sCatalog->aliases[i].c_str(), g.stage.c_str(), g.englishName.empty() ? "(English label unavailable)" : g.englishName.c_str());
        bool first = true;
        for (const Scenario& s : g.scenarios) {
            std::fprintf(out, "%s%d%s%s%s", first ? "" : " ", s.number, s.comet.empty() ? "" : "C", s.powerStarType == "Hidden" ? "H" : "", s.powerStarType == "Green" ? "G" : "");
            first = false;
        }
        std::fputs("]\n", out);
    }
}

}  // namespace PetariNative::App::LaunchStage
