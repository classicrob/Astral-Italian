#pragma once

#include <cstdint>
#include <cstring>
#include <type_traits>

namespace PetariNative::SaveWire {
// Save files are byte streams, including fields at odd offsets.
template<class T> T load(const void* address) {
    using U = std::make_unsigned_t<T>;
    const auto* bytes = static_cast<const unsigned char*>(address);
    U value = 0;
    for (unsigned i = 0; i < sizeof(T); ++i) value = (value << 8) | bytes[i];
    T result;
    std::memcpy(&result, &value, sizeof(T));
    return result;
}
template<class T> void store(void* address, T value) {
    using U = std::make_unsigned_t<T>;
    U bits;
    std::memcpy(&bits, &value, sizeof(T));
    auto* bytes = static_cast<unsigned char*>(address);
    for (unsigned i = 0; i < sizeof(T); ++i) {
        bytes[sizeof(T) - 1 - i] = static_cast<unsigned char>(bits);
        bits >>= 8;
    }
}
template<class T> struct Value {
    unsigned char bytes[sizeof(T)];
    operator T() const { return load<T>(bytes); }
    Value& operator=(T value) { store(bytes, value); return *this; }
};
template<class T, class Stream> void write(Stream& stream, T value) {
    unsigned char bytes[sizeof(T)];
    store(bytes, value);
    stream.write(bytes, sizeof(bytes));
}
template<class T, class Stream> T read(Stream& stream) {
    unsigned char bytes[sizeof(T)]{};
    stream.read(bytes, sizeof(bytes));
    return load<T>(bytes);
}
inline std::uint32_t checksum(const void* data, std::uint32_t size) {
    const auto* bytes = static_cast<const unsigned char*>(data);
    std::uint16_t sum = 0, inverse = 0;
    for (std::uint32_t i = 0; i + 1 < size; i += 2) {
        const auto word = load<std::uint16_t>(bytes + i);
        sum = static_cast<std::uint16_t>(sum + word);
        inverse = static_cast<std::uint16_t>(inverse + static_cast<std::uint16_t>(~word));
    }
    return (std::uint32_t(sum) << 16) | inverse;
}
} // namespace PetariNative::SaveWire
