#include "checked_png.hpp"
#include <CommonCrypto/CommonDigest.h>
#include <algorithm>
#include <array>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <limits>
#include <stdexcept>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

namespace petari::capture {
ssize_t Io::write(int fd, const void* p, std::size_t n) { return ::write(fd, p, n); }
ssize_t Io::readAt(int fd, void* p, std::size_t n, off_t at) { return ::pread(fd, p, n, at); }
int Io::flush(int fd) { return ::fsync(fd); }
int Io::close(int fd) { return ::close(fd); }
int Io::publish(const char* temp, const char* final) { return ::link(temp, final); }
namespace {
using Bytes = std::vector<unsigned char>;
void fail(const char* operation) { throw std::runtime_error(std::string(operation) + ": " + std::strerror(errno)); }
void be(Bytes& out, std::uint32_t n) {
    for (int shift = 24; shift >= 0; shift -= 8) out.push_back(static_cast<unsigned char>(n >> shift));
}
std::uint32_t crc(const unsigned char* p, std::size_t n) {
    std::uint32_t c = ~0u;
    for (std::size_t i = 0; i < n; ++i) {
        c ^= p[i];
        for (int k = 0; k < 8; ++k) c = (c >> 1) ^ ((c & 1) ? 0xedb88320u : 0u);
    }
    return ~c;
}
void chunk(Bytes& out, const char* type, const Bytes& body) {
    if (body.size() > 0x7fffffffu) throw std::runtime_error("PNG chunk length exceeds specification");
    be(out, static_cast<std::uint32_t>(body.size()));
    auto start = out.size();
    out.insert(out.end(), type, type + 4);
    out.insert(out.end(), body.begin(), body.end());
    be(out, crc(out.data() + start, out.size() - start));
}
Bytes encode(std::uint32_t w, std::uint32_t h, const unsigned char* rgb, std::size_t size) {
    if (!w || !h || w > 0x7fffffffu || h > 0x7fffffffu || !rgb)
        throw std::runtime_error("invalid dimensions or RGB pointer");
    if (std::size_t(w) > std::numeric_limits<std::size_t>::max() / 3 / h)
        throw std::runtime_error("RGB dimension overflow");
    std::size_t expected = std::size_t(w) * h * 3;
    if (expected > maxRgbBytes || size != expected) throw std::runtime_error("RGB byte count/capture limit mismatch");
    if (expected > std::numeric_limits<std::size_t>::max() - h) throw std::runtime_error("scanline overflow");
    Bytes raw;
    raw.reserve(expected + h);
    for (std::uint32_t y = 0; y < h; ++y) {
        raw.push_back(0);
        raw.insert(raw.end(), rgb + std::size_t(y) * w * 3, rgb + std::size_t(y + 1) * w * 3);
    }
    Bytes z{0x78, 0x01};
    std::uint32_t a = 1, b = 0;
    for (auto c : raw) { a = (a + c) % 65521; b = (b + a) % 65521; }
    for (std::size_t at = 0; at < raw.size();) {
        auto n = std::min<std::size_t>(65535, raw.size() - at);
        z.push_back(at + n == raw.size() ? 1 : 0);
        z.push_back(n & 255); z.push_back(n >> 8);
        z.push_back((~n) & 255); z.push_back(((~n) >> 8) & 255);
        z.insert(z.end(), raw.begin() + at, raw.begin() + at + n);
        at += n;
    }
    be(z, (b << 16) | a);
    Bytes out{137, 80, 78, 71, 13, 10, 26, 10}, header;
    be(header, w); be(header, h); header.insert(header.end(), {8, 2, 0, 0, 0});
    chunk(out, "IHDR", header); chunk(out, "IDAT", z); chunk(out, "IEND", {});
    return out;
}
void verify(int fd, const Bytes& expected, Io& io, CC_SHA256_CTX* hash = nullptr) {
    struct stat st{};
    if (::fstat(fd, &st)) fail("fstat");
    if (!S_ISREG(st.st_mode) || st.st_size < 0 || static_cast<std::uint64_t>(st.st_size) != expected.size())
        throw std::runtime_error("closed PNG size/type mismatch");
    std::array<unsigned char, 16384> buf{};
    std::size_t at = 0;
    while (at < expected.size()) {
        auto wanted = std::min(buf.size(), expected.size() - at);
        auto n = io.readAt(fd, buf.data(), wanted, static_cast<off_t>(at));
        if (n < 0 && errno == EINTR) continue;
        if (n < 0) fail("pread");
        if (!n || static_cast<std::size_t>(n) > wanted) throw std::runtime_error("PNG readback truncated/invalid");
        if (std::memcmp(buf.data(), expected.data() + at, n)) throw std::runtime_error("PNG readback content mismatch");
        if (hash && !CC_SHA256_Update(hash, buf.data(), static_cast<CC_LONG>(n))) throw std::runtime_error("SHA256 update failed");
        at += n;
    }
}
bool owns(const std::string& path, const struct stat& owner) {
    struct stat st{};
    return !::lstat(path.c_str(), &st) && st.st_dev == owner.st_dev && st.st_ino == owner.st_ino;
}
}
Result publishRgbPng(const std::string& path, std::uint32_t w, std::uint32_t h,
                     const unsigned char* rgb, std::size_t size, Io& io) {
    int fd = -1;
    std::string temp;
    struct stat owner{};
    bool published = false;
    try {
        if (path.empty() || path.back() == '/' || path.find('\0') != std::string::npos)
            throw std::runtime_error("invalid final path");
        auto bytes = encode(w, h, rgb, size);
        temp = path + ".partial.XXXXXX";
        std::vector<char> name(temp.begin(), temp.end()); name.push_back(0);
        fd = ::mkstemp(name.data());
        if (fd < 0) fail("mkstemp");
        temp = name.data();
        if (::fstat(fd, &owner)) fail("fstat owner");
        for (std::size_t at = 0; at < bytes.size();) {
            auto n = io.write(fd, bytes.data() + at, bytes.size() - at);
            if (n < 0 && errno == EINTR) continue;
            if (n < 0) fail("write");
            if (!n || static_cast<std::size_t>(n) > bytes.size() - at) throw std::runtime_error("write made no/invalid progress");
            at += n;
        }
        int rc;
        do { rc = io.flush(fd); } while (rc < 0 && errno == EINTR);
        if (rc < 0) fail("fsync");
        verify(fd, bytes, io);
        int closing = fd; fd = -1;
        if (io.close(closing)) fail("close partial");
        if (!owns(temp, owner)) throw std::runtime_error("partial identity changed");
        if (io.publish(temp.c_str(), path.c_str())) fail("atomic no-replace publication");
        published = true;
        fd = ::open(path.c_str(), O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
        if (fd < 0) fail("open published PNG");
        struct stat publishedStat{};
        if (::fstat(fd, &publishedStat)) fail("fstat published PNG");
        if (publishedStat.st_dev != owner.st_dev || publishedStat.st_ino != owner.st_ino) throw std::runtime_error("published identity changed");
        CC_SHA256_CTX hash;
        if (!CC_SHA256_Init(&hash)) throw std::runtime_error("SHA256 init failed");
        verify(fd, bytes, io, &hash);
        closing = fd; fd = -1;
        if (io.close(closing)) fail("close published PNG");
        unsigned char digest[CC_SHA256_DIGEST_LENGTH];
        if (!CC_SHA256_Final(digest, &hash)) throw std::runtime_error("SHA256 final failed");
        std::string hex;
        for (auto c : digest) { hex.push_back("0123456789abcdef"[c >> 4]); hex.push_back("0123456789abcdef"[c & 15]); }
        if (::unlink(temp.c_str())) fail("remove partial link");
        return {Receipt{path, hex, w, h, bytes.size()}, {}};
    } catch (const std::exception& e) {
        std::string error = e.what();
        if (fd >= 0 && io.close(fd)) error += std::string("; cleanup close: ") + std::strerror(errno);
        if (!temp.empty() && owns(temp, owner) && ::unlink(temp.c_str()))
            error += std::string("; cleanup partial: ") + std::strerror(errno);
        if (published && owns(path, owner) && ::unlink(path.c_str()))
            error += std::string("; cleanup final: ") + std::strerror(errno);
        return {std::nullopt, error};
    }
}
}
