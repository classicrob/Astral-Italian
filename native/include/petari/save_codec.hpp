#pragma once

#include <petari/save_wire.hpp>
#include <algorithm>
#include <array>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace PetariNative::SaveWire {
using Bytes = std::vector<unsigned char>;
inline void require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}
inline std::uint32_t little32(const unsigned char* p) {
    return std::uint32_t(p[0]) | (std::uint32_t(p[1]) << 8) | (std::uint32_t(p[2]) << 16) | (std::uint32_t(p[3]) << 24);
}
inline std::uint32_t legacyChecksum(const unsigned char* bytes, std::size_t size) {
    std::uint16_t sum = 0, inverse = 0;
    for (std::size_t i = 0; i + 1 < size; i += 2) {
        const std::uint16_t word = bytes[i] | (std::uint16_t(bytes[i + 1]) << 8);
        sum = static_cast<std::uint16_t>(sum + word);
        inverse = static_cast<std::uint16_t>(inverse + static_cast<std::uint16_t>(~word));
    }
    return (std::uint32_t(sum) << 16) | inverse;
}
inline std::uint32_t hashCode(const char* name) {
    std::uint32_t hash = 0;
    for (; *name; ++name) hash = hash * 31 + static_cast<signed char>(*name);
    return hash;
}
inline std::uint16_t nameHash(const char* name) { return static_cast<std::uint16_t>(hashCode(name)); }
inline std::uint32_t headerHash(std::uint32_t signature) {
    switch (signature) {
    case 0x504c4159: case 0x47414c41: return 32;
    case 0x464c4731: return hashCode("2bytes/flag");
    case 0x53544631: return hashCode("SaveDataStorageTicoFat") + 0x120;
    case 0x564c4531: return signature;
    case 0x5353574d: return hashCode("SaveDataStorageWorldMap") * 9;
    case 0x434f4e46: return 0x2432da;
    case 0x4d494920: return 0x2836e9;
    case 0x4d495343: return 1;
    case 0x53595343: return 3;
    default: throw std::runtime_error("unsupported save chunk");
    }
}
struct Field { const char* name; unsigned width; };
inline constexpr Field playerFields[] = {{"mPlayerLeft",1}, {"mStockedStarPieceNum",2}, {"mStockedCoinNum",2},
    {"mLast1upCoinNum",2}, {"mFlag",1}};
inline constexpr Field galaxyFields[] = {{"mGalaxyName",2}, {"mDataSize",2}, {"mScenarioNum",1}, {"mGalaxyState",1}, {"mFlag",1}};
inline constexpr Field scenarioFields[] = {{"mMissNum",1}, {"mBestTime",4}, {"mFlag",1}};
inline constexpr Field systemFields[] = {{"mIsEncouragePal60",1}, {"mTimeSent",8}, {"mSentBytes",4}, {"mBankStarPieceNum",2},
    {"mBankStarPieceMax",2}, {"mGiftedPlayerLeft",1}, {"mGiftedFileNameHash",2}};

