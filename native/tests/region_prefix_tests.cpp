// Minimal language-state fixtures, using the production class declarations.
// Their test constructors avoid starting the game's archive/DVD/NAND services.
#include "Game/System/GameSystem.hpp"
#include "Game/System/GameSystemObjHolder.hpp"
#include "Game/System/Language.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include <cstdio>
#include <cstring>

GameSystem::GameSystem() : NerveExecutor("region test"), mObjHolder(nullptr) {}
GameSystemObjHolder::GameSystemObjHolder() : mRandom(0), mLanguage(0) {}

int main() {
    GameSystem system;
    GameSystemObjHolder objects;
    system.mObjHolder = &objects;
    SingletonHolder<GameSystem>::set(&system);
    const char* expected[] = {"Jp", "Us", "Us", "Us", "Eu", "Eu", "Eu", "Eu", "Eu", "Eu", "Cn", "Cn", "Kr"};
    for (u32 i = 0; i < MR::getLanguageNum(); ++i) {
        // Map the actual language-name table to the corresponding public language ID.
        const u32 ids[] = {LANGUAGE_JPJAPANESE, LANGUAGE_USENGLISH, LANGUAGE_USSPANISH,
            LANGUAGE_USFRENCH, LANGUAGE_EUENGLISH, LANGUAGE_EUSPANISH, LANGUAGE_EUFRENCH,
            LANGUAGE_EUGERMAN, LANGUAGE_EUITALIAN, LANGUAGE_EUDUTCH, LANGUAGE_CNSIMPCHINESE,
            LANGUAGE_CNTRADCHINESE, LANGUAGE_KRKOREAN};
        objects.mLanguage = ids[i];
        const char* region = MR::getCurrentRegionPrefix();
        if (region == nullptr || std::strcmp(region, expected[i]) != 0) {
            std::fprintf(stderr, "Incorrect region for %s\n", MR::getLanguagePrefixByIndex(i));
            return 1;
        }
    }
    objects.mLanguage = 0xFFFFFFFF;
    if (MR::getCurrentRegionPrefix() != nullptr) return 1;
    SingletonHolder<GameSystem>::set(nullptr);
    std::puts("region prefixes: all 13 languages and fallback passed");
}
