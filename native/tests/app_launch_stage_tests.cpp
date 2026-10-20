// Synthetic SMG2-schema records exercise launch logic without disc assets or a save.
#include "petari/launch_stage.hpp"
#include "launch_stage_messages.hpp"
#include "archive.hpp"

#include <cstdio>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace LS = PetariNative::App::LaunchStage;
namespace {
int failures = 0;
void check(bool ok, const char* what) {
    if (!ok) { std::fprintf(stderr, "FAIL: %s\n", what); ++failures; }
}

LS::CatalogGalaxy fixture() {
    return {"SyntheticTestGalaxy", "Sample's Test Galaxy", 1,
            {{7, 4, "Hidden", "", "HiddenName"}, {2, 1, "Normal", "Purple", "CometName"},
             {12, 0x80000005u, "Green", "", "GreenName"}},
            "synthetic fixture: ScenarioData.bcsv + GalaxyWorldOrderList.bcsv; NOT disc inventory"};
}

std::string listing() {
    std::FILE* file = std::tmpfile();
    if (!file) { check(false, "temporary list output"); return {}; }
    LS::printList(file);
    std::rewind(file);
    std::string text;
    char buffer[512];
    while (std::fgets(buffer, sizeof(buffer), file)) text += buffer;
    std::fclose(file);
    return text;
}

using Bytes = std::vector<std::uint8_t>;
void put32(Bytes& b, std::size_t at, std::uint32_t x) {
    for (int i = 0; i < 4; ++i) b[at + i] = static_cast<std::uint8_t>(x >> (24 - 8 * i));
}
Bytes message(const std::string& label, const std::u16string& text) {
    const std::size_t labels = 12 + 1 + label.size() + 4;
    const std::size_t txt = (0x30 + labels + 15) & ~std::size_t(15);
    const std::size_t textSize = 8 + 2 * (text.size() + 1);
    Bytes b((txt + 16 + textSize + 15) & ~std::size_t(15));
    std::memcpy(b.data(), "MsgStdBn", 8); b[8] = 0xfe; b[9] = 0xff; b[0xc] = 1; b[0xd] = 3; b[0xf] = 2;
    put32(b, 0x12, static_cast<std::uint32_t>(b.size()));
    std::memcpy(b.data() + 0x20, "LBL1", 4); put32(b, 0x24, static_cast<std::uint32_t>(labels));
    put32(b, 0x30, 1); put32(b, 0x34, 1); put32(b, 0x38, 12);
    b[0x3c] = static_cast<std::uint8_t>(label.size()); std::memcpy(b.data() + 0x3d, label.data(), label.size());
    put32(b, 0x3d + label.size(), 0);
    std::memcpy(b.data() + txt, "TXT2", 4); put32(b, txt + 4, static_cast<std::uint32_t>(textSize));
    put32(b, txt + 16, 1); put32(b, txt + 20, 8);
    for (std::size_t i = 0; i < text.size(); ++i) { b[txt + 24 + 2 * i] = static_cast<std::uint8_t>(text[i] >> 8); b[txt + 25 + 2 * i] = static_cast<std::uint8_t>(text[i]); }
    return b;
}
Bytes archive(const Bytes& payload) {
    Bytes b(0x80 + payload.size());
    std::memcpy(b.data(), "RARC", 4); put32(b, 4, static_cast<std::uint32_t>(b.size())); put32(b, 8, 0x20);
    put32(b, 12, 0x60); put32(b, 16, static_cast<std::uint32_t>(payload.size()));
    put32(b, 0x20, 1); put32(b, 0x24, 0x20); put32(b, 0x28, 1); put32(b, 0x2c, 0x30);
    put32(b, 0x30, 16); put32(b, 0x34, 0x44); std::memcpy(b.data() + 0x40, "ROOT", 4); b[0x4b] = 1;
    b[0x54] = 0x11; b[0x57] = 5; put32(b, 0x5c, static_cast<std::uint32_t>(payload.size()));
    std::memcpy(b.data() + 0x64, "root\0names.msbt\0", 16); std::memcpy(b.data() + 0x80, payload.data(), payload.size());
    return b;
}
Bytes yaz(const Bytes& data) {
    Bytes b(16); std::memcpy(b.data(), "Yaz0", 4); put32(b, 4, static_cast<std::uint32_t>(data.size()));
    for (std::size_t i = 0; i < data.size();) { b.push_back(0xff); for (int bit = 0; bit < 8 && i < data.size(); ++bit) b.push_back(data[i++]); }
    return b;
}
void messageTests() {
    std::string error;
    const Bytes msbt = message("GalaxyName_SyntheticTestGalaxy", u"Sample's Test Galaxy");
    LS::EnglishNames names;
    check(LS::readEnglishGalaxyNames({msbt.data(), msbt.size()}, "synthetic.msbt", &names, &error) &&
              names.at("SyntheticTestGalaxy").text == "Sample's Test Galaxy", "actual BE MSBT label/text decode");
    const auto first = names.at("SyntheticTestGalaxy").provenance;
    for (std::size_t n = 0; n < msbt.size(); ++n) {
        check(!LS::readEnglishGalaxyNames({msbt.data(), n}, "truncated", &names, &error) && names.size() == 1 &&
                  names.at("SyntheticTestGalaxy").provenance == first, "all MSBT truncations reject atomically");
    }
    auto bad = msbt; put32(bad, 0x38, 0xffffffffu);
    check(!LS::readEnglishGalaxyNames({bad.data(), bad.size()}, "bad", &names, &error), "label bucket offset bounds");
    bad = msbt; bad[8] = 0xff; bad[9] = 0xfe;
    check(!LS::readEnglishGalaxyNames({bad.data(), bad.size()}, "bad", &names, &error), "unexpected byte order rejected");
    bad = message("GalaxyName_../Escape", u"Bad");
    check(!LS::readEnglishGalaxyNames({bad.data(), bad.size()}, "bad", &names, &error), "label cannot inject stage path");
    bad = message("GalaxyName_SyntheticTestGalaxy", u"Conflicting name");
    check(!LS::readEnglishGalaxyNames({bad.data(), bad.size()}, "bad", &names, &error), "conflicting labels do not overwrite names");
    bad = message("GalaxyName_ControlGalaxy", std::u16string{u'A', 14, u'B'});
    check(!LS::readEnglishGalaxyNames({bad.data(), bad.size()}, "bad", &names, &error), "formatted name fails rather than guessing tag removal");
    bad = message("GalaxyName_UnicodeGalaxy", u"A \U0001f30c Galaxy");
    check(LS::readEnglishGalaxyNames({bad.data(), bad.size()}, "unicode", &names, &error) &&
              names.at("UnicodeGalaxy").text == "A \xf0\x9f\x8c\x8c Galaxy", "UTF-16 surrogate pair converts to UTF-8");
    std::filesystem::path root;
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    for (int attempt = 0; attempt < 100; ++attempt) {
        const auto candidate = std::filesystem::temp_directory_path() / ("petari-launch-stage-" + std::to_string(stamp) + "-" + std::to_string(attempt));
        if (std::filesystem::create_directory(candidate)) { root = candidate; break; }
    }
    if (root.empty()) { check(false, "private synthetic resource directory"); return; }
    const auto path = root / "files/LocalizeData/UsEnglish/MessageData";
    std::filesystem::create_directories(path);
    auto write = [&](const char* filename, const Bytes& bytes) {
        std::ofstream f(path / filename, std::ios::binary); f.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    };
    write("SystemMessage.arc", archive(msbt)); write("LayoutMessage.arc", yaz(archive(msbt)));
    LS::EnglishNames loaded;
    check(LS::loadEnglishGalaxyNames(root, &loaded, &error) && loaded.size() == 1 &&
              loaded.at("SyntheticTestGalaxy").text == "Sample's Test Galaxy", "English archive pipeline including Yaz0 RARC");
    std::filesystem::remove(path / "LayoutMessage.arc");
    check(!LS::loadEnglishGalaxyNames(root, &loaded, &error) && loaded.size() == 1, "missing English archive fails without fabricating labels");
    std::filesystem::remove_all(root);  // Only this test's newly created synthetic directory.
}
}  // namespace

