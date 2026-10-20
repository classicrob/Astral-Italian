#include <petari/utf16.h>
#include "Game/Util/StringUtil.hpp"
#include <cstdio>
#include <cstring>

static_assert(sizeof(PetariChar16) == 2, "Legacy game text must stay UTF-16");
static_assert(sizeof(wchar_t) == 4, "The host wide-character ABI must stay intact");

namespace {
int failures;
void check(const PetariChar16* text, int capacity, const char* expected, int length) {
    char output[32];
    std::memset(output, 0x55, sizeof(output));
    const int result = MR::convertUTF16ToASCII(output, text, capacity);
    if (result != length || std::memcmp(output, expected, length + 1) != 0 ||
        static_cast<unsigned char>(output[length + 1]) != 0x55) {
        std::fprintf(stderr, "Incorrect conversion: capacity=%d expected length=%d actual=%d\n", capacity, length, result);
        ++failures;
    }
}
}

int main() {
    check(u"StarPiece", 32, "StarPiece", 9);
    check(u"", 32, "", 0);
    check(u"StarPiece", 5, "Star", 4);
    check(u"StarPiece", 1, "", 0);
    check(u"A\u0100B", 32, "A", 1);
    check(u"\u2605", 32, "", 0);
    check(u"\u0100", 32, "", 0); // zero low byte must not become an empty accepted unit
    check(u"\u0180", 32, "", 0);
    check(u"\u00ff\u0080", 32, "\xff\x80", 2); // retain the DOL's full byte range
    const PetariChar16 embeddedNull[] = {u'A', 0, u'B', 0};
    check(embeddedNull, 32, "A", 1);
    const PetariChar16 surrogate[] = {0xD83D, 0xDE00, 0};
    check(surrogate, 32, "", 0);
    // A source that has no terminator: output capacity must stop the read first.
    const PetariChar16 exactSource[] = {u'A', u'B', u'C'};
    check(exactSource, 4, "ABC", 3);
    if (failures) return 1;
    std::puts("UTF-16 conversion: 12 host-order, range and capacity cases passed");
}
