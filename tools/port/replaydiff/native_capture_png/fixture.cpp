#include "checked_png.hpp"
#include <algorithm>
#include <cerrno>
#include <filesystem>
#include <fstream>
#include <fcntl.h>
#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include <vector>
using namespace petari::capture;
namespace fs = std::filesystem;
struct Faults : Io {
    std::string mode;
    int writes = 0, reads = 0, flushes = 0, closes = 0;
    explicit Faults(std::string m) : mode(std::move(m)) {}
    ssize_t write(int fd, const void* p, std::size_t n) override {
        ++writes;
        if (mode == "write-eintr" && writes <= 3) { errno = EINTR; return -1; }
        if (mode == "enospc" && writes == 2) { errno = ENOSPC; return -1; }
        if (mode == "write-eio") { errno = EIO; return -1; }
        if (mode == "zero-write") return 0;
        if (mode == "short-write" || mode == "enospc") n = std::min<std::size_t>(13, n);
        return Io::write(fd, p, n);
    }
    ssize_t readAt(int fd, void* p, std::size_t n, off_t at) override {
        ++reads;
        if (mode == "read-eintr" && reads <= 3) { errno = EINTR; return -1; }
        if (mode == "read-eio") { errno = EIO; return -1; }
        if (mode == "read-eof") return 0;
        if (mode == "short-read") n = std::min<std::size_t>(17, n);
        auto result = Io::readAt(fd, p, n, at);
        if (mode == "corrupt-read" && result > 0) static_cast<unsigned char*>(p)[0] ^= 1;
        return result;
    }
    int flush(int fd) override {
        ++flushes;
        if (mode == "flush-eintr" && flushes <= 3) { errno = EINTR; return -1; }
        if (mode == "flush-eio") { errno = EIO; return -1; }
        if (mode == "truncate" && ::ftruncate(fd, 8)) return -1;
        return Io::flush(fd);
    }
    int close(int fd) override {
        ++closes;
        int rc = Io::close(fd);
        if ((mode == "close-partial" && closes == 1) || (mode == "close-final" && closes == 2)) {
            errno = EIO; return -1;
        }
        if (mode == "close-eintr" && closes == 1) { errno = EINTR; return -1; }
        return rc;
    }
    int publish(const char* temp, const char* final) override {
        if (mode == "publish-eio") { errno = EIO; return -1; }
        if (mode == "racing-collision") { std::ofstream other(final); other << "sentinel"; }
        return Io::publish(temp, final);
    }
};
void require(bool value, const std::string& why) { if (!value) throw std::runtime_error(why); }
int openDescriptors() {
    int count = 0;
    for (int fd = 0; fd < 256; ++fd) if (::fcntl(fd, F_GETFD) >= 0) ++count;
    return count;
}
void checkNoPartial(const fs::path& root) {
    for (const auto& p : fs::directory_iterator(root)) require(p.path().string().find(".partial.") == std::string::npos, "partial leaked");
}
int main(int argc, char** argv) {
    try {
        require(argc == 2, "fixture requires private output directory");
        fs::path root = argv[1];
        require(fs::create_directory(root), "fresh fixture output required");
        std::vector<unsigned char> rgb(640 * 48 * 3);
        for (std::size_t i = 0; i < rgb.size(); ++i) rgb[i] = (i * 37 + i / 7) & 255;
        std::ofstream pixels(root / "expected.rgb", std::ios::binary); pixels.write(reinterpret_cast<const char*>(rgb.data()), rgb.size()); pixels.close();
        require(bool(pixels), "fixture RGB write");
        int checks = 0;
        const auto descriptors = openDescriptors();
        for (auto mode : {"success", "short-write", "write-eintr", "read-eintr", "short-read", "flush-eintr"}) {
            Faults io(mode); auto path = root / (std::string(mode) + ".png");
            auto r = publishRgbPng(path.string(), 640, 48, rgb.data(), rgb.size(), io);
            require(r.receipt.has_value(), std::string(mode) + ": " + r.error);
            require(r.receipt->width == 640 && r.receipt->height == 48 && r.receipt->sha256.size() == 64, "receipt dimensions/hash");
            std::cout << "RECEIPT " << path.filename().string() << ' ' << r.receipt->sha256 << ' ' << r.receipt->bytes << '\n';
            checkNoPartial(root); ++checks;
        }
        for (auto mode : {"enospc", "write-eio", "zero-write", "read-eio", "read-eof", "corrupt-read", "flush-eio", "truncate", "close-partial", "close-final", "close-eintr", "publish-eio", "racing-collision"}) {
            Faults io(mode); auto path = root / (std::string(mode) + ".png");
            auto r = publishRgbPng(path.string(), 640, 48, rgb.data(), rgb.size(), io);
            require(!r.receipt && !r.error.empty(), std::string(mode) + " unexpectedly accepted");
            if (io.mode == "racing-collision") {
                std::ifstream existing(path); std::string data; existing >> data;
                require(data == "sentinel", "collision modified existing destination");
                fs::remove(path);
            } else require(!fs::exists(path), std::string(mode) + " failed final leaked");
            checkNoPartial(root); ++checks;
        }
        {
            auto path = root / "collision.png";
            { std::ofstream f(path); f << "sentinel"; }
            Faults io("success"); auto r = publishRgbPng(path.string(), 640, 48, rgb.data(), rgb.size(), io);
            require(!r.receipt, "existing destination overwritten");
            std::ifstream f(path); std::string data; f >> data; require(data == "sentinel", "existing destination changed");
            fs::remove(path); checkNoPartial(root); ++checks;
        }
        struct Invalid { std::uint32_t w, h; std::size_t n; bool null; };
        for (auto v : {Invalid{0, 1, 0, false}, Invalid{1, 0, 0, false}, Invalid{0xffffffffu, 0xffffffffu, 0, false}, Invalid{0x7fffffffu, 0x7fffffffu, 0, false}, Invalid{640, 48, rgb.size() - 1, false}, Invalid{640, 48, rgb.size() + 1, false}, Invalid{640, 48, rgb.size(), true}, Invalid{4096, 4096, 50331648, false}}) {
            Faults io("success"); auto path = root / "invalid.png";
            auto r = publishRgbPng(path.string(), v.w, v.h, v.null ? nullptr : rgb.data(), v.n, io);
            require(!r.receipt && !fs::exists(path), "invalid input accepted"); require(io.writes == 0, "invalid input reached I/O");
            checkNoPartial(root); ++checks;
        }
        {
            Faults io("success"); unsigned char pixel[]{1, 2, 3};
            auto r = publishRgbPng((root / "one.png").string(), 1, 1, pixel, 3, io);
            require(r.receipt.has_value(), "one-pixel PNG failed");
            std::cout << "RECEIPT one.png " << r.receipt->sha256 << ' ' << r.receipt->bytes << '\n'; ++checks;
        }
        checkNoPartial(root);
        require(openDescriptors() == descriptors, "descriptor leaked");
        std::cout << "PASS " << checks << " native controls\n";
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
