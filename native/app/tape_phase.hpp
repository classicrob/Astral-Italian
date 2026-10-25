#pragma once
// Schema-2 states are sampled BEFORE the first KPAD read of a loop. They are
// not post-loop route rows. No SDK types or game pointers enter this contract.
#include <array>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

namespace PetariNative::App::TapePhase {
inline constexpr const char* kProtocol = "game-scene-pre-read-v1";
inline constexpr const char* kPolicy = "game-defined-scene-input-v2";
struct State {
    std::uint64_t inputFrame = 0;
    std::string kind = "none", scene, stage;
    int scenario = -1;
    bool firstNormal = false, valid = false, grounded = false, dead = false;
    bool rngObserved = false, mapWait = false, galaxyMap = false;
    std::uint32_t rng = 0;
    std::array<std::uint32_t,3> position{};
    std::string key() const { return kind + "\n" + scene + "\n" + stage + "\n" + std::to_string(scenario); }
};
struct Checkpoint {
    std::string kind;
    State state;
};
struct Phase {
    std::uint64_t id = 0, endInputFrame = 0;
    bool open = true;
    State marker;
    std::vector<Checkpoint> checkpoints;
};
inline std::string quote(const std::string& s) {
    std::string out = "\"";
    for (unsigned char c : s) {
        if (c == '\\' || c == '"') out += '\\';
        if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else if (c == '\t') out += "\\t";
        else out += static_cast<char>(c);
    }
    return out + "\"";
}
inline std::string stateJson(const State& s) {
    std::ostringstream o;
    o << "{\"input_frame\":" << s.inputFrame << ",\"kind\":" << quote(s.kind)
      << ",\"scene\":" << quote(s.scene) << ",\"stage\":" << quote(s.stage)
      << ",\"scenario\":" << s.scenario << ",\"first_normal_update\":" << (s.firstNormal ? "true":"false")
      << ",\"valid\":" << (s.valid ? "true":"false") << ",\"grounded\":" << (s.grounded ? "true":"false")
      << ",\"dead\":" << (s.dead ? "true":"false") << ",\"position_bits\":["
      << s.position[0] << ',' << s.position[1] << ',' << s.position[2]
      << "],\"rng_observed\":" << (s.rngObserved ? "true":"false") << ",\"rng_seed\":" << s.rng
      << ",\"map_wait\":" << (s.mapWait ? "true":"false")
      << ",\"galaxy_map\":" << (s.galaxyMap ? "true":"false") << '}';
    return o.str();
}
}  // namespace PetariNative::App::TapePhase
