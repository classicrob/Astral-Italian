#include "disc_import.hpp"
#include "disc_import_internal.hpp"
#include "dvd_fst.hpp"

#include <CommonCrypto/CommonDigest.h>
#include <nod.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cerrno>
#include <cstring>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

namespace fs = std::filesystem;
namespace PetariNative::DiscImport {
namespace {
using Platform::DVD::Fst;
constexpr const char* kDolSha1 = "7d79810889824edd01c4ff02624713060a69818c";
constexpr std::uint64_t kReserve = 1024ull * 1024 * 1024;
constexpr std::uintmax_t kMaxMetadata = 64 * 1024 * 1024;
using Handle = std::unique_ptr<NodHandle, decltype(&nod_free)>;

void setError(std::string* error, const std::string& message) {
    if (error) *error = message;
}
void need(bool ok, const std::string& message) {
    if (!ok) throw std::runtime_error(message);
}
Handle openDisc(const fs::path& path) {
    NodHandle* raw = nullptr;
    NodDiscOptions options{};
    // One importing thread; nod still handles compression, encryption and integrity.
    NodResult result = nod_disc_open(path.c_str(), &options, &raw);
    Handle handle(raw, nod_free);
    need(result == NOD_RESULT_OK && handle, "This disc image is incomplete, damaged, or unreadable. Make a new copy of your disc and try again.");
    return handle;
}
std::vector<std::uint8_t> readFile(const fs::path& path, std::uintmax_t limit) {
    std::error_code ec;
    need(fs::is_regular_file(fs::symlink_status(path, ec)), "The extracted disc is missing " + path.filename().string() + ". Extract the entire disc again.");
    const auto size = fs::file_size(path);
    need(size <= limit, "The extracted disc has an invalid " + path.filename().string() + ". Extract the entire disc again.");
    std::vector<std::uint8_t> bytes(size);
    std::ifstream in(path, std::ios::binary);
    need(bool(in) && bool(in.read(reinterpret_cast<char*>(bytes.data()), bytes.size())), "Could not read " + path.filename().string() + ". Check that you can open this folder.");
    return bytes;
}
Probe headerProbe(const std::uint8_t* header, std::size_t size) {
    Probe out;
    if (size < 0x60) {
        out.problem = "This disc header is incomplete. Extract the entire disc again.";
        return out;
    }
    out.gameId.assign(reinterpret_cast<const char*>(header), 6);
    out.revision = header[7];
    out.title.assign(reinterpret_cast<const char*>(header + 0x20), strnlen(reinterpret_cast<const char*>(header + 0x20), 64));
    const bool wii = Platform::DVD::readU32BE(header + 0x18) == 0x5d1c9ea3;
    const bool gc = Platform::DVD::readU32BE(header + 0x1c) == 0xc2339f3d;
    const std::string identity = (out.title.empty() ? "an unnamed game" : out.title) + " (" + out.gameId + "), revision " + std::to_string(out.revision);
    if (gc && !wii) {
        out.problem = "This looks like " + identity + ", a GameCube disc. Choose your USA Super Mario Galaxy 2 Wii disc (SB4E01, revision 0).";
    } else if (!wii) {
        out.problem = "This is not a Wii disc image. Choose an image made from your USA Super Mario Galaxy 2 disc.";
    } else if (out.gameId != "SB4E01" || out.revision != 0) {
        out.problem = "This looks like " + identity + ". Astral Italian needs the USA Super Mario Galaxy 2 disc (SB4E01, revision 0).";
    } else {
        out.isSupportedGame = true;
    }
    return out;
}
std::string sha1(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);
    need(bool(in), "Could not read sys/main.dol. Extract the entire disc again.");
    CC_SHA1_CTX context;
    CC_SHA1_Init(&context);
    std::vector<char> buffer(1024 * 1024);  // heap: importers run on threads with small stacks
    while (in) {
        in.read(buffer.data(), buffer.size());
        CC_SHA1_Update(&context, buffer.data(), static_cast<CC_LONG>(in.gcount()));
    }
    need(in.eof(), "Could not finish reading sys/main.dol. Check that the disc folder is readable.");
    unsigned char digest[CC_SHA1_DIGEST_LENGTH];
    CC_SHA1_Final(digest, &context);
    std::string out;
    for (unsigned char byte : digest) {
        out += "0123456789abcdef"[byte >> 4];
        out += "0123456789abcdef"[byte & 15];
    }
    return out;
}
struct Partial {
    fs::path path;
    bool owned = false;
    ~Partial() {
        if (owned) {
            std::error_code ec;
            fs::remove_all(path, ec);
        }
    }
};
struct Cancelled {};
class Reporter {
public:
    explicit Reporter(const ProgressFn& fn) : mFn(fn) {}
    Progress state;
    void update(bool force = false) {
        auto now = std::chrono::steady_clock::now();
        if (mFn && (force || now - mLast >= std::chrono::milliseconds(100))) {
            mLast = now;
            if (!mFn(state)) throw Cancelled{};
        }
    }
private:
    const ProgressFn& mFn;
    std::chrono::steady_clock::time_point mLast{};
};
// Exclusive creation also rejects duplicate FST names and case collisions on macOS.
class Output {
public:
    explicit Output(const fs::path& path) : mFd(::open(path.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW, 0600)) {
        need(mFd >= 0, "Could not create " + path.filename().string() + ". Check free space, folder permissions, and disc file names.");
    }
    ~Output() { if (mFd >= 0) ::close(mFd); }
    void write(const std::uint8_t* bytes, std::size_t size) {
        while (size) {
            auto count = ::write(mFd, bytes, size);
            if (count < 0 && errno == EINTR) continue;
            need(count > 0, "Could not save the disc files. Free some disk space and try again.");
            bytes += count;
            size -= count;
        }
    }
    void close() {
        const int fd = mFd;
        mFd = -1;
        need(::close(fd) == 0, "Could not finish saving the disc files. Check free disk space and try again.");
    }
private:
    int mFd;
};
}

