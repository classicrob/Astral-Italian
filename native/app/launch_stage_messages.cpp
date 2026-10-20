#include "launch_stage_messages.hpp"
#include "archive.hpp"
#include <petari/endian.hpp>

#include <cstring>
#include <fstream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace PetariNative::App::LaunchStage {
namespace {
void require(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
bool contains(std::size_t size, std::size_t at, std::size_t n) { return at <= size && n <= size - at; }
struct Block { const std::uint8_t* data = nullptr; std::size_t size = 0; };
std::uint32_t u32(Block b, std::size_t at) { require(contains(b.size, at, 4), "MSBT u32 outside block"); return readU32BE(b.data + at); }

void utf8(std::string& out, std::uint32_t c) {
    if (c < 0x80) out += static_cast<char>(c);
    else if (c < 0x800) { out += static_cast<char>(0xc0 | (c >> 6)); out += static_cast<char>(0x80 | (c & 63)); }
    else if (c < 0x10000) { out += static_cast<char>(0xe0 | (c >> 12)); out += static_cast<char>(0x80 | ((c >> 6) & 63)); out += static_cast<char>(0x80 | (c & 63)); }
    else { out += static_cast<char>(0xf0 | (c >> 18)); out += static_cast<char>(0x80 | ((c >> 12) & 63)); out += static_cast<char>(0x80 | ((c >> 6) & 63)); out += static_cast<char>(0x80 | (c & 63)); }
}

std::string text(Block txt, std::uint32_t index) {
    const auto count = u32(txt, 0);
    require(count <= (txt.size - 4) / 4 && index < count, "MSBT text index/table outside TXT2");
    const std::size_t at = u32(txt, 4 + 4 * index);
    const std::size_t end = index + 1 < count ? u32(txt, 8 + 4 * index) : txt.size;
    require(at >= 4 + 4 * count && at % 2 == 0 && end >= at && end <= txt.size, "MSBT text extent outside TXT2");
    std::string result;
    std::size_t pos = at;
    for (unsigned units = 0; units < 1024; ++units) {
        require(contains(end, pos, 2), "unterminated MSBT galaxy name");
        std::uint32_t c = readU16BE(txt.data + pos); pos += 2;
        if (c == 0) { require(!result.empty(), "empty English galaxy name"); return result; }
        require(c >= 0x20, "formatted/control MSBT galaxy name is unsupported");
        if (c >= 0xd800 && c <= 0xdbff) {
            require(contains(end, pos, 2), "truncated UTF-16 surrogate");
            const auto low = readU16BE(txt.data + pos); pos += 2;
            require(low >= 0xdc00 && low <= 0xdfff, "invalid UTF-16 surrogate pair");
            c = 0x10000 + ((c - 0xd800) << 10) + low - 0xdc00;
        } else require(c < 0xdc00 || c > 0xdfff, "unpaired UTF-16 low surrogate");
        utf8(result, c);
    }
    throw std::runtime_error("English galaxy name exceeds length limit");
}

void merge(EnglishNames& names, const std::string& stage, EnglishName name) {
    auto old = names.find(stage);
    require(old == names.end() || old->second.text == name.text, "conflicting English GalaxyName labels");
    if (old == names.end()) names.emplace(stage, std::move(name));
}

EnglishNames decode(Resource::Bytes bytes, const std::string& origin) {
    require(bytes.data && bytes.size >= 0x20 && bytes.size <= 16 * 1024 * 1024 && std::memcmp(bytes.data, "MsgStdBn", 8) == 0, "invalid/oversized MSBT header");
    require(readU16BE(bytes.data + 8) == 0xfeff && bytes.data[0xc] == 1, "expected big-endian UTF-16 MSBT");
    const auto size = readU32BE(bytes.data + 0x12);
    const auto count = readU16BE(bytes.data + 0xe);
    require(size >= 0x20 && size <= bytes.size && count > 0 && count <= 256, "invalid MSBT file size/block count");
    Block labels, strings;
    std::size_t offset = 0x20;
    for (unsigned i = 0; i < count; ++i) {
        require(contains(size, offset, 16), "MSBT block header outside file");
        const std::size_t length = readU32BE(bytes.data + offset + 4);
        require(contains(size, offset + 16, length), "MSBT block payload outside file");
        Block block{bytes.data + offset + 16, length};
        if (std::memcmp(bytes.data + offset, "LBL1", 4) == 0) { require(!labels.data, "duplicate LBL1"); labels = block; }
        if (std::memcmp(bytes.data + offset, "TXT2", 4) == 0) { require(!strings.data, "duplicate TXT2"); strings = block; }
        offset = (offset + 16 + length + 15) & ~std::size_t(15);
    }
    require(labels.data && strings.data, "MSBT requires LBL1 and TXT2");
    const auto buckets = u32(labels, 0);
    require(buckets > 0 && buckets <= (labels.size - 4) / 8, "invalid MSBT label bucket table");
    EnglishNames names;
    for (std::uint32_t bucket = 0; bucket < buckets; ++bucket) {
        const auto entries = u32(labels, 4 + bucket * 8);
        std::size_t at = u32(labels, 8 + bucket * 8);
        require(entries <= labels.size / 5 && (entries == 0 || at >= 4 + buckets * 8), "invalid MSBT label bucket entries");
        for (std::uint32_t entry = 0; entry < entries; ++entry) {
            require(contains(labels.size, at, 1), "MSBT label outside LBL1");
            const std::size_t length = labels.data[at++];
            require(length > 0 && contains(labels.size, at, length + 4), "MSBT label payload outside LBL1");
            std::string label(reinterpret_cast<const char*>(labels.data + at), length);
            require(label.find('\0') == std::string::npos, "embedded NUL in MSBT label");
            const auto index = u32(labels, at + length); at += length + 4;
            constexpr char prefix[] = "GalaxyName_";
            if (label.compare(0, sizeof(prefix) - 1, prefix) != 0) continue;
            const std::string stage = label.substr(sizeof(prefix) - 1);
            require(!stage.empty() && stage.size() < 64 && stage.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789") == std::string::npos, "invalid GalaxyName stage suffix");
            std::uint32_t hash = 0;
            for (unsigned char c : label) hash = hash * 0x492u + c;
            require(hash % buckets == bucket, "GalaxyName label is in the wrong LMS hash bucket");
            merge(names, stage, {text(strings, index), origin + ":" + label});
        }
    }
    return names;
}

std::vector<std::uint8_t> readFile(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) throw std::runtime_error("cannot open English message archive: " + path.string());
    const auto size = file.tellg();
    require(size > 0 && size <= 64 * 1024 * 1024, "English message archive exceeds size limit");
    std::vector<std::uint8_t> data(static_cast<std::size_t>(size));
    file.seekg(0);
    require(static_cast<bool>(file.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(data.size()))), "cannot read English message archive");
    return data;
}
}  // namespace

