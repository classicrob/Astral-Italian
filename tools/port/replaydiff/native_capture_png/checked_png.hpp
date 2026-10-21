#pragma once
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>

namespace petari::capture {
constexpr std::size_t maxRgbBytes = 16 * 1024 * 1024;
struct Io {
    virtual ~Io() = default;
    virtual ssize_t write(int, const void*, std::size_t);
    virtual ssize_t readAt(int, void*, std::size_t, off_t);
    virtual int flush(int);
    virtual int captureOwner(int, struct stat*);
    virtual int close(int);
    virtual int publish(const char*, const char*);
};
struct Receipt {
    std::string path;
    std::string sha256;
    std::uint32_t width;
    std::uint32_t height;
    std::size_t bytes;
};
struct Result {
    std::optional<Receipt> receipt;
    std::string error;
    std::string retainedPartialPath;
    std::string cleanupStatus;
};
Result publishRgbPng(const std::string& finalPath, std::uint32_t width,
                     std::uint32_t height, const unsigned char* rgb,
                     std::size_t rgbBytes, Io& io);
}