Probe probe(const fs::path& source) {
    Probe out;
    try {
        if (fs::is_directory(source)) {
            auto bytes = readFile(source / "sys/boot.bin", 0x440);
            need(bytes.size() == 0x440, "sys/boot.bin is incomplete. Extract the entire disc again.");
            out = headerProbe(bytes.data(), bytes.size());
            out.format = Format::ExtractedFolder;
            return out;
        }
        need(fs::is_regular_file(source), "Choose a disc image file or a folder containing an entire extracted disc.");
        out.sourceBytes = fs::file_size(source);
        std::array<std::uint8_t, 0x440> prefix{};
        std::ifstream in(source, std::ios::binary);
        in.read(reinterpret_cast<char*>(prefix.data()), prefix.size());
        const bool recognized = Platform::DVD::readU32BE(prefix.data() + 0x18) == 0x5d1c9ea3 ||
                                Platform::DVD::readU32BE(prefix.data() + 0x1c) == 0xc2339f3d ||
                                std::memcmp(prefix.data(), "WBFS", 4) == 0 || std::memcmp(prefix.data(), "RVZ", 3) == 0 ||
                                std::memcmp(prefix.data(), "WIA", 3) == 0 || std::memcmp(prefix.data(), "CISO", 4) == 0;
        NodHandle* raw = nullptr;
        NodDiscOptions options{};
        auto result = nod_disc_open(source.c_str(), &options, &raw);
        Handle disc(raw, nod_free);
        need(result == NOD_RESULT_OK && disc, recognized ?
             "This disc image is incomplete or damaged. Make a new copy of your disc and try again." :
             "This is not a readable disc image. Choose an ISO, WBFS, RVZ, WIA, or another disc format supported by nod.");
        NodDiscHeader header{};
        NodDiscMeta meta{};
        need(nod_disc_header(disc.get(), &header) == NOD_RESULT_OK && nod_disc_meta(disc.get(), &meta) == NOD_RESULT_OK,
             "Could not read the disc header. Make a new copy of your disc and try again.");
        out = headerProbe(reinterpret_cast<const std::uint8_t*>(&header), sizeof(header));
        out.sourceBytes = fs::file_size(source);
        switch (meta.format) {
        case NOD_FORMAT_WBFS: out.format = Format::Wbfs; break;
        case NOD_FORMAT_RVZ: case NOD_FORMAT_WIA: out.format = Format::Rvz; break;
        default: out.format = Format::GcmIso; break;
        }
    } catch (const fs::filesystem_error&) {
        out.isSupportedGame = false;
        out.problem = "Could not open this file or folder. Check that it exists and that you have permission to read it.";
    } catch (const std::exception& e) {
        out.isSupportedGame = false;
        out.problem = e.what();
    }
    return out;
}

