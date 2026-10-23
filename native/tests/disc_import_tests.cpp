#include "disc_import.hpp"
#include "disc_import_internal.hpp"
#include "dvd_fst.hpp"
#include <CommonCrypto/CommonDigest.h>
#include <algorithm>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <unistd.h>

namespace fs = std::filesystem;
using namespace PetariNative::DiscImport;
using PetariNative::Platform::DVD::Fst;
using Bytes = std::vector<std::uint8_t>;
namespace {
void check(bool ok, const std::string& what) { if (!ok) throw std::runtime_error(what); }
void be(Bytes& b, std::size_t off, std::uint32_t v) {
    for (int i = 3; i >= 0; --i) { b[off + i] = v & 255; v >>= 8; }
}
void text(Bytes& b, std::size_t off, const std::string& s) { std::copy(s.begin(), s.end(), b.begin() + off); }
void save(const fs::path& p, const Bytes& b) {
    fs::create_directories(p.parent_path());
    std::ofstream out(p, std::ios::binary);
    out.write(reinterpret_cast<const char*>(b.data()), b.size());
    check(bool(out), "write synthetic fixture");
}
struct Temp {
    fs::path root;
    Temp() {
        std::string pattern = (fs::temp_directory_path() / "petari-import-tests-XXXXXX").string();
        check(mkdtemp(pattern.data()) != nullptr, "create temporary test directory");
        root = pattern;
    }
    ~Temp() { std::error_code ec; fs::remove_all(root, ec); }
};
struct Fixture {
    Bytes boot = Bytes(0x440), bi2 = Bytes(0x2000), app = Bytes(0x20), dol = Bytes(0x120), fst;
    Fixture() {
        text(boot, 0, "SB4E01"); text(boot, 0x20, "Synthetic import test"); be(boot, 0x18, 0x5d1c9ea3);
        boot[0x60] = boot[0x61] = 1; // unencrypted, hashless test disc
        be(boot, 0x420, 0x3000 / 4); be(boot, 0x424, 0x4000 / 4);
        be(dol, 0, 0x100); be(dol, 0x90, 0x20);
        fst.resize(60);
        auto name = [&](const std::string& s) {
            auto offset = fst.size() - 60;
            fst.insert(fst.end(), s.begin(), s.end()); fst.push_back(0);
            return static_cast<std::uint32_t>(offset);
        };
        be(fst, 0, 0x01000000); be(fst, 8, 5);
        auto a = name("SystemData"); be(fst, 12, 0x01000000 | a); be(fst, 20, 3);
        a = name("ObjNameTable.arc"); be(fst, 24, a); be(fst, 28, 0x8000 / 4); be(fst, 32, 3);
        a = name("ObjectData"); be(fst, 36, 0x01000000 | a); be(fst, 44, 5);
        a = name("Mario.arc"); be(fst, 48, a); be(fst, 52, 0x9000 / 4); be(fst, 56, 5);
        while (fst.size() % 4) fst.push_back(0);
        be(boot, 0x428, fst.size() / 4);
    }
    Bytes image() const {
        Bytes iso(0x80000);
        std::copy(boot.begin(), boot.end(), iso.begin());
        be(iso, 0x40000, 1); be(iso, 0x40004, 0x40020 / 4); be(iso, 0x40020, 0x50000 / 4);
        text(iso, 0x50140, "Root-CA00000001-XS00000003");
        be(iso, 0x502b8, 0x8000 / 4); be(iso, 0x502bc, 0x20000 / 4);
        auto copy = [&](const Bytes& b, std::size_t off) { std::copy(b.begin(), b.end(), iso.begin() + 0x58000 + off); };
        copy(boot, 0); copy(bi2, 0x440); copy(app, 0x2440); copy(dol, 0x3000); copy(fst, 0x4000);
        text(iso, 0x60000, "abc"); text(iso, 0x61000, "12345");
        return iso;
    }
    void extracted(const fs::path& p) const {
        save(p / "sys/boot.bin", boot); save(p / "sys/bi2.bin", bi2); save(p / "sys/apploader.img", app);
        save(p / "sys/main.dol", dol); save(p / "sys/fst.bin", fst);
        save(p / "files/SystemData/ObjNameTable.arc", Bytes{'a','b','c'});
        save(p / "files/ObjectData/Mario.arc", Bytes{'1','2','3','4','5'});
    }
    std::string hash() const {
        unsigned char digest[CC_SHA1_DIGEST_LENGTH];
        CC_SHA1(dol.data(), dol.size(), digest);
        std::string out;
        for (auto byte : digest) { out += "0123456789abcdef"[byte >> 4]; out += "0123456789abcdef"[byte & 15]; }
        return out;
    }
};
}
int main() {
    try {
        Temp temp;
        Fixture fixture;
        const auto iso = temp.root / "test.iso", tree = temp.root / "tree", dest = temp.root / "dest";
        save(iso, fixture.image()); fixture.extracted(tree);
        auto p = probe(iso);
        check(p.isSupportedGame && p.format == Format::GcmIso && p.gameId == "SB4E01" && p.revision == 0, "synthetic Wii probe: " + p.problem);
        check(probe(tree).format == Format::ExtractedFolder && probe(tree).isSupportedGame, "extracted folder probe");
        check(!probe(temp.root / "missing").problem.empty(), "missing probe");
        check(!probe(temp.root).problem.empty(), "unrelated directory probe");
        save(temp.root / "garbage", Bytes{'h','e','l','l','o'});
        check(!probe(temp.root / "garbage").isSupportedGame, "garbage probe");
        auto image = fixture.image(); image.resize(0x100); save(temp.root / "short.iso", image);
        check(probe(temp.root / "short.iso").problem.find("incomplete") != std::string::npos, "truncated probe");
        image = fixture.image(); text(image, 0, "SB4J01"); text(image, 0x20, "Japanese synthetic game"); save(temp.root / "other.iso", image);
        p = probe(temp.root / "other.iso");
        check(!p.isSupportedGame && p.problem.find("SB4J01") != std::string::npos && p.problem.find("Japanese synthetic game") != std::string::npos, "wrong game/region probe");
        image = fixture.image(); image[7] = 1; save(temp.root / "rev.iso", image);
        check(probe(temp.root / "rev.iso").problem.find("revision 1") != std::string::npos, "wrong revision probe");
        Bytes gc(0x10000); text(gc, 0, "GXYZ01"); text(gc, 0x20, "Synthetic GameCube"); be(gc, 0x1c, 0xc2339f3d); save(temp.root / "gc.gcm", gc);
        check(probe(temp.root / "gc.gcm").problem.find("GameCube") != std::string::npos, "GameCube probe");

        // Container fixtures wrap only the tiny synthetic Wii image.
        image = fixture.image();
        Bytes ciso(0x8000 + image.size()); text(ciso, 0, "CISO"); ciso[5] = 0x80;
        std::fill(ciso.begin() + 8, ciso.begin() + 8 + image.size() / 0x8000, 1);
        std::copy(image.begin(), image.end(), ciso.begin() + 0x8000); save(temp.root / "test.ciso", ciso);
        check(probe(temp.root / "test.ciso").isSupportedGame, "CISO probe: " + probe(temp.root / "test.ciso").problem);
        Bytes wbfs(0x200000); text(wbfs, 0, "WBFS"); be(wbfs, 4, wbfs.size() / 512); wbfs[8] = 9; wbfs[9] = 20; wbfs[12] = 1;
        std::copy_n(image.begin(), 0x100, wbfs.begin() + 512); wbfs[512 + 0x100 + 1] = 1;
        std::copy(image.begin(), image.end(), wbfs.begin() + 0x100000); save(temp.root / "test.wbfs", wbfs);
        p = probe(temp.root / "test.wbfs"); check(p.isSupportedGame && p.format == Format::Wbfs, "WBFS probe: " + p.problem);
        for (const char* magic : {"RVZ", "WIA"}) {
            Bytes compressed(64); text(compressed, 0, magic); save(temp.root / magic, compressed);
            check(!probe(temp.root / magic).isSupportedGame && !probe(temp.root / magic).problem.empty(), "truncated compressed image");
        }

        std::string err;
        auto hash = fixture.hash();
        check(Detail::validateTree(tree, true, &err, hash.c_str()), "synthetic quick validate: " + err);
        check(Detail::validateTree(tree, false, &err, hash.c_str()), "synthetic full validate: " + err);
        check(!validateExtracted(tree, false, &err) && err.find("main.dol") != std::string::npos, "production rejects synthetic DOL digest");
        save(tree / "files/ObjectData/Mario.arc", Bytes{1});
        check(!Detail::validateTree(tree, false, &err, hash.c_str()), "wrong FST size");
        check(!Detail::validateTree(tree, true, &err, hash.c_str()), "quick key size");
        fixture.extracted(tree); fs::remove(tree / "files/ObjectData/Mario.arc");
        check(!Detail::validateTree(tree, false, &err, hash.c_str()), "missing FST file");
        fixture.extracted(tree); fs::remove(tree / "files/ObjectData/Mario.arc");
        fs::create_symlink(tree / "files/SystemData/ObjNameTable.arc", tree / "files/ObjectData/Mario.arc");
        check(!Detail::validateTree(tree, false, &err, hash.c_str()), "symlink file rejected");
        fs::remove(tree / "files/ObjectData/Mario.arc"); fixture.extracted(tree);
        fs::remove_all(tree / "files/ObjectData"); fs::create_directory_symlink(tree / "files/SystemData", tree / "files/ObjectData");
        check(!Detail::validateTree(tree, false, &err, hash.c_str()), "symlink directory rejected");
        fs::remove(tree / "files/ObjectData"); fixture.extracted(tree);
        auto bad = fixture.fst;
        Fst parsed;
        be(bad, 4, 1);
        check(!Fst::fromDiscImage(bad, tree / "files", parsed, &err, false), "invalid FST root rejected");
        bad = fixture.fst;
        std::fill(bad.begin() + 60, bad.end(), 'x');
        check(!Fst::fromDiscImage(bad, tree / "files", parsed, &err, false), "unterminated FST name rejected");
        bad = fixture.fst;
        be(bad, 48, static_cast<std::uint32_t>(bad.size() - 60));
        const std::string rawName = "\x82\xa0.arc";
        bad.insert(bad.end(), rawName.begin(), rawName.end()); bad.push_back(0);
        check(Fst::fromDiscImage(bad, tree / "files", parsed, &err, false) && parsed.hostPath(4).filename().native() == rawName,
              "raw FST name bytes preserved");
        for (const std::string name : {"..", ".", "/absolute", "a/b", ""}) {
            bad = fixture.fst;
            const std::uint32_t offset = 0x00ffffff & PetariNative::Platform::DVD::readU32BE(bad.data() + 24);
            std::fill(bad.begin() + 60 + offset, bad.begin() + 60 + offset + 17, 0);
            text(bad, 60 + offset, name);
            Fst fst;
            check(!Fst::fromDiscImage(bad, temp.root / "files", fst, &err, false), "unsafe FST component rejected: " + name);
            auto malicious = fixture;
            malicious.fst = bad;
            save(temp.root / "unsafe.iso", malicious.image());
            check(importDisc(temp.root / "unsafe.iso", dest, {}, &err) == Result::Failed, "unsafe import rejected");
            check(!fs::exists(dest) && !fs::exists(dest.string() + ".partial"), "unsafe import leaves nothing");
        }
        const auto enough = [](const fs::path&) { return UINT64_MAX; };
        bool queried = false;
        check(Detail::importWithSpace(iso, dest, {}, &err, [&](const fs::path&) { queried = true; return 0; }) == Result::Failed && queried &&
              err.find("space") != std::string::npos && !fs::exists(dest.string() + ".partial"), "free space fails before extraction");
        for (const auto& source : {iso, temp.root / "test.ciso", temp.root / "test.wbfs"}) {
            int calls = 0;
            auto result = Detail::importWithSpace(source, dest, [&](const Progress& p) {
                ++calls; check(p.totalBytes > 0 && p.doneBytes <= p.totalBytes, "progress totals"); return false;
            }, &err, enough);
            check(result == Result::Cancelled && calls == 1, "cancel callback for " + source.extension().string() + ": " + err);
            check(!fs::exists(dest) && !fs::exists(dest.string() + ".partial"), "cancel cleanup");
        }
        int writtenCalls = 0;
        check(Detail::importWithSpace(iso, dest, [&](const Progress& p) {
            ++writtenCalls;
            if (writtenCalls == 1) {
                std::this_thread::sleep_for(std::chrono::milliseconds(110));
                return true;
            }
            check(p.doneBytes > 0 && fs::exists(fs::path(dest.string() + ".partial") / "sys/boot.bin"), "cancel after writing begins");
            return false;
        }, &err, enough) == Result::Cancelled && writtenCalls == 2, "cancel during extraction");
        check(!fs::exists(dest) && !fs::exists(dest.string() + ".partial"), "mid-write cancellation cleanup");
        int calls = 0;
        check(Detail::importWithSpace(iso, dest, [&](const Progress&) { ++calls; return true; }, &err, enough) == Result::Failed && calls > 0,
              "synthetic extraction must fail production DOL validation");
        check(err.find("main.dol") != std::string::npos && !fs::exists(dest.string() + ".partial"), "validation failure cleanup: " + err);
        fs::create_directory(dest.string() + ".partial"); save(fs::path(dest.string() + ".partial") / "keep", Bytes{1});
        check(importDisc(iso, dest, {}, &err) == Result::Failed && fs::exists(fs::path(dest.string() + ".partial") / "keep"), "foreign partial preserved");
        fs::remove_all(dest.string() + ".partial");
        fs::create_directory(dest); save(dest / "keep", Bytes{1});
        check(importDisc(iso, dest, {}, &err) == Result::Failed && fs::exists(dest / "keep"), "nonempty destination preserved");
        std::cout << "PASS: synthetic Wii/GCM/CISO/WBFS probes; invalid/truncated/RVZ/WIA probes; header/region/revision; quick/full validation; SHA-1; FST sizes/names/symlinks; space injection; progress/cancel/failure cleanup; destination ownership\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "FAIL: " << e.what() << '\n'; return 1;
    }
}