bool readEnglishGalaxyNames(Resource::Bytes msbt, const std::string& origin, EnglishNames* names, std::string* error) {
    try {
        if (!names || origin.empty()) throw std::runtime_error("English name output/provenance is required");
        auto next = *names;
        for (auto& entry : decode(msbt, origin)) merge(next, entry.first, std::move(entry.second));
        names->swap(next);
        if (error) error->clear();
        return true;
    } catch (const std::exception& e) { if (error) *error = e.what(); return false; }
}

bool loadEnglishGalaxyNames(const std::filesystem::path& disc, EnglishNames* names, std::string* error) {
    try {
        if (!names) throw std::runtime_error("English name output is required");
        const auto files = std::filesystem::is_directory(disc / "files") ? disc / "files" : disc;
        EnglishNames next;
        for (const char* filename : {"SystemMessage.arc", "LayoutMessage.arc"}) {
            const std::string relative = std::string("LocalizeData/UsEnglish/MessageData/") + filename;
            const auto bytes = readFile(files / relative);
            const auto archive = Resource::Archive::parse({bytes.data(), bytes.size()}, 64 * 1024 * 1024);
            for (std::size_t i = 0; i < archive.entries().size(); ++i) {
                const auto& entry = archive.entries()[i];
                if (entry.isDirectory() || entry.name.size() < 5 || entry.name.compare(entry.name.size() - 5, 5, ".msbt") != 0) continue;
                const auto resource = archive.resourceData(i, 16 * 1024 * 1024);
                for (auto& name : decode({resource.data(), resource.size()}, relative + "/" + entry.name)) merge(next, name.first, std::move(name.second));
            }
        }
        require(!next.empty(), "English message archives contain no GalaxyName labels");
        names->swap(next);
        if (error) error->clear();
        return true;
    } catch (const std::exception& e) { if (error) *error = e.what(); return false; }
}
}  // namespace PetariNative::App::LaunchStage