bool Detail::validateTree(const fs::path& root, bool quick, std::string* error, const char* expectedSha1) {
    setError(error, "");
    try {
        need(fs::is_directory(fs::symlink_status(root / "files")), "This folder is missing files/. Extract the entire DATA partition of your disc.");
        need(fs::is_directory(fs::symlink_status(root / "sys")), "This folder is missing sys/. Extract the entire DATA partition of your disc.");
        auto boot = readFile(root / "sys/boot.bin", 0x440);
        need(boot.size() == 0x440, "sys/boot.bin is incomplete. Extract the entire disc again.");
        auto info = headerProbe(boot.data(), boot.size());
        need(info.isSupportedGame, info.problem);
        need(readFile(root / "sys/bi2.bin", 0x2000).size() == 0x2000, "sys/bi2.bin is incomplete. Extract the entire disc again.");
        auto app = readFile(root / "sys/apploader.img", kMaxMetadata);
        need(app.size() >= 0x20 && app.size() == 0x20ull + Platform::DVD::readU32BE(app.data() + 0x14) +
             Platform::DVD::readU32BE(app.data() + 0x18), "sys/apploader.img is incomplete. Extract the entire disc again.");
        need(fs::is_regular_file(fs::symlink_status(root / "sys/main.dol")) && fs::file_size(root / "sys/main.dol") <= kMaxMetadata,
             "sys/main.dol is missing or damaged. Extract the entire disc again.");
        need(sha1(root / "sys/main.dol") == expectedSha1,
             "sys/main.dol does not match the USA Super Mario Galaxy 2 disc (revision 0). Extract an unmodified copy of that disc.");
        auto image = readFile(root / "sys/fst.bin", kMaxMetadata);
        need(image.size() == std::uint64_t(Platform::DVD::readU32BE(boot.data() + 0x428)) * 4,
             "sys/fst.bin has the wrong size. Extract the entire disc again.");
        Fst fst;
        const bool parsed = Fst::fromDiscImage(image, root / "files", fst, nullptr, !quick);
        need(parsed, "Some extracted files are missing, damaged, or have unsafe names. Extract the entire disc again.");
        if (quick) {
            // A small fixed set needed by startup, checked against the disc's own sizes.
            for (const char* path : {"SystemData/ObjNameTable.arc", "ObjectData/Mario.arc"}) {
                auto index = fst.convertPathToEntrynum(path, 0);
                need(index > 0 && !fst.entry(index).isDir, "The disc is missing a required game file. Extract the entire disc again.");
                const auto& host = fst.hostPath(index);
                need(fs::is_regular_file(fs::symlink_status(host)) && fs::file_size(host) == fst.entry(index).nextOrLength,
                     std::string("The extracted disc is missing or has a damaged ") + path + ". Extract the entire disc again.");
            }
        }
        return true;
    } catch (const fs::filesystem_error&) {
        setError(error, "Could not read the extracted disc folder. Check that all disc files are present and readable, or extract the entire disc again.");
        return false;
    } catch (const std::exception& e) {
        setError(error, std::string("Could not validate the extracted disc. ") + e.what());
        return false;
    }
}

