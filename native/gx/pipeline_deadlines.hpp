#pragma once
// PFD1 uses exact first-use renderer frames. It never alters a manifest's ranks,
// configs, shaders or coverage. A malformed/missing sidecar is an order fallback.
#include <algorithm>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <map>
#include <span>
#include <string>
#include <vector>

namespace PetariDeadlines {
struct Row {
    std::uint8_t type;
    std::uint64_t key;
    std::uint16_t version;
    std::span<const unsigned char> config;
    std::uint32_t rank;
};
inline std::uint64_t fingerprint(std::vector<Row> rows) {
    std::sort(rows.begin(), rows.end(), [](const Row& a, const Row& b) {
        return a.type != b.type ? a.type < b.type : a.key < b.key;
    });
    std::uint64_t hash = UINT64_C(14695981039346656037);
    const auto byte = [&](unsigned char value) { hash = (hash ^ value) * UINT64_C(1099511628211); };
    const auto number = [&](std::uint64_t value, unsigned bytes) {
        for (unsigned n = 0; n < bytes; ++n) byte(static_cast<unsigned char>(value >> (8 * n)));
    };
    for (const auto& row : rows) {
        number(row.type, 1); number(row.key, 8); number(row.version, 2);
        number(row.config.size(), 2); number(row.rank, 4);
        for (auto value : row.config) byte(value);
    }
    return hash;
}
struct Result {
    bool valid = false;
    std::string reason;
    std::map<std::uint64_t, std::uint32_t> frames;
};
inline Result decode(std::span<const unsigned char> data, const std::string& stage, const std::vector<Row>& rows) {
    const auto bad = [](const char* reason) { return Result{false, reason, {}}; };
    constexpr std::size_t Header = 24, MaxRows = 65536;
    if (data.size() < Header || data.size() > Header + 80 + MaxRows * 12) return bad("file-size");
    const auto number = [&](std::size_t at, unsigned bytes) {
        std::uint64_t value = 0;
        for (unsigned n = 0; n < bytes; ++n) value |= std::uint64_t(data[at + n]) << (8 * n);
        return value;
    };
    if (data[0] != 'P' || data[1] != 'F' || data[2] != 'D' || data[3] != '1' ||
        number(4, 2) != 1 || number(6, 2) != 13 || number(10, 2)) return bad("header-version");
    const auto length = number(8, 2), count = number(20, 4);
    if (!length || length > 80 || !count || count > MaxRows || data.size() != Header + length + 12 * count)
        return bad("length-count-trailing");
    if (std::string(reinterpret_cast<const char*>(data.data() + Header), length) != stage ||
        number(12, 8) != fingerprint(rows)) return bad("stage-manifest-binding");
    std::map<std::uint64_t, bool> keys;
    for (const auto& row : rows) if (row.type == 1 && row.version == 13) keys.emplace(row.key, true);
    Result result{true, "validated", {}};
    std::uint64_t previous = 0;
    for (std::size_t n = 0; n < count; ++n) {
        const auto at = Header + length + 12 * n;
        const auto key = number(at, 8), frame = number(at + 8, 4);
        if ((n && key <= previous) || !keys.contains(key) || frame == UINT32_MAX)
            return bad("duplicate-order-unknown-frame");
        result.frames.emplace(key, static_cast<std::uint32_t>(frame)); previous = key;
    }
    return result;
}
inline Result load(const std::filesystem::path& path, const std::string& stage, const std::vector<Row>& rows) {
    try {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file) return {false, "missing-unreadable", {}};
        const auto length = static_cast<std::streamoff>(file.tellg());
        if (length < 24 || length > 24 + 80 + 65536 * 12) return {false, "file-size", {}};
        std::vector<unsigned char> data(static_cast<std::size_t>(length));
        file.seekg(0);
        if (!file.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(length))) return {false, "read-failed", {}};
        return decode(data, stage, rows);
    } catch (const std::exception&) {
        return {false, "read-exception", {}};
    }
}
} // namespace PetariDeadlines
