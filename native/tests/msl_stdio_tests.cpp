// MSL printf semantics of petari/msl_stdio.h (legacy narrow formatting).
#include <petari/msl_stdio.h>

#include <cstdio>
#include <cstring>

static int sFailures;

static void check(bool ok, const char* what) {
    if (!ok) {
        std::fprintf(stderr, "FAIL: %s\n", what);
        sFailures++;
    }
}

static bool is(const char* got, const char* want) {
    return std::strcmp(got, want) == 0;
}

int main() {
    char buffer[64];
    const char* none = nullptr;
    check(petari_msl_snprintf(buffer, sizeof(buffer), "InitSensor%s.bcsv", none) == 15 && is(buffer, "InitSensor.bcsv"),
          "a null %s prints nothing");
    check(petari_msl_snprintf(buffer, sizeof(buffer), "[%5s|%-4s]", none, "ab") == 12 && is(buffer, "[     |ab  ]"), "null %s pads");
    check(petari_msl_snprintf(buffer, sizeof(buffer), "%s/%s.arc", "ObjectData", "Coin") == 19 && is(buffer, "ObjectData/Coin.arc"),
          "strings");
    check(petari_msl_snprintf(buffer, sizeof(buffer), "%.3s", "abcdef") == 3 && is(buffer, "abc"), "string precision");
    check(petari_msl_snprintf(buffer, sizeof(buffer), "%d %03d %-3d| %+d %x %X %o %u", -5, 7, 4, 9, 255, 255, 8, 4000000000u) > 0 &&
              is(buffer, "-5 007 4  | +9 ff FF 10 4000000000"),
          "integers");
    check(petari_msl_snprintf(buffer, sizeof(buffer), "%ld %lu %lx", -12, 3000000000u, 0xABCDu) > 0 && is(buffer, "-12 3000000000 abcd"),
          "l integers read Wii long (32 bits)");
    check(petari_msl_snprintf(buffer, sizeof(buffer), "%lld %hhd %hd", -9000000000LL, 300, 70000) > 0 && is(buffer, "-9000000000 44 4464"),
          "ll/hh/h");
    check(petari_msl_snprintf(buffer, sizeof(buffer), "%*d|%-*d|%.*f", 4, 1, 3, 2, 2, 3.14159) > 0 && is(buffer, "   1|2  |3.14"), "star");
    check(petari_msl_snprintf(buffer, sizeof(buffer), "%5.1f%%", 12.25) > 0 && is(buffer, " 12.2%"), "float and %%");
    check(petari_msl_snprintf(buffer, sizeof(buffer), "%c%c", 'O', 'K') == 2 && is(buffer, "OK"), "chars");
    const char16_t* wide = u"Mario";
    const char16_t* noWide = nullptr;
    check(petari_msl_snprintf(buffer, sizeof(buffer), "%ls|%ls|", wide, noWide) == 7 && is(buffer, "Mario||"), "%ls and null %ls");
    check(petari_msl_snprintf(buffer, 6, "%s", "truncated") == 9 && is(buffer, "trunc"), "truncation returns the full length");
    check(petari_msl_snprintf(nullptr, 0, "%d", 12345) == 5, "size query");
    int count = 0;
    check(petari_msl_snprintf(buffer, sizeof(buffer), "abc%n%d", &count, 1) == 4 && count == 3 && is(buffer, "abc1"), "%n");
    check(petari_msl_snprintf(buffer, sizeof(buffer), "a%qb%d", 1) == 6 && is(buffer, "a%qb%d"), "invalid conversion copies the rest");
    check(petari_msl_sprintf(buffer, "%s-%d", "Zone", 3) == 6 && is(buffer, "Zone-3"), "sprintf");
    if (sFailures == 0) {
        std::printf("msl stdio tests passed\n");
    }
    return sFailures == 0 ? 0 : 1;
}