class Payload {
public:
    Bytes data;
    bool legacy;
    explicit Payload(Bytes bytes, bool old) : data(std::move(bytes)), legacy(old) {}
    void range(std::size_t offset, std::size_t count) const {
        require(offset <= data.size() && count <= data.size() - offset, "truncated save payload");
    }
    std::uint32_t number(std::size_t offset, unsigned width) const {
        range(offset, width);
        require(width <= 4, "invalid numeric field width");
        std::uint32_t result = 0;
        for (unsigned i = 0; i < width; ++i) result = (result << 8) | data[offset + (legacy ? width - 1 - i : i)];
        return result;
    }
    void scalar(std::size_t offset, unsigned width) {
        range(offset, width);
        if (legacy) std::reverse(data.begin() + offset, data.begin() + offset + width);
    }
    struct Attribute { std::uint16_t hash; unsigned offset, width; };
    struct Header {
        unsigned length, size;
        std::vector<Attribute> attributes;
        unsigned offset(const char* name) const {
            for (auto a : attributes) if (a.hash == nameHash(name)) return a.offset;
            throw std::runtime_error("required save attribute missing");
        }
    };
    template<std::size_t N> Header header(std::size_t offset, const Field (&fields)[N]) {
        const auto count = number(offset, 2), size = number(offset + 2, 2);
        require(count == N, "unsupported save attribute schema");
        range(offset, 4 + count * 4);
        Header result{static_cast<unsigned>(4 + count * 4), static_cast<unsigned>(size), {}};
        std::set<std::uint16_t> seen;
        std::vector<bool> occupied(size);
        for (unsigned i = 0; i < count; ++i) {
            const auto at = offset + 4 + i * 4;
            const auto hash = static_cast<std::uint16_t>(number(at, 2));
            const auto position = number(at + 2, 2);
            auto field = std::find_if(std::begin(fields), std::end(fields), [=](const Field& f) {return nameHash(f.name) == hash;});
            require(field != std::end(fields) && seen.insert(hash).second, "unknown or repeated save attribute");
            require(position <= size && field->width <= size - position, "save attribute outside data");
            for (unsigned j = 0; j < field->width; ++j) {
                require(!occupied[position + j], "overlapping save attributes");
                occupied[position + j] = true;
            }
            result.attributes.push_back({hash, static_cast<unsigned>(position), field->width});
        }
        for (unsigned i = 0; i < 4 + count * 4; i += 2) scalar(offset + i, 2);
        return result;
    }
    void record(const Header& header, std::size_t offset) {
        range(offset, header.size);
        for (auto a : header.attributes) scalar(offset + a.offset, a.width);
    }
    void finish(std::uint32_t signature) {
        switch (signature) {
        case 0x504c4159: {
            auto h = header(0, playerFields);
            require(h.length + h.size == data.size(), "invalid player payload size");
            record(h, h.length);
            break;
        }
        case 0x53595343: {
            auto h = header(0, systemFields);
            require(h.length + h.size == data.size(), "invalid system payload size");
            record(h, h.length);
            break;
        }
        case 0x47414c41: {
            const auto count = number(0, 2);
            require(count <= 96, "too many saved galaxies");
            auto g = header(2, galaxyFields);
            auto s = header(2 + g.length, scenarioFields);
            std::size_t offset = 2 + g.length + s.length;
            std::set<std::uint16_t> names;
            for (unsigned i = 0; i < count; ++i) {
                range(offset, g.size);
                const auto name = static_cast<std::uint16_t>(number(offset + g.offset("mGalaxyName"), 2));
                const auto length = number(offset + g.offset("mDataSize"), 2);
                const auto scenarios = number(offset + g.offset("mScenarioNum"), 1);
                require(name && names.insert(name).second && scenarios > 0 && scenarios <= 32 &&
                        length == g.size + scenarios * s.size, "invalid saved galaxy record");
                range(offset, length);
                record(g, offset);
                for (unsigned j = 0; j < scenarios; ++j) record(s, offset + g.size + j * s.size);
                offset += length;
            }
            require(offset == data.size(), "unexpected galaxy payload tail");
            scalar(0, 2);
            break;
        }
        case 0x464c4731: // FLG1: hash and state share one u16.
            require(data.size() % 2 == 0, "invalid flag payload");
            for (std::size_t i = 0; i < data.size(); i += 2) scalar(i, 2);
            break;
        case 0x564c4531: // VLE1: (hash, value) pairs.
            require(data.size() % 4 == 0, "invalid event-value payload");
            for (std::size_t i = 0; i < data.size(); i += 2) scalar(i, 2);
            break;
        case 0x53544631:
            require(data.size() == 128, "invalid Hungry Luma payload");
            for (unsigned i = 0; i < 128; i += 2) scalar(i, 2);
            break;
        case 0x4d495343:
            require(data.size() == 8, "invalid config timestamp");
            scalar(0, 8);
            break;
        case 0x5353574d: require(data.size() == 9, "invalid world-map payload"); break;
        case 0x434f4e46: require(data.size() == 1, "invalid creation payload"); break;
        case 0x4d494920: require(data.size() == 10, "invalid Mii payload"); break;
        default: throw std::runtime_error("unsupported save chunk; refusing migration");
        }
    }
};
struct Record { std::string name; unsigned offset, size; };
inline std::vector<Record> directory(const Bytes& bytes, bool legacy, bool verifyChecksum = true) {
    require(bytes.size() >= 16 && bytes.size() <= 0x8000, "invalid save length");
    auto word = [&](unsigned at) {return legacy ? little32(bytes.data() + at) : load<std::uint32_t>(bytes.data() + at);};
    const auto size = word(12), count = word(8);
    require(word(4) == 3 && count == 7 && size >= 128 && size < 0x8000 && bytes.size() == ((size + 31u) & ~31u), "invalid save header");
    require(!verifyChecksum || word(0) == (legacy ? legacyChecksum(bytes.data() + 4, size - 4) : checksum(bytes.data() + 4, size - 4)), "save checksum mismatch");
    std::vector<Record> result;
    constexpr const char* names[] = {"user1","config1","user2","config2","user3","config3","sysconf"};
    unsigned end = size;
    for (unsigned i = count; i-- > 0;) {
        const auto at = 16 + i * 16;
        require(std::memchr(bytes.data() + at, 0, 12), "unterminated save record name");
        const std::string name(reinterpret_cast<const char*>(bytes.data() + at));
        const auto offset = word(at + 12);
        require(name == names[i] && offset >= 128 && offset < end, "invalid save directory");
        result.push_back({name, offset, end - offset});
        end = offset;
    }
    std::reverse(result.begin(), result.end());
    return result;
}
inline Bytes canonical(const Bytes& bytes, bool legacy, bool verifyChecksum = true) {
    const auto records = directory(bytes, legacy, verifyChecksum);
    Bytes output = bytes;
    if (legacy) {
        for (unsigned i = 0; i < 16; i += 4) store<std::uint32_t>(output.data() + i, little32(bytes.data() + i));
        for (unsigned i = 0; i < 7; ++i) store<std::uint32_t>(output.data() + 28 + i * 16, records[i].offset);
    }
    for (auto r : records) {
        require(r.size >= 4 && bytes[r.offset] == 2 && bytes[r.offset + 2] == 0 && bytes[r.offset + 3] == 0, "unsupported save record version");
        const unsigned count = bytes[r.offset + 1];
        const bool user = r.name.compare(0, 4, "user") == 0;
        const bool system = r.name == "sysconf";
        require(count == (user ? 6 : system ? 1 : 3), "unexpected save chunk count");
        std::size_t from = r.offset + 4, to = from;
        const auto end = r.offset + r.size;
        std::set<std::uint32_t> signatures;
        if (legacy) std::fill(output.begin() + to, output.begin() + end, 0);
        for (unsigned i = 0; i < count; ++i) {
            const unsigned header = legacy ? 16 : 12;
            require(from <= end && header <= end - from, "truncated save chunk header");
            auto word = [&](unsigned at) {return legacy ? little32(bytes.data() + from + at) : load<std::uint32_t>(bytes.data() + from + at);};
            const auto signature = word(0), hash = word(4), size = word(8);
            require(size >= header && size <= end - from && signatures.insert(signature).second, "invalid save chunk extent");
            const bool allowed = user ? (signature == 0x504c4159 || signature == 0x464c4731 || signature == 0x53544631 || signature == 0x564c4531 || signature == 0x47414c41 || signature == 0x5353574d)
                : system ? signature == 0x53595343 : (signature == 0x434f4e46 || signature == 0x4d494920 || signature == 0x4d495343);
            require(allowed, "chunk in wrong save record");
            require(hash == headerHash(signature), "unsupported save chunk hash");
            Payload payload(Bytes(bytes.begin() + from + header, bytes.begin() + from + size), legacy);
            payload.finish(signature);
            if (legacy) {
                store<std::uint32_t>(output.data() + to, signature);
                store<std::uint32_t>(output.data() + to + 4, hash);
                store<std::uint32_t>(output.data() + to + 8, static_cast<std::uint32_t>(payload.data.size() + 12));
                std::copy(payload.data.begin(), payload.data.end(), output.begin() + to + 12);
            }
            from += size;
            to += payload.data.size() + 12;
        }
    }
    if (legacy) store<std::uint32_t>(output.data(), checksum(output.data() + 4, load<std::uint32_t>(output.data() + 12) - 4));
    return output;
}
} // namespace PetariNative::SaveWire
