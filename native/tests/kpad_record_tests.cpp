// Tape recorder schema: KPADStatus <-> the 240-byte big-endian tape sample, and the .kpd/.actions files.
#include <petari/kpad_record.hpp>

#include <chrono>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <unistd.h>
#include <string>

using namespace PetariNative::Input;

namespace {
int failures = 0;
void check(bool ok, const char* what) {
    if (!ok) {
        std::fprintf(stderr, "FAIL: %s\n", what);
        ++failures;
    }
}
std::string hex(const unsigned char* b, int n) {
    std::string out;
    char two[3];
    for (int i = 0; i < n; ++i) {
        std::snprintf(two, sizeof two, "%02x", b[i]);
        out += two;
    }
    return out;
}
std::uint64_t be(const std::string& s, size_t at, int n) {
    std::uint64_t v = 0;
    for (int i = 0; i < n; ++i) v = (v << 8) | static_cast<unsigned char>(s[at + i]);
    return v;
}
KPADStatus sample() {
    KPADStatus s;
    std::memset(&s, 0, sizeof s);
    s.hold = 0x0c08;
    s.trig = 0x0800;
    s.release = 0x0400;
    s.acc = {3.0f, -1.0f, 0.5f};
    s.acc_value = 1.0f;
    s.pos = {0.25f, -0.5f};
    s.horizon = {1.0f, 0.0f};
    s.acc_vertical = {1.0f, 0.0f};
    s.dev_type = 1;
    s.data_format = 5;
    s.ex_status.fs.stick = {0.5f, -1.0f};
    s.ex_status.fs.acc = {0.0f, -1.0f, 0.0f};
    s.ex_status.fs.acc_value = 1.0f;
    s.mpls.direction.X = {0, 0, 0};
    s.unk_EC = 7;
    return s;
}
}  // namespace

int main(int argc, char** argv) {
    // Cross-check with native/tools/headless.py encode_sample (same field values, hex taken from it).
    static const char* kExpected =
        "00000c08000008000000040040400000bf8000003f0000003f800000000000003e800000bf0000000000000000000000000000003f80000000000000"
        "0000000000000000000000000000000000000000000000003f80000000000000010000053f000000bf80000000000000bf800000000000003f800000"
        "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000007";
    KPADStatus s = sample();
    unsigned char bytes[240];
    check(KpadRecord::encodeSample(s, bytes), "sample encodes");
    check(hex(bytes, 240) == kExpected, "encoding equals headless.py's");
    KPADStatus back;
    check(KpadRecord::decodeSample(bytes, &back) && std::memcmp(&back, &s, 0x60) == 0 &&
              std::memcmp(&back.ex_status.fs, &s.ex_status.fs, sizeof s.ex_status.fs) == 0 && back.unk_EC == 7,
          "decode(encode(x)) == x");
    KPADStatus bad = s;
    unsigned int nan = 0x7fc00000u;
    std::memcpy(&bad.acc.x, &nan, 4);
    check(!KpadRecord::encodeSample(bad, bytes), "non-finite float is refused, not hidden");
    KPADStatus odd = s;
    odd.dev_type = 3;
    check(!KpadRecord::encodeSample(odd, bytes), "unsupported extension union is refused");

    // Files: header, one read, a mod line.
    const auto dir = std::filesystem::temp_directory_path() / ("kpad_record_test_" + std::to_string(getpid()));
    std::filesystem::create_directories(dir);
    const std::string base = (dir / "t").string();
    std::string error;
    check(KpadRecord::open(base, &error), "open");
    check(!KpadRecord::open(base, &error), "no second open");
    KpadRecord::read(0, 120, 1, 0, 0, 1, &s);
    KpadRecord::modPress(3, true);
    KpadRecord::modHeld(3, true);
    KpadRecord::modHeld(3, false);  // only the first answer of the frame counts
    KpadRecord::seam();
    KpadRecord::read(1, 120, -2, -2, -1, 253, &s);  // error return still carries slot 0
    KpadRecord::seam();
    KpadRecord::close();
    std::ifstream kpd(base + ".kpd", std::ios::binary);
    std::string data((std::istreambuf_iterator<char>(kpd)), std::istreambuf_iterator<char>());
    check(data.compare(0, 8, "SMG2KPD1") == 0, "magic");
    check(be(data, 8, 8) == 3, "frames = loops + 1");
    check(be(data, 16, 8) == 2, "two reads");
    check(data.size() == 24 + 2 * (40 + 240), "record sizes");
    check(be(data, 24, 8) == 0 && be(data, 24 + 8, 4) == 0 && be(data, 24 + 12, 4) == 0 && be(data, 24 + 16, 4) == 120, "first read: frame 0 ordinal 0 channel 0 capacity 120");
    const size_t second = 24 + 40 + 240;
    check(be(data, second, 8) == 1 && be(data, second + 8, 4) == 0 && be(data, second + 12, 4) == 1, "second read: frame 1, ordinal reset, channel 1");
    std::ifstream act(base + ".actions");
    std::string text((std::istreambuf_iterator<char>(act)), std::istreambuf_iterator<char>());
    check(text.find("\n0 3 1 1\n") != std::string::npos, "mod 3: press and the first held answer in frame 0");
    check(KpadRecord::stats().reads == 2 && KpadRecord::stats().lossy == 0 && KpadRecord::stats().unencodable == 0, "stats");
    std::filesystem::remove_all(dir);
    if (argc > 1 && std::string(argv[1]) == "--bench") {
        // Cost of the recorder per game loop: 4 channel reads (one sample each), a mod press/held query pair, the seam.
        const auto bdir = std::filesystem::temp_directory_path() / ("kpad_record_bench_" + std::to_string(getpid()));
        std::filesystem::create_directories(bdir);
        KpadRecord::open((bdir / "t").string(), &error);
        constexpr int kFrames = 200000;
        const auto t0 = std::chrono::steady_clock::now();
        for (int f = 0; f < kFrames; ++f) {
            for (int c = 0; c < 4; ++c) KpadRecord::read(c, 120, c == 0 ? 1 : 0, c == 0 ? 0 : -2, c == 0 ? 0 : -1, c == 0 ? 1 : 253, &s);
            KpadRecord::modPress(3, f % 90 == 0);
            KpadRecord::modHeld(3, f % 90 < 5);
            KpadRecord::seam();
            if (f % 60 == 59) KpadRecord::checkpoint();
        }
        const auto t1 = std::chrono::steady_clock::now();
        KpadRecord::close();
        const double us = std::chrono::duration<double, std::micro>(t1 - t0).count() / kFrames;
        std::printf("recorder cost: %.2f us per game loop (4 reads + mod answers + seam, checkpoint every 60), %.4f%% of a 16.68 ms frame\n", us, us / 166.8);
        std::filesystem::remove_all(bdir);
    }
    if (failures == 0) std::puts("kpad record tests passed");
    return failures == 0 ? 0 : 1;
}
