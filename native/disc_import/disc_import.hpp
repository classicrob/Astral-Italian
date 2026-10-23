#pragma once
// Disc import: turn the player's own SB4E01 disc image (or an already extracted folder) into the extracted
// tree the game reads (files/ and sys/, the layout DVD::mount expects), and tell the first-run setup what
// is wrong when it cannot. No game data is embedded here; every byte comes from the player's disc.
//
// The implementation reads disc formats through encounter/nod (the library Aurora uses for its DVD module).
// Interface agreed between the importer (native/disc_import/*.cpp) and the first-run UI (native/app/first_run*).

#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>

namespace PetariNative::DiscImport {

enum class Format {
    Unknown,
    GcmIso,           // plain GameCube/Wii disc image (the common .iso)
    Wbfs,             // Wii Backup File System container (.wbfs)
    Rvz,              // Dolphin RVZ/WIA (.rvz, .wia)
    ExtractedFolder,  // a directory holding files/ (and sys/), as a disc extractor writes it
};

struct Probe {
    Format format = Format::Unknown;
    std::string gameId;        // six characters from the disc header, e.g. "SB4E01"
    int revision = -1;         // disc header version byte
    std::string title;         // disc header game name
    std::uint64_t sourceBytes = 0;
    bool isSupportedGame = false;  // gameId == "SB4E01" and revision 0 (USA, Super Mario Galaxy 2)
    // Non-empty when the file cannot be imported. Plain, player-facing English that says what to do next
    // ("This is a Japanese disc (SB4J01); Astral Italian needs the USA disc (SB4E01)", "RVZ files are not
    // supported yet: convert it to ISO in Dolphin").
    std::string problem;
};

// Reads only headers: fast, never writes. Safe on any path (a missing file, a directory, a text file).
Probe probe(const std::filesystem::path& source);

struct Progress {
    std::uint64_t doneBytes = 0;
    std::uint64_t totalBytes = 0;
    std::string currentFile;  // path inside the disc
};
// Called from the importing thread, at most about 10 times a second. Return false to cancel.
using ProgressFn = std::function<bool(const Progress&)>;

enum class Result { Ok, Cancelled, Failed };

// Extracts source into destRoot (created; must not exist or must be empty): destRoot/files/..., destRoot/sys/
// {boot.bin, bi2.bin, apploader.img, fst.bin, main.dol}. Works in destRoot's sibling "<name>.partial" and renames
// it into place only after validateExtracted() passes, so a cancelled or failed import leaves nothing behind.
// For an ExtractedFolder source, nothing is copied: use validateExtracted() and keep the path.
// On Failed, *error is a player-facing sentence (disk full, unreadable disc, not an SB4E01 disc, bad data...).
Result importDisc(const std::filesystem::path& source, const std::filesystem::path& destRoot, const ProgressFn& progress,
                  std::string* error);

// True when root is a complete, consistent SB4E01 extraction: files/ present, sys/boot.bin names SB4E01
// revision 0, sys/main.dol has the expected SHA-1, and every file listed by sys/fst.bin exists with its size.
// With quick, only the header, main.dol and a fixed list of key files are checked (used at every launch);
// without it every FST entry is checked.
bool validateExtracted(const std::filesystem::path& root, bool quick, std::string* error);

}  // namespace PetariNative::DiscImport