int main(int argc, char** argv) {
    if (argc == 2) {
        std::string error;
        const bool unknown = std::strcmp(argv[1], "--unknown-startup") == 0;
        check(LS::configureStartupRequest(unknown ? "galaxy Absent scenario 2" : "galaxy SyntheticTestGalaxy scenario 2", &error), "startup owns command before catalog");
        check(LS::startupStatus() == LS::StartupStatus::PendingCatalog, "pending catalog is not success");
        check(LS::installCatalog({fixture()}, &error), "publish for startup failure test");
        if (!unknown) LS::finishStartupRequest(false, "tutorial retained");
        check(LS::startupStatus() == LS::StartupStatus::Failed && !LS::startupError().empty() && LS::startupPlacedScenario() == 0,
              "unknown galaxy/refused destination remains visibly failed");
        return failures ? 1 : 0;
    }
    std::string stage = "unchanged", error;
    int count = -1;
    check(!LS::catalogReady() && LS::galaxies(&count) == nullptr && count == 0, "no authored fallback catalog");
    check(!LS::resolve("good-egg", 1, &stage, &error) && error.find("not been loaded") != std::string::npos && stage == "unchanged",
          "old imported SMG1 catalog is not accepted before data loading");
    check(!LS::installCatalog({}, &error), "empty inventory fails closed");
    check(listing().find("not been loaded") != std::string::npos, "list states unavailable data truthfully");
    check(!LS::configureStartupRequest("galaxy Test scenario invalid", &error) && LS::startupStatus() == LS::StartupStatus::Idle, "syntax error before boot");
    check(LS::configureStartupRequest("galaxy samples-test scenario 12", &error) && LS::startupStatus() == LS::StartupStatus::PendingCatalog,
          "configure once before real metadata is available");

    check(LS::installCatalog({fixture()}, &error), "publish synthetic SMG2 records");
    check(LS::catalogReady() && error.empty(), "successful publication clears prior error");
    check(LS::startupStatus() == LS::StartupStatus::PendingFileLoad && LS::pendingStartupRequest()->stage == "SyntheticTestGalaxy" &&
              LS::resolvedStartupRequest()->scenario == 12 && LS::startupPlacedScenario() == 0, "catalog resolution is not dispatch or arrival");
    LS::finishStartupRequest(true, {}, 2);
    check(LS::startupStatus() == LS::StartupStatus::Applied && LS::pendingStartupRequest() == nullptr && LS::startupPlacedScenario() == 2 &&
              LS::resolvedStartupRequest()->scenario == 12, "selected and placed scenarios remain distinct after dispatch");
    check(!LS::configureStartupRequest("galaxy Test scenario 1", &error), "startup is one-shot, never repeated by another file load");
    const LS::Galaxy* all = LS::galaxies(&count);
    check(count == 1 && all && all[0].missions == 3 && all[0].dome == 1, "actual row count and SMG2 world");
    check(LS::resolve("samples-test", 2, &stage, &error) && stage == "SyntheticTestGalaxy", "English alias");
    check(LS::resolve("Sample's TEST Galaxy", 7, &stage, &error), "case, apostrophe, spaces and suffix normalization");
    check(LS::resolve("synthetictestgalaxy", 12, &stage, &error), "canonical internal name, scenario above9");
    check(!LS::resolve("samples-test", 1, &stage, &error) && error.find("ScenarioNo 2 7 12") != std::string::npos,
          "noncontiguous ScenarioNo membership, not1..count");
    check(!LS::resolve("samples-test", 3, &stage, &error) && !LS::resolve("samples-test", 0, &stage, &error), "missing and zero scenarios rejected");
    check(LS::scenarioNumberAt(all[0], 0) == 2 && LS::scenarioNumberAt(all[0], 1) == 7 && LS::scenarioNumberAt(all[0], 2) == 12 &&
              LS::scenarioNumberAt(all[0], 3) == 0 && LS::scenarioNumberAt(all[0], -1) == 0,
          "menu ordinal maps to actual scenario number");
    const LS::Scenario* s = LS::scenario("SyntheticTestGalaxy", 12);
    check(s && s->powerStarMask == 0x80000005u && s->powerStarType == "Green", "raw star mask/type retained without inventing IDs");
    check(LS::catalog()[0].provenance == fixture().provenance, "source provenance retained");
    const std::string list = listing();
    check(list.find("World 1") != std::string::npos && list.find("[2C 7H 12G]") != std::string::npos, "list uses actual multi-digit rows and metadata tags");
    check(list.find("Terrace") == std::string::npos && list.find("Observatory") == std::string::npos, "no SMG1 dome labels");

    LS::Request request{"unchanged", 99};
    check(LS::parseRequest("galaxy \"Sample's Test Galaxy\" scenario 12", &request, &error) && request.scenario == 12 &&
              LS::resolveRequest(request.stage, request.scenario, &request, &error) && request.stage == "SyntheticTestGalaxy",
          "headless grammar shares canonical catalog resolution");
    for (const char* command : {"", "galaxy", "galaxy Test scenario", "galaxy Test scenario 0", "galaxy Test scenario -1",
                                "galaxy Test scenario 2147483648", "galaxy Test scenario 1.5", "galaxy Test scenario 1 extra",
                                "galaxy \"Test scenario 1", "galaxy Test\" scenario 1", "galaxy \"\" scenario 1"}) {
        const LS::Request before = request;
        check(!LS::parseRequest(command, &request, &error) && request.stage == before.stage && request.scenario == before.scenario,
              "malformed headless request fails without mutating output");
    }
    check(!LS::resolveRequest("no-such-stage", 2, &request, &error) && request.stage == "SyntheticTestGalaxy", "unknown request leaves output intact");
    check(LS::resolve("samples-test", 2, nullptr, nullptr), "optional outputs");

    const LS::Galaxy* before = LS::find("SyntheticTestGalaxy");
    auto bad = fixture(); bad.scenarios.push_back(bad.scenarios[0]);
    check(!LS::installCatalog({bad}, &error) && LS::find("SyntheticTestGalaxy") == before, "duplicate scenario rejects whole update atomically");
    bad = fixture(); bad.stage = "../StageData/Escape";
    check(!LS::installCatalog({bad}, &error), "path traversal cannot become an internal stage");
    bad = fixture(); bad.provenance.clear();
    check(!LS::installCatalog({bad}, &error), "unprovenanced inventory rejected");
    bad = fixture(); bad.scenarios[0].number = -1;
    check(!LS::installCatalog({bad}, &error), "negative ScenarioNo rejected");
    bad = fixture(); bad.world = -1;
    check(!LS::installCatalog({bad}, &error), "negative world rejected");
    check(!LS::installCatalog({fixture(), fixture()}, &error), "duplicate canonical stage rejected");

    auto other = fixture(); other.stage = "AnotherSyntheticGalaxy"; other.world = 7;
    check(LS::installCatalog({other, fixture()}, &error), "distinct stages may have ambiguous friendly names");
    check(LS::find("samples-test") == nullptr && !LS::resolve("samples-test", 2, &stage, &error) && error.find("ambiguous") != std::string::npos,
          "ambiguous friendly name never picks first record");
    check(LS::resolve("AnotherSyntheticGalaxy", 7, &stage, &error), "canonical name still disambiguates");
    check(listing().find("World 1") < listing().find("World S"), "list sorts by SMG2 world");

    other.englishName.clear();
    check(LS::installCatalog({other}, &error) && LS::resolve(other.stage, 2, &stage, &error), "missing English label retains canonical internal destination");
    check(LS::galaxies(&count) == nullptr && count == 0 && listing().find(other.stage) == std::string::npos,
          "nameless records stay out of the player-facing catalog and CLI list");

    LS::CatalogGalaxy world7{"WorldMap07Galaxy", "World S", 7, {{1, 0, "", "", ""}}, "synthetic named world"};
    auto world8 = world7; world8.stage = "WorldMap08Galaxy"; world8.world = 8; world8.englishName.clear();
    check(LS::installCatalog({world8, fixture(), world7}, &error), "named and nameless worlds install together");
    all = LS::galaxies(&count);
    check(count == 2 && all && std::strcmp(all[0].stage, "SyntheticTestGalaxy") == 0 &&
              std::strcmp(all[1].stage, "WorldMap07Galaxy") == 0, "named worlds stay selectable without world8");
    check(listing().find("WorldMap07Galaxy") != std::string::npos && listing().find("WorldMap08Galaxy") == std::string::npos,
          "CLI list excludes the nameless world while retaining named worlds");
    check(LS::resolve("WorldMap08Galaxy", 1, &stage, &error) && stage == "WorldMap08Galaxy" &&
              LS::find("WorldMap08Galaxy") && LS::scenario("WorldMap08Galaxy", 1),
          "explicit world8 test launch retains canonical resolution and scenario metadata");
    check(!LS::resolve("WorldMap08Galaxy", 2, &stage, &error) && error.find("ScenarioNo 1;") != std::string::npos,
          "unlisted world still validates actual scenario membership");
    check(LS::find("FileSelect") == nullptr && LS::find("AstroDome") == nullptr, "no implicit menu/SMG1 hub destinations");
    messageTests();

    if (failures) return 1;
    std::puts("launch stage: data-derived catalog, sparse scenarios, masks, normalization, request grammar and atomic validation PASS");
    return 0;
}
