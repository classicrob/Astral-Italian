#pragma once

#include "Game/Player/GhostPacket.hpp"
#include "Game/Player/GhostPacketWriter.hpp"
#include <cstring>
#include <climits>
#if !defined(__MWERKS__)
#include <cfloat>
#endif

#if defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && defined(__ORDER_LITTLE_ENDIAN__)
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define GHOST_PACKET_BIG_ENDIAN 1
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define GHOST_PACKET_BIG_ENDIAN 0
#else
#error Unsupported Ghost packet byte order
#endif
#elif defined(__MWERKS__) && defined(__BIG_ENDIAN__) && __BIG_ENDIAN__ == 1
#define GHOST_PACKET_BIG_ENDIAN 1
#else
#error Ghost packets require a known compiler byte order
#endif

namespace GhostPacketWire {
    typedef char WireTypesRequireExactWidths[(CHAR_BIT == 8 && sizeof(u16) == 2 && sizeof(s16) == 2 && sizeof(u32) == 4 && sizeof(s32) == 4) ? 1 : -1];
    typedef char WireFloatRequiresFourBytes[sizeof(f32) == 4 ? 1 : -1];
#if !defined(__MWERKS__)
    typedef char WireFloatRequiresBinary32[(FLT_RADIX == 2 && FLT_MANT_DIG == 24 && FLT_MAX_EXP == 128) ? 1 : -1];
#endif

    inline void readU16(GhostPacket& packet, u16& value) {
#if GHOST_PACKET_BIG_ENDIAN
        packet.read(reinterpret_cast<u8*>(&value), 2);
#else
        u8 bytes[2];
        packet.read(bytes, 2);
        value = static_cast<u16>((static_cast<u16>(bytes[0]) << 8) | bytes[1]);
#endif
    }

    inline void readS16(GhostPacket& packet, s16& value) {
#if GHOST_PACKET_BIG_ENDIAN
        packet.read(reinterpret_cast<u8*>(&value), 2);
#else
        u16 bits;
        readU16(packet, bits);
        value = static_cast<s16>(bits < 0x8000 ? static_cast<s32>(bits) : static_cast<s32>(bits) - 0x10000);
#endif
    }

    inline void readU32(GhostPacket& packet, u32& value) {
#if GHOST_PACKET_BIG_ENDIAN
        packet.read(reinterpret_cast<u8*>(&value), 4);
#else
        u8 bytes[4];
        packet.read(bytes, 4);
        value = (static_cast<u32>(bytes[0]) << 24) | (static_cast<u32>(bytes[1]) << 16) |
            (static_cast<u32>(bytes[2]) << 8) | static_cast<u32>(bytes[3]);
#endif
    }

    inline void readF32(GhostPacket& packet, f32& value) {
#if GHOST_PACKET_BIG_ENDIAN
        packet.read(reinterpret_cast<u8*>(&value), 4);
#else
        u32 bits;
        readU32(packet, bits);
        std::memcpy(&value, &bits, 4);
#endif
    }

    inline void writeU16(GhostPacketWriter& packet, const u16& value) {
#if GHOST_PACKET_BIG_ENDIAN
        packet.write(reinterpret_cast<const u8*>(&value), 2);
#else
        u8 bytes[2] = {static_cast<u8>(value >> 8), static_cast<u8>(value)};
        packet.write(bytes, 2);
#endif
    }

    inline void writeS16(GhostPacketWriter& packet, const s16& value) {
#if GHOST_PACKET_BIG_ENDIAN
        packet.write(reinterpret_cast<const u8*>(&value), 2);
#else
        u16 bits = static_cast<u16>(value);
        writeU16(packet, bits);
#endif
    }

    inline void writeU32(GhostPacketWriter& packet, const u32& value) {
#if GHOST_PACKET_BIG_ENDIAN
        packet.write(reinterpret_cast<const u8*>(&value), 4);
#else
        u8 bytes[4] = {static_cast<u8>(value >> 24), static_cast<u8>(value >> 16),
            static_cast<u8>(value >> 8), static_cast<u8>(value)};
        packet.write(bytes, 4);
#endif
    }

    inline void writeF32(GhostPacketWriter& packet, const f32& value) {
#if GHOST_PACKET_BIG_ENDIAN
        packet.write(reinterpret_cast<const u8*>(&value), 4);
#else
        u32 bits;
        std::memcpy(&bits, &value, 4);
        writeU32(packet, bits);
#endif
    }
}

#undef GHOST_PACKET_BIG_ENDIAN