bool validateExtracted(const fs::path& root, bool quick, std::string* error) {
    return Detail::validateTree(root, quick, error, kDolSha1);
}

Result Detail::importWithSpace(const fs::path& source, const fs::path& destRoot, const ProgressFn& progress,
                              std::string* error, const FreeSpaceFn& freeSpace) {
    setError(error, "");
    Partial partial{fs::path(destRoot.string() + ".partial")};
    try {
        const auto info = probe(source);
        need(info.isSupportedGame, info.problem);
        if (info.format == Format::ExtractedFolder) {
            return validateExtracted(source, false, error) ? Result::Ok : Result::Failed;
        }
        const auto dest = fs::absolute(destRoot).lexically_normal();
        need(dest.has_filename() && dest.filename() != "." && dest.filename() != "..", "Choose a new, empty folder for the disc files.");
        const auto status = fs::symlink_status(dest);
        const bool exists = fs::exists(status);
        need(!exists || (fs::is_directory(status) && fs::is_empty(dest)), "The destination folder must be empty. Choose a new folder for the disc files.");
        partial.path = fs::path(dest.string() + ".partial");
        need(!fs::exists(fs::symlink_status(partial.path)), "An unfinished import already exists at " + partial.path.string() + ". Remove it or choose another destination.");
        need(fs::is_directory(dest.parent_path()), "The destination's parent folder does not exist. Create it or choose another folder.");
        auto disc = openDisc(source);
        const auto count = nod_disc_partitions(disc.get(), nullptr, 0);
        need(count > 0 && count <= 256, "The disc has no readable DATA partition. Make a new copy of your disc.");
        std::vector<NodPartitionInfo> partitions(count);
        nod_disc_partitions(disc.get(), partitions.data(), partitions.size());
        auto data = std::find_if(partitions.begin(), partitions.end(), [](const auto& part) { return part.kind == NOD_PARTITION_KIND_DATA; });
        need(data != partitions.end(), "The disc has no DATA partition. Choose an entire disc image.");
        need(data->data_size < UINT64_MAX - kReserve && freeSpace(dest.parent_path()) >= data->data_size + kReserve,
             "There is not enough free disk space. Free at least " + std::to_string((data->data_size + kReserve + kReserve - 1) / kReserve) + " GB on the destination disk and try again.");
        NodHandle* raw = nullptr;
        NodPartitionOptions options{};
        options.validate_hashes = true;
        auto result = nod_disc_open_partition(disc.get(), data->index, &options, &raw);
        Handle partition(raw, nod_free);
        need(result == NOD_RESULT_OK && partition, "Could not read the DATA partition. Make a new copy of your disc and try again.");
        NodPartitionMeta meta{};
        need(nod_partition_meta(partition.get(), &meta) == NOD_RESULT_OK, "The DATA partition is incomplete or damaged. Make a new copy of your disc and try again.");
        struct SystemFile { const char* name; NodBlob bytes; };
        const SystemFile system[] = {{"boot.bin", meta.raw_boot}, {"bi2.bin", meta.raw_bi2}, {"apploader.img", meta.raw_apploader},
                                     {"fst.bin", meta.raw_fst}, {"main.dol", meta.raw_dol}};
        need(meta.raw_fst.data && meta.raw_fst.size <= kMaxMetadata, "The disc's file list is damaged. Make a new copy of your disc.");
        std::vector<std::uint8_t> image(meta.raw_fst.data, meta.raw_fst.data + meta.raw_fst.size);
        Fst fst;
        std::string detail;
        const bool parsed = Fst::fromDiscImage(image, partial.path / "files", fst, nullptr, false);
        need(parsed, "The disc has an invalid file list or unsafe file name. Make a new copy of your disc.");
        Reporter report(progress);
        for (const auto& file : system) {
            need(file.bytes.data && file.bytes.size && file.bytes.size <= kMaxMetadata, "The disc's system files are incomplete. Make a new copy of your disc.");
            report.state.totalBytes += file.bytes.size;
        }
        for (std::uint32_t i = 1; i < fst.entryCount(); ++i) {
            if (!fst.entry(i).isDir) report.state.totalBytes += fst.entry(i).nextOrLength;
        }
        need(fs::create_directory(partial.path), "Could not create the temporary import folder. Choose another destination.");
        partial.owned = true;
        fs::permissions(partial.path, fs::perms::owner_all, fs::perm_options::replace);
        fs::create_directory(partial.path / "files");
        fs::create_directory(partial.path / "sys");
        report.update(true);
        for (const auto& file : system) {
            report.state.currentFile = std::string("sys/") + file.name;
            Output out(partial.path / report.state.currentFile);
            out.write(file.bytes.data, file.bytes.size);
            out.close();
            report.state.doneBytes += file.bytes.size;
            report.update();
        }
        std::vector<std::uint8_t> buffer(1024 * 1024);
        for (std::uint32_t i = 1; i < fst.entryCount(); ++i) {
            const auto& entry = fst.entry(i);
            const auto& host = fst.hostPath(i);
            if (entry.isDir) {
                need(fs::create_directory(host), "The disc contains conflicting folder names. Make a new copy of your disc.");
                continue;
            }
            report.state.currentFile = host.lexically_relative(partial.path).string();
            NodHandle* fileRaw = nullptr;
            result = nod_partition_open_file(partition.get(), i, &fileRaw);
            Handle file(fileRaw, nod_free);
            need(result == NOD_RESULT_OK && file, "Could not open " + report.state.currentFile + " on the disc. Make a new copy of your disc.");
            Output out(host);
            std::uint64_t remaining = entry.nextOrLength;
            while (remaining) {
                const auto wanted = static_cast<std::size_t>(std::min<std::uint64_t>(buffer.size(), remaining));
                auto got = nod_read(file.get(), buffer.data(), wanted);
                need(got > 0 && static_cast<std::uint64_t>(got) <= wanted, "The disc contains unreadable or damaged data in " + report.state.currentFile + ". Make a new copy of your disc.");
                out.write(buffer.data(), got);
                remaining -= got;
                report.state.doneBytes += got;
                report.update();
            }
            out.close();
            report.update();
        }
        report.state.currentFile = "Checking extracted files";
        report.update();
        const bool valid = validateExtracted(partial.path, false, &detail);
        need(valid, detail);
        report.update(true);
        // macOS's exclusive rename prevents replacing a destination created during import.
        // An existing empty destination is removed only after successful validation.
        if (exists) {
            need(fs::is_directory(fs::symlink_status(dest)) && fs::is_empty(dest) && fs::remove(dest),
                 "The destination folder changed during import. Choose another empty folder.");
        }
        need(::renamex_np(partial.path.c_str(), dest.c_str(), RENAME_EXCL) == 0,
             "Could not finish moving the disc files into place. Check destination permissions and try again.");
        partial.owned = false;
        return Result::Ok;
    } catch (const Cancelled&) {
        return Result::Cancelled;
    } catch (const fs::filesystem_error&) {
        setError(error, "Could not access the source or destination folder. Check folder permissions and free disk space, then try again.");
        return Result::Failed;
    } catch (const std::exception& e) {
        setError(error, e.what());
        return Result::Failed;
    }
}

Result importDisc(const fs::path& source, const fs::path& destRoot, const ProgressFn& progress, std::string* error) {
    return Detail::importWithSpace(source, destRoot, progress, error, [](const fs::path& path) { return fs::space(path).available; });
}
}
