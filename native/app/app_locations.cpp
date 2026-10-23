#include "app_locations.hpp"

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#ifdef __APPLE__
#include <copyfile.h>
#endif

namespace PetariNative::App {
namespace fs = std::filesystem;
namespace {
std::string stamp() {
    const auto now = std::time(nullptr);
    std::tm time{};
    localtime_r(&now, &time);
    char text[32];
    std::strftime(text, sizeof(text), "%Y%m%d-%H%M%S", &time);
    return text;
}
std::runtime_error ioError(const std::string& action) {
    return std::runtime_error(action + ": " + std::strerror(errno) + ". Check that the folder is writable and has free space.");
}
void writeExclusive(const fs::path& path, const std::string& text) {
    const int fd = ::open(path.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0600);
    if (fd < 0) throw ioError("Could not create " + path.string());
    std::size_t done = 0;
    while (done < text.size()) {
        const auto n = ::write(fd, text.data() + done, text.size() - done);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) { ::close(fd); throw ioError("Could not write " + path.string()); }
        done += static_cast<std::size_t>(n);
    }
    ::close(fd);
}
void copyAbsent(const fs::path& source, const fs::path& destination) {
    const auto status = fs::symlink_status(source);
    if (fs::is_symlink(status)) throw std::runtime_error("The old save folder contains a link. Copy your saves manually to " + destination.string() + ".");
    if (fs::is_directory(status)) {
        if (!fs::create_directory(destination)) return;
        for (const auto& entry : fs::directory_iterator(source)) copyAbsent(entry.path(), destination / entry.path().filename());
#ifdef __APPLE__
        if (copyfile(source.c_str(), destination.c_str(), nullptr, COPYFILE_METADATA) != 0)
            throw ioError("Could not copy save folder metadata");
#endif
    } else if (fs::is_regular_file(status)) {
#ifdef __APPLE__
        if (copyfile(source.c_str(), destination.c_str(), nullptr, COPYFILE_ALL | COPYFILE_EXCL) != 0 && errno != EEXIST)
            throw ioError("Could not copy " + source.string());
#else
        fs::copy_file(source, destination, fs::copy_options::skip_existing);
#endif
    }
}
bool nullDescriptor(int fd) {
    struct stat current{}, null{};
    if (fstat(fd, &current) != 0) return errno == EBADF;
    return stat("/dev/null", &null) == 0 && S_ISCHR(current.st_mode) && current.st_rdev == null.st_rdev;
}
bool ownedName(const std::string& name, bool tapes) {
    const std::string prefix = tapes ? "session-" : "astral-";
    const std::string suffix = tapes ? "" : ".log";
    if (name.compare(0, prefix.size(), prefix) != 0 || name.size() < prefix.size() + 15 + suffix.size()) return false;
    for (unsigned i = 0; i < 15; ++i) {
        const char c = name[prefix.size() + i];
        if (i == 8 ? c != '-' : c < '0' || c > '9') return false;
    }
    return suffix.empty() || name.compare(name.size() - suffix.size(), suffix.size(), suffix) == 0;
}
}

