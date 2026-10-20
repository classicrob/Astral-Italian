#pragma once

#include <petari/save_codec.hpp>
#include <cerrno>
#include <filesystem>
#include <fstream>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace PetariNative::SaveWire {
inline Bytes readRegular(const std::filesystem::path& path) {
    const int fd = ::open(path.c_str(), O_RDONLY | O_NOFOLLOW);
    require(fd >= 0, "cannot open save or backup");
    struct stat stat{};
    if (::fstat(fd, &stat) || !S_ISREG(stat.st_mode) || stat.st_size < 16 || stat.st_size > 0x8000) {
        ::close(fd);
        throw std::runtime_error("invalid save or backup file");
    }
    Bytes data(static_cast<std::size_t>(stat.st_size));
    std::size_t done = 0;
    while (done < data.size()) {
        const auto count = ::read(fd, data.data() + done, data.size() - done);
        if (count < 0 && errno == EINTR) continue;
        if (count <= 0) {::close(fd); throw std::runtime_error("cannot read complete save or backup");}
        done += static_cast<std::size_t>(count);
    }
    const int result = ::close(fd);
    require(result == 0, "cannot close save or backup");
    return data;
}
inline void writeSynced(int fd, const Bytes& bytes) {
    std::size_t done = 0;
    while (done < bytes.size()) {
        const auto count = ::write(fd, bytes.data() + done, bytes.size() - done);
        if (count < 0 && errno == EINTR) continue;
        if (count <= 0) {::close(fd); throw std::runtime_error("cannot write migration file");}
        done += static_cast<std::size_t>(count);
    }
    const int result = ::fsync(fd);
    const int closed = ::close(fd);
    require(result == 0 && closed == 0, "cannot sync migration file");
}
inline void syncDirectory(const std::filesystem::path& directory) {
    const int fd = ::open(directory.c_str(), O_RDONLY | O_DIRECTORY);
    require(fd >= 0, "cannot open save directory");
    const int result = ::fsync(fd);
    const int closed = ::close(fd);
    require(result == 0 && closed == 0, "cannot sync save directory");
}
// The original is not replaced until both backup contents and directory entry are durable.
inline void migrateFile(const std::filesystem::path& path, const Bytes& original, const Bytes& converted) {
    require(canonical(original, true) == converted && canonical(converted, false) == converted,
            "invalid migration input");
    require(readRegular(path) == original, "save changed before backup");
    auto backup = path;
    backup += ".le-backup";
    const int fd = ::open(backup.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW, 0600);
    if (fd >= 0) writeSynced(fd, original);
    else require(errno == EEXIST, "cannot create legacy backup; original untouched");
    require(readRegular(backup) == original, "legacy backup differs; original untouched");
    const int backupFd = ::open(backup.c_str(), O_RDONLY | O_NOFOLLOW);
    require(backupFd >= 0, "cannot reopen verified backup; original untouched");
    const int synced = ::fsync(backupFd);
    const int closed = ::close(backupFd);
    require(synced == 0 && closed == 0, "cannot sync verified backup; original untouched");
    syncDirectory(path.parent_path());
    require(readRegular(path) == original, "save changed after backup; original untouched");
    struct stat status{};
    require(::lstat(path.c_str(), &status) == 0 && S_ISREG(status.st_mode), "save no longer regular");
    std::string temporary = path.string() + ".wii-XXXXXX";
    std::vector<char> name(temporary.begin(), temporary.end());
    name.push_back(0);
    const int output = ::mkstemp(name.data());
    require(output >= 0, "cannot create migrated save; original untouched");
    try {
        if (::fchmod(output, status.st_mode & 0777)) {
            ::close(output);
            throw std::runtime_error("cannot preserve save permissions");
        }
        writeSynced(output, converted);
        require(readRegular(name.data()) == converted, "migrated file readback differs");
        require(readRegular(path) == original, "save changed during migration");
        require(::rename(name.data(), path.c_str()) == 0, "cannot replace save; original untouched");
        syncDirectory(path.parent_path());
        require(readRegular(path) == converted, "migrated save readback differs; backup retained");
    } catch (...) {
        ::unlink(name.data());
        throw;
    }
}
} // namespace PetariNative::SaveWire
