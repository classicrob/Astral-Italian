#include <petari/complete_save_paths.hpp>
#include <cassert>
#include <filesystem>
#include <iostream>

int main(int argc, char** argv) {
    namespace fs = std::filesystem;
    assert(argc == 2);
    const fs::path home = fs::weakly_canonical(fs::absolute(argv[1]));
    assert(!fs::exists(home));
    fs::create_directories(home / "Library/Application Support");
    const auto base = home / "Library/Application Support/AstralItalian-unlocked";
    using PetariNative::CompleteSave::isIsolatedUserAt;
    assert(isIsolatedUserAt(base, home));
    assert(isIsolatedUserAt(base / "validation", home));
    assert(!isIsolatedUserAt(home / "Library/Application Support/Petari", home));
    assert(!isIsolatedUserAt(home / "Library/Application Support/Astral Italian", home));
    assert(!isIsolatedUserAt(home / "MacioGalaxy2-play/user", home));
    assert(!isIsolatedUserAt(home / "Library/Application Support/AstralItalian-unlocked-other", home));
    assert(!isIsolatedUserAt(base / "../Astral Italian", home));
    fs::create_directory_symlink(home / "Library/Application Support", base);
    assert(!isIsolatedUserAt(base, home));
    fs::remove(base);
    fs::create_directories(base);
    fs::create_directory_symlink(home / "Library/Application Support", base / "escape");
    assert(!isIsolatedUserAt(base / "escape", home));
    fs::remove_all(home);
    std::cout << "complete-save isolation paths PASS\n";
}