fs::path AppLocations::defaultRoot(const fs::path& home) { return home / "Library/Application Support/Astral Italian"; }
bool isOtherGameUserPath(const fs::path& path, const fs::path& home) {
    if (home.empty()) return false;
    const auto candidate = fs::weakly_canonical(path);
    const auto otherGame = fs::weakly_canonical(home / "Library/Application Support/Petari");
    auto c = candidate.begin();
    for (auto p = otherGame.begin(); p != otherGame.end(); ++p, ++c) {
        if (c == candidate.end() || *c != *p) return false;
    }
    return true;
}
AppLocations AppLocations::fromEnvironment() {
    const char* home = std::getenv("HOME");
    const char* override = std::getenv("ASTRAL_SUPPORT_DIR");
    if ((!home || !*home) && (!override || !*override)) throw std::runtime_error("Your home folder could not be found. Set HOME before starting Astral Italian.");
    AppLocations locations{override && *override ? fs::path(override) : defaultRoot(home)};
    if (home && *home && isOtherGameUserPath(locations.root, home))
        throw std::runtime_error("Astral Italian cannot use Petari's folder. That folder contains your other game's saves.");
    return locations;
}
AppConfig readAppConfig(const AppLocations& locations) {
    if (!fs::exists(locations.config())) return {};
    std::ifstream input(locations.config());
    if (!input) throw std::runtime_error("Could not read " + locations.config().string() + ". Check its permissions and try again.");
    AppConfig result;
    std::string line;
    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line.front() == '#') continue;
        const auto equal = line.find('=');
        if (equal == std::string::npos || equal == 0) throw std::runtime_error("The app.cfg settings file is damaged. Rename it and open Astral Italian again.");
        result[line.substr(0, equal)] = line.substr(equal + 1);
    }
    if (input.bad()) throw std::runtime_error("Could not finish reading app.cfg. Check the drive and try again.");
    return result;
}
void writeAppConfig(const AppLocations& locations, const AppConfig& config) {
    std::string text;
    for (const auto& [key, value] : config) {
        if (key.empty() || key.find_first_of("=\r\n") != std::string::npos || value.find_first_of("\r\n") != std::string::npos)
            throw std::runtime_error("This path cannot be remembered because it contains a line break. Choose a different folder.");
        text += key + "=" + value + "\n";
    }
    fs::create_directories(locations.root);
    auto pattern = (locations.root / ".app.cfg-XXXXXX").string();
    const int fd = mkstemp(pattern.data());
    if (fd < 0) throw ioError("Could not save app.cfg");
    bool closed = false;
    try {
        std::size_t done = 0;
        while (done < text.size()) {
            const auto n = ::write(fd, text.data() + done, text.size() - done);
            if (n < 0 && errno == EINTR) continue;
            if (n <= 0) throw ioError("Could not save app.cfg");
            done += static_cast<std::size_t>(n);
        }
        if (fsync(fd) != 0) throw ioError("Could not save app.cfg");
        const int closeResult = ::close(fd);
        closed = true;
        if (closeResult != 0 || ::rename(pattern.c_str(), locations.config().c_str()) != 0) throw ioError("Could not save app.cfg");
    } catch (...) {
        if (!closed) ::close(fd);
        ::unlink(pattern.c_str());
        throw;
    }
}
fs::path migrationSource(const AppLocations& locations, const fs::path& home, const AppConfig& config) {
    if (home.empty() || config.count("migration") || fs::exists(locations.root / "NAND")) return {};
    for (const auto& source : {home / "MacioGalaxy2-play/user", home / "Library/Application Support/AstralItalian"}) {
        if (!isOtherGameUserPath(source, home) && fs::is_directory(source / "NAND") && fs::weakly_canonical(source) != fs::weakly_canonical(locations.root)) return source;
    }
    return {};
}
void migrateUserState(const AppLocations& locations, const fs::path& source, bool copy, AppConfig* config) {
    (*config)["migration"] = copy ? "copy" : "declined";
    writeAppConfig(locations, *config);
    if (!copy) return;
    // Stage complete entries before publishing them. Failed copies cannot leave a partial NAND.
    auto pattern = (locations.root / ".migration-XXXXXX").string();
    if (!mkdtemp(pattern.data())) throw ioError("Could not prepare the save copy");
    struct Staging {
        fs::path path;
        ~Staging() { std::error_code ec; fs::remove_all(path, ec); }
    } staging{pattern};
    for (const char* name : {"NAND", "settings.txt", "mods.txt", "controls.txt", "camera.txt", "progress.json", "progress_settings.txt", "mods-saves"}) {
        if (fs::exists(fs::symlink_status(locations.root / name))) continue;
        if (fs::exists(fs::symlink_status(source / name))) copyAbsent(source / name, staging.path / name);
    }
    for (const auto& entry : fs::directory_iterator(staging.path)) {
        const auto destination = locations.root / entry.path().filename();
#ifdef __APPLE__
        // Exclusive rename also protects an entry created while the copy was in progress.
        if (renamex_np(entry.path().c_str(), destination.c_str(), RENAME_EXCL) != 0 && errno != EEXIST)
            throw ioError("Could not put the copied saves in Astral Italian");
#else
        copyAbsent(entry.path(), destination);
#endif
    }
    if (!fs::exists(fs::symlink_status(locations.root / "migrated-from.txt"))) writeExclusive(locations.root / "migrated-from.txt", source.string() + "\n");
}
void pruneRunArtifacts(const fs::path& directory, bool tapes, unsigned keep) {
    if (!fs::exists(directory)) return;
    std::vector<fs::directory_entry> entries;
    for (const auto& entry : fs::directory_iterator(directory)) {
        const auto status = entry.symlink_status();
        if ((tapes ? fs::is_directory(status) : fs::is_regular_file(status)) && ownedName(entry.path().filename().string(), tapes)) entries.push_back(entry);
    }
    std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
        const auto at = a.last_write_time(), bt = b.last_write_time();
        return at != bt ? at > bt : a.path().filename() > b.path().filename();
    });
    for (std::size_t i = keep; i < entries.size(); ++i) {
        if (tapes) fs::remove_all(entries[i].path());
        else fs::remove(entries[i].path());
    }
}
void prepareRunArtifacts(const AppLocations& locations) {
    if (!std::getenv("PETARI_CAPTURE_DIR")) {
        fs::create_directories(locations.captures());
        if (setenv("PETARI_CAPTURE_DIR", locations.captures().c_str(), 0) != 0) throw ioError("Could not choose the capture folder");
    }
    const char* tape = std::getenv("ASTRAL_TAPE");
    if (!std::getenv("PETARI_TAPE_RECORD") && !std::getenv("PETARI_ROUTE_RECORD") && (!tape || std::strcmp(tape, "off") != 0)) {
        fs::create_directories(locations.tapes());
        const auto session = locations.tapes() / ("session-" + stamp() + "-" + std::to_string(getpid()));
        pruneRunArtifacts(locations.tapes(), true, 9);
        if (!fs::create_directory(session)) throw std::runtime_error("This session's tape folder already exists. Try opening Astral Italian again.");
        if (setenv("PETARI_TAPE_RECORD", session.c_str(), 0) != 0) throw ioError("Could not choose the tape folder");
    }
}
fs::path redirectAppLog(const AppLocations& locations) {
    if (!nullDescriptor(STDOUT_FILENO) || !nullDescriptor(STDERR_FILENO)) return {};
    fs::create_directories(locations.logs());
    pruneRunArtifacts(locations.logs(), false, 19);
    auto path = locations.logs() / ("astral-" + stamp() + ".log");
    int fd = ::open(path.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0600);
    if (fd < 0 && errno == EEXIST) {
        path = locations.logs() / ("astral-" + stamp() + "-" + std::to_string(getpid()) + ".log");
        fd = ::open(path.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0600);
    }
    if (fd < 0) throw ioError("Could not open the app log");
    // Keep the source away from fd 1/2 when Finder supplied closed descriptors.
    const int source = fcntl(fd, F_DUPFD_CLOEXEC, 3);
    ::close(fd);
    if (source < 0) throw ioError("Could not open the app log");
    const bool ok = dup2(source, STDOUT_FILENO) >= 0 && dup2(source, STDERR_FILENO) >= 0;
    ::close(source);
    if (!ok) throw ioError("Could not redirect the app log");
    setenv("PETARI_CAPTURE_LOG", path.c_str(), 0);
    return path;
}
}  // namespace PetariNative::App
