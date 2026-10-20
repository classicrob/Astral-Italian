#ifdef GALAXY_NATIVE
#include "Game/System/NativeSaveMigration.hpp"
#include "Game/System/SaveDataHandler.hpp"
#include "Game/System/NANDManager.hpp"
#include "Game/System/SaveDataFileAccessor.hpp"
#include "Game/System/SysConfigFile.hpp"
#include "Game/System/UserFile.hpp"
#include <petari/save_backup.hpp>
#include <petari/host_allocation.hpp>
#include <petari/platform/nand.hpp>
#include <revolution/nand.h>
#include <cstdio>
#include <cstdlib>

namespace NativeSaveMigration {
namespace {
PetariNative::SaveWire::Bytes sRoundTrip;
std::filesystem::path savePath() {
    char home[64]{};
    PetariNative::SaveWire::require(NANDGetHomeDir(home) == NAND_RESULT_OK, "cannot resolve save title directory");
    const auto path = PetariNative::Platform::NAND::hostPath(home) / "GameData.bin";
    const char* userHome = std::getenv("HOME");
    PetariNative::SaveWire::require(userHome != nullptr, "HOME is required to protect SMG1 saves");
    const auto forbidden = std::filesystem::weakly_canonical(std::filesystem::path(userHome) / "Library/Application Support/Petari");
    const auto relative = std::filesystem::weakly_canonical(path).lexically_relative(forbidden);
    PetariNative::SaveWire::require(relative.empty() || *relative.begin() == "..", "SMG1 saves are never a migration destination");
    return path;
}
void checkTestRoot() {
    using namespace PetariNative::SaveWire;
    const char* enabled = std::getenv("ASTRAL_SAVE_WIRE_ROUNDTRIP");
    if (!enabled || std::strcmp(enabled, "1") != 0) return;
    const char* home = std::getenv("HOME");
    require(home != nullptr, "HOME is required for private save tests");
    const auto root = PetariNative::Platform::NAND::hostPath("/").parent_path();
    const auto expected = std::filesystem::path(home) / "Library/Application Support/AstralItalian-unlocked/save-wire-validation";
    require(std::filesystem::canonical(expected) == expected, "round-trip test root must not be a symlink");
    const auto relative = std::filesystem::canonical(root).lexically_relative(expected);
    require(!relative.empty() && *relative.begin() != "..", "round-trip hook requires private unlocked/save-wire-validation user directory");
}

}
void prepareLoaded(u8* bytes, u32 length) {
    PetariNative::HostAllocationScope allocations;
    using namespace PetariNative::SaveWire;
    try {
        checkTestRoot();
        require(length >= 16 && length <= 0x8000, "invalid loaded save length");
        Bytes original(bytes, bytes + length);
        const bool legacy = little32(bytes + 4) == 3;
        auto converted = canonical(original, legacy);
        if (legacy) {
            migrateFile(savePath(), original, converted);
            std::memcpy(bytes, converted.data(), converted.size());
            std::fprintf(stderr, "ASTRAL SAVE MIGRATED: verified GameData.bin.le-backup; Wii big-endian file reloaded\n");
        }
    } catch (const std::exception& error) {
        std::fprintf(stderr, "ASTRAL SAVE REFUSED: %s; no reset or new save permitted\n", error.what());
        std::abort();
    }
}

void checkRoundTrip(SaveDataHandler& handler) {
    const char* enabled = std::getenv("ASTRAL_SAVE_WIRE_ROUNDTRIP");
    if (!enabled || std::strcmp(enabled, "1") != 0) return;
    PetariNative::HostAllocationScope allocations;
    using namespace PetariNative::SaveWire;
    try {
        const auto file = savePath();
        checkTestRoot();
        const auto original = readRegular(file);
        const auto records = directory(original, false);
        Bytes result = original;
        // Exercise actual chunk deserializers and serializers without gameplay/timestamp updates.
        for (unsigned slot = 1; slot <= 3; ++slot) {
            UserFile user;
            const auto& game = records[(slot - 1) * 2];
            const auto& config = records[(slot - 1) * 2 + 1];
            user.loadFromGameDataBinary(game.name.c_str(), original.data() + game.offset, game.size);
            user.loadFromConfigDataBinary(config.name.c_str(), original.data() + config.offset, config.size);
            require(!user.mIsGameDataCorrupted && !user.mIsConfigDataCorrupted, "game rejected round-trip input");
            user.makeGameDataBinary(result.data() + game.offset, game.size);
            user.makeConfigDataBinary(result.data() + config.offset, config.size);
        }
        SysConfigFile system;
        const auto& record = records.back();
        system.loadFromDataBinary(original.data() + record.offset, record.size);
        system.makeDataBinary(result.data() + record.offset, record.size);
        require(result == original, "game serializer round-trip changed Wii bytes");
        std::memcpy(handler._18, result.data(), result.size());
        sRoundTrip = original;
        handler.requestSaveSaveData();
        require(std::memcmp(handler._14, original.data(), original.size()) == 0, "normal save writer changed Wii bytes");
        std::fprintf(stderr, "ASTRAL SAVE WII SERIALIZERS: all seven game records and normal writer byte-identical (%zu bytes)\n", original.size());
    } catch (const std::exception& error) {
        std::fprintf(stderr, "ASTRAL SAVE ROUNDTRIP FAILED: %s\n", error.what());
        std::abort();
    }
}
void pollRoundTrip(SaveDataHandler& handler) {
    if (sRoundTrip.empty() || !handler.isDone()) return;
    PetariNative::HostAllocationScope allocations;
    try {
        PetariNative::SaveWire::require(handler.getLastResultCode().isSuccess(), "normal NAND writer failed");
        PetariNative::SaveWire::require(PetariNative::SaveWire::readRegular(savePath()) == sRoundTrip,
                                       "normal NAND write differs from round-trip input");
        std::fprintf(stderr, "ASTRAL SAVE WII ROUNDTRIP: normal NAND file byte-identical (%zu bytes)\n", sRoundTrip.size());
        sRoundTrip.clear();
    } catch (const std::exception& error) {
        std::fprintf(stderr, "ASTRAL SAVE ROUNDTRIP FAILED: %s\n", error.what());
        std::abort();
    }
}
} // namespace NativeSaveMigration
#endif
