// The native-only host-file API must exclude directories in every mount mode.
// Synthetic extracted disc only: no game assets or user caches.
#include <petari/platform/dvd.hpp>
#include <revolution/dvd.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <unistd.h>

extern "C" void __OSThreadInit(void);
namespace fs = std::filesystem;
namespace DVD = PetariNative::Platform::DVD;

namespace {
void check(bool ok, const char* why) {
    if (!ok) throw std::runtime_error(why);
}
struct Disc {
    fs::path root;
    Disc() {
        std::string pattern = (fs::temp_directory_path() / "petari-dvd-host-path-XXXXXX").string();
        std::vector<char> path(pattern.begin(), pattern.end());
        path.push_back('\0');
        const char* made = ::mkdtemp(path.data());
        check(made != nullptr, "private temporary root");
        root = made;
        fs::create_directories(root / "files/Folder");
        fs::create_directories(root / "sys");
        std::ofstream(root / "files/Folder/child.bin", std::ios::binary) << "abc";
        std::ofstream(root / "files/root.bin", std::ios::binary) << "root";
        std::ofstream(root / "replacement.bin", std::ios::binary) << "MOD!";
        // Four Wii FST entries: root, Folder, child.bin, root.bin.
        // Names and word offsets are unrelated to the host filesystem.
        std::vector<std::uint8_t> fst;
        auto word = [&](std::uint32_t value) {
            for (int shift = 24; shift >= 0; shift -= 8) fst.push_back(static_cast<std::uint8_t>(value >> shift));
        };
        word(0x01000000); word(0); word(4);
        word(0x01000001); word(0); word(3);
        word(8); word(0x1000); word(3);
        word(18); word(0x2000); word(4);
        const char names[] = "\0Folder\0child.bin\0root.bin";
        fst.insert(fst.end(), names, names + sizeof(names));
        std::ofstream out(root / "sys/fst.bin", std::ios::binary);
        out.write(reinterpret_cast<const char*>(fst.data()), static_cast<std::streamsize>(fst.size()));
        check(out.good(), "write synthetic FST");
    }
    ~Disc() {
        DVD::shutdown();
        std::error_code error;
        fs::remove_all(root, error); // This object's unique mkdtemp root only.
    }
};
void checkMode(const Disc& disc, bool scanned, bool overlaid) {
    DVD::MountOptions options{disc.root};
    options.ignoreDiscFst = scanned;
    if (overlaid) options.overlay.push_back({"root.bin", disc.root / "replacement.bin", "test"});
    std::string error;
    const bool mounted = DVD::mount(options, &error);
    check(mounted, error.c_str());
    const auto info = DVD::mountInfo();
    check(info.source == (scanned ? DVD::FstSource::DirectoryScan : DVD::FstSource::DiscFst), "actual FST mount mode");
    if (overlaid) check(info.overlayReplaced == 1, "overlay builder exercised");
    const auto directory = DVDConvertPathToEntrynum("/Folder");
    const auto child = DVDConvertPathToEntrynum("/Folder/child.bin");
    const auto file = DVDConvertPathToEntrynum("/root.bin");
    check(directory > 0 && child > 0 && file > 0, "existing directory and files resolve");
    check(DVD::hostPathForEntry(0).empty(), "root is not a host file");
    check(DVD::hostPathForEntry(directory).empty(), "directory is not a host file");
    check(DVD::hostPathForEntry(-1).empty(), "negative entry is not a host file");
    check(DVD::hostPathForEntry(static_cast<std::int32_t>(info.entryCount)).empty(), "one-past-end entry is not a host file");
    check(DVD::hostPathForEntry(99).empty(), "invalid entry is not a host file");
    check(DVD::hostPathForEntry(child) == disc.root / "files/Folder/child.bin", "nested regular file keeps its backing path");
    check(DVD::hostPathForEntry(file) == disc.root / (overlaid ? "replacement.bin" : "files/root.bin"), "regular file keeps disc or replacement path");
    DVDFileInfo opened{};
    check(!DVDFastOpen(directory, &opened), "SDK file open also rejects a directory");
    check(DVDFastOpen(file, &opened), "host-file consumer can open the regular entry");
    DVD::shutdown();
    check(DVD::hostPathForEntry(file).empty(), "unmounted entry is not a host file");
}
}
int main() try {
    __OSThreadInit();
    Disc disc;
    checkMode(disc, false, false);
    checkMode(disc, true, false);
    checkMode(disc, false, true);
    std::puts("DVD host file paths: disc-FST / scan / overlay PASS");
    return 0;
} catch (const std::exception& error) {
    std::fprintf(stderr, "FAIL: %s\n", error.what());
    return 1;
}
