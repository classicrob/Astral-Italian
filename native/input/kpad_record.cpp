#include <petari/kpad_record.hpp>

#include <cmath>
#include <cstdio>
#include <cstring>

namespace PetariNative::Input::KpadRecord {
namespace {

constexpr int kMods = 16;
std::FILE* gKpd = nullptr;
std::FILE* gActions = nullptr;
std::uint64_t gFrame = 0;
std::uint32_t gOrdinal = 0;
std::uint64_t gReadCount = 0;
Stats gStats;
bool gPress[kMods] = {};
bool gHeld[kMods] = {};
bool gSeen[kMods] = {};  // the held value of a frame is its first answer (and only the first call can consume a press)

void put(unsigned char* out, unsigned offset, std::uint32_t value) {
    for (unsigned i = 0; i < 4; ++i) out[offset + i] = static_cast<unsigned char>(value >> (24 - 8 * i));
}
std::uint32_t get(const unsigned char* in, unsigned offset) {
    std::uint32_t value = 0;
    for (unsigned i = 0; i < 4; ++i) value = (value << 8) | in[offset + i];
    return value;
}
bool putReal(unsigned char* out, unsigned offset, f32 value) {
    std::uint32_t bits;
    std::memcpy(&bits, &value, 4);
    if ((bits & 0x7f800000u) == 0x7f800000u) return false;
    put(out, offset, bits);
    return true;
}
f32 getReal(const unsigned char* in, unsigned offset) {
    const std::uint32_t bits = get(in, offset);
    f32 value;
    std::memcpy(&value, &bits, 4);
    return value;
}
bool putVec(unsigned char* out, unsigned offset, const Vec& v) {
    return putReal(out, offset, v.x) & putReal(out, offset + 4, v.y) & putReal(out, offset + 8, v.z);
}
bool putVec2(unsigned char* out, unsigned offset, const Vec2& v) {
    return putReal(out, offset, v.x) & putReal(out, offset + 4, v.y);
}
Vec getVec(const unsigned char* in, unsigned offset) { return {getReal(in, offset), getReal(in, offset + 4), getReal(in, offset + 8)}; }
Vec2 getVec2(const unsigned char* in, unsigned offset) { return {getReal(in, offset), getReal(in, offset + 4)}; }

void u64(std::FILE* f, std::uint64_t v) {
    unsigned char b[8];
    for (int i = 0; i < 8; ++i) b[i] = static_cast<unsigned char>(v >> (56 - 8 * i));
    std::fwrite(b, 1, 8, f);
}
void u32be(std::FILE* f, std::uint32_t v) {
    unsigned char b[4];
    put(b, 0, v);
    std::fwrite(b, 1, 4, f);
}

}  // namespace

bool encodeSample(const KPADStatus& s, unsigned char out[240]) {
    std::memset(out, 0, 240);
    bool ok = true;
    put(out, 0, s.hold);
    put(out, 4, s.trig);
    put(out, 8, s.release);
    put(out, 236, s.unk_EC);
    ok &= putVec(out, 12, s.acc);
    ok &= putReal(out, 24, s.acc_value);
    ok &= putReal(out, 28, s.acc_speed);
    ok &= putVec2(out, 32, s.pos);
    ok &= putVec2(out, 40, s.vec);
    ok &= putReal(out, 48, s.speed);
    ok &= putVec2(out, 52, s.horizon);
    ok &= putVec2(out, 60, s.hori_vec);
    ok &= putReal(out, 68, s.hori_speed);
    ok &= putReal(out, 72, s.dist);
    ok &= putReal(out, 76, s.dist_vec);
    ok &= putReal(out, 80, s.dist_speed);
    ok &= putVec2(out, 84, s.acc_vertical);
    out[92] = s.dev_type;
    std::memcpy(&out[93], &s.wpad_err, 1);
    std::memcpy(&out[94], &s.dpd_valid_fg, 1);
    out[95] = s.data_format;
    if (s.dev_type == 1 || s.dev_type == 6) {
        ok &= putVec2(out, 96, s.ex_status.fs.stick);
        ok &= putVec(out, 104, s.ex_status.fs.acc);
        ok &= putReal(out, 116, s.ex_status.fs.acc_value);
        ok &= putReal(out, 120, s.ex_status.fs.acc_speed);
    } else if (s.dev_type == 2 || s.dev_type == 7) {
        put(out, 96, s.ex_status.cl.hold);
        put(out, 100, s.ex_status.cl.trig);
        put(out, 104, s.ex_status.cl.release);
        ok &= putVec2(out, 108, s.ex_status.cl.lstick);
        ok &= putVec2(out, 116, s.ex_status.cl.rstick);
        ok &= putReal(out, 124, s.ex_status.cl.ltrigger);
        ok &= putReal(out, 128, s.ex_status.cl.rtrigger);
    } else if (s.dev_type != 0 && s.dev_type != 5 && s.dev_type != 252 && s.dev_type != 253 && s.dev_type != 255) {
        ok = false;
    }
    ok &= putVec(out, 176, s.mpls.angularVelocity);
    ok &= putVec(out, 188, s.mpls.angle);
    ok &= putVec(out, 200, s.mpls.direction.X);
    ok &= putVec(out, 212, s.mpls.direction.Y);
    ok &= putVec(out, 224, s.mpls.direction.Z);
    return ok;
}

// The same layout petari_headless decodes (native/app/headless/kpad_decode.cpp); the extension union stays zero
// for devices that carry none.
bool decodeSample(const unsigned char in[240], KPADStatus* out) {
    std::memset(out, 0, sizeof(*out));
    out->hold = get(in, 0);
    out->trig = get(in, 4);
    out->release = get(in, 8);
    out->acc = getVec(in, 12);
    out->acc_value = getReal(in, 24);
    out->acc_speed = getReal(in, 28);
    out->pos = getVec2(in, 32);
    out->vec = getVec2(in, 40);
    out->speed = getReal(in, 48);
    out->horizon = getVec2(in, 52);
    out->hori_vec = getVec2(in, 60);
    out->hori_speed = getReal(in, 68);
    out->dist = getReal(in, 72);
    out->dist_vec = getReal(in, 76);
    out->dist_speed = getReal(in, 80);
    out->acc_vertical = getVec2(in, 84);
    out->dev_type = in[92];
    std::memcpy(&out->wpad_err, &in[93], 1);
    std::memcpy(&out->dpd_valid_fg, &in[94], 1);
    out->data_format = in[95];
    if (out->dev_type == 1 || out->dev_type == 6) {
        out->ex_status.fs.stick = getVec2(in, 96);
        out->ex_status.fs.acc = getVec(in, 104);
        out->ex_status.fs.acc_value = getReal(in, 116);
        out->ex_status.fs.acc_speed = getReal(in, 120);
    } else if (out->dev_type == 2 || out->dev_type == 7) {
        out->ex_status.cl.hold = get(in, 96);
        out->ex_status.cl.trig = get(in, 100);
        out->ex_status.cl.release = get(in, 104);
        out->ex_status.cl.lstick = getVec2(in, 108);
        out->ex_status.cl.rstick = getVec2(in, 116);
        out->ex_status.cl.ltrigger = getReal(in, 124);
        out->ex_status.cl.rtrigger = getReal(in, 128);
    }
    out->mpls.angularVelocity = getVec(in, 176);
    out->mpls.angle = getVec(in, 188);
    out->mpls.direction.X = getVec(in, 200);
    out->mpls.direction.Y = getVec(in, 212);
    out->mpls.direction.Z = getVec(in, 224);
    out->unk_EC = get(in, 236);
    return true;
}

bool open(const std::string& base, std::string* error) {
    if (gKpd) {
        *error = "tape already open";
        return false;
    }
    gKpd = std::fopen((base + ".kpd").c_str(), "wxb");
    gActions = gKpd ? std::fopen((base + ".actions").c_str(), "wx") : nullptr;
    if (!gKpd || !gActions) {
        *error = "cannot create " + base + ".kpd / .actions (they must not exist yet)";
        if (gKpd) std::fclose(gKpd);
        gKpd = nullptr;
        return false;
    }
    std::setvbuf(gKpd, nullptr, _IOFBF, 1 << 20);
    std::setvbuf(gActions, nullptr, _IOFBF, 1 << 16);
    std::fwrite("SMG2KPD1", 1, 8, gKpd);
    u64(gKpd, 1);  // frames, patched at close()
    u64(gKpd, 0);  // read count, patched at close()
    std::fputs("# frame mod press held (absolute game loop; one line per frame and mod with a press or the button held)\n", gActions);
    gFrame = gReadCount = 0;
    gOrdinal = 0;
    gStats = {};
    return true;
}

bool active() { return gKpd != nullptr; }
std::uint64_t frame() { return gFrame; }
Stats stats() { return gStats; }

void seam() {
    if (!gKpd) return;
    for (int m = 0; m < kMods; ++m) {
        if (gPress[m] || gHeld[m]) {
            std::fprintf(gActions, "%llu %d %d %d\n", static_cast<unsigned long long>(gFrame), m, gPress[m] ? 1 : 0, gHeld[m] ? 1 : 0);
            ++gStats.modLines;
        }
        gPress[m] = gHeld[m] = gSeen[m] = false;
    }
    ++gFrame;
    gOrdinal = 0;
    gStats.frames = gFrame;
}

void modPress(int mod, bool press) {
    if (!gKpd || mod < 0 || mod >= kMods) return;
    gPress[mod] |= press;
}

void modHeld(int mod, bool held) {
    if (!gKpd || mod < 0 || mod >= kMods || gSeen[mod]) return;
    gHeld[mod] = held;  // the first answer of the frame
    gSeen[mod] = true;
}

void read(s32 channel, u32 capacity, s32 returned, s32 error, s32 probeError, u32 device, const KPADStatus* samples) {
    if (!gKpd) return;
    const std::uint32_t count = returned > 0 ? static_cast<std::uint32_t>(returned) : 1u;  // slot 0 even for zero/error returns
    u64(gKpd, gFrame);
    u32be(gKpd, gOrdinal++);
    u32be(gKpd, static_cast<std::uint32_t>(channel));
    u32be(gKpd, capacity);
    u32be(gKpd, static_cast<std::uint32_t>(returned));
    u32be(gKpd, static_cast<std::uint32_t>(error));
    u32be(gKpd, static_cast<std::uint32_t>(probeError));
    u32be(gKpd, device);
    u32be(gKpd, count);
    for (std::uint32_t i = 0; i < count; ++i) {
        unsigned char bytes[240];
        if (!encodeSample(samples[i], bytes)) ++gStats.unencodable;
        // What the replay will hand the game must be what the game saw: bytes of the extension union the tape
        // cannot carry (beyond the Nunchuk/Classic members in use, or any for other devices) must be zero.
        KPADStatus want = samples[i];
        bool carried = true;
        unsigned char* ex = reinterpret_cast<unsigned char*>(&want.ex_status);
        const std::size_t used = (want.dev_type == 1 || want.dev_type == 6) ? 28 : (want.dev_type == 2 || want.dev_type == 7) ? 36 : 0;
        for (std::size_t k = used; k < sizeof(want.ex_status); ++k) {
            if (ex[k] != 0) carried = false;
            ex[k] = 0;
        }
        KPADStatus back;
        decodeSample(bytes, &back);
        if (!carried || std::memcmp(&want, &back, sizeof(KPADStatus)) != 0) ++gStats.lossy;
        std::fwrite(bytes, 1, 240, gKpd);
        ++gStats.samples;
    }
    ++gReadCount;
    ++gStats.reads;
}

// Valid for the loops finished so far: frames = completed seams + 1 (the header counts the open loop), reads written.
void checkpoint() {
    if (!gKpd) return;
    std::fflush(gActions);
    std::fflush(gKpd);
    const long end = std::ftell(gKpd);
    std::fseek(gKpd, 8, SEEK_SET);
    u64(gKpd, gFrame + 1);
    u64(gKpd, gReadCount);
    std::fflush(gKpd);
    std::fseek(gKpd, end, SEEK_SET);
}

void close() {
    if (!gKpd) return;
    std::fflush(gKpd);
    std::fseek(gKpd, 8, SEEK_SET);
    u64(gKpd, gFrame + 1);
    u64(gKpd, gReadCount);
    std::fclose(gKpd);
    std::fclose(gActions);
    gKpd = gActions = nullptr;
}

}  // namespace PetariNative::Input::KpadRecord
