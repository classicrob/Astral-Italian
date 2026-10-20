// MSL printf semantics for legacy narrow-text code. See msl_stdio.h.
#include <petari/msl_stdio.h>

#include <cstdint>
#include <cstdio>
#include <cstring>

namespace {
struct Output {
    char* dst;
    size_t capacity;  // bytes dst can hold (SIZE_MAX for the sprintf forms)
    size_t length;    // formatted length so far, including what did not fit
};

void write(Output& out, const char* src, size_t count) {
    if (out.dst != nullptr && out.length < out.capacity) {
        const size_t room = out.capacity - out.length;
        std::memcpy(out.dst + out.length, src, count < room ? count : room);
    }
    out.length += count;
}

void pad(Output& out, size_t count) {
    for (size_t i = 0; i < count; i++) {
        write(out, " ", 1);
    }
}

enum Length { kNoLength, kChar, kShort, kLong, kLongLong, kIntmax, kSize, kPtrdiff, kLongDouble };

struct Spec {
    char flags[8];
    int flagCount;
    bool leftJustify;
    bool hasWidth;
    int width;
    bool hasPrecision;
    int precision;
    Length length;
    char conversion;
};

bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

// Parses the conversion after '%'. Returns the character after it, or nullptr for an invalid conversion.
const char* parse(const char* s, va_list* args, Spec* spec) {
    std::memset(spec, 0, sizeof(*spec));
    for (; std::strchr("-+ #0", *s) != nullptr && *s != '\0'; s++) {
        if (*s == '-') {
            spec->leftJustify = true;
        }
        if (spec->flagCount < static_cast< int >(sizeof(spec->flags)) - 1) {
            spec->flags[spec->flagCount++] = *s;
        }
    }
    if (*s == '*') {
        int width = va_arg(*args, int);
        if (width < 0) {
            spec->leftJustify = true;
            width = -width;
        }
        spec->hasWidth = true;
        spec->width = width;
        s++;
    } else if (isDigit(*s)) {
        spec->hasWidth = true;
        for (; isDigit(*s); s++) {
            spec->width = spec->width * 10 + (*s - '0');
        }
    }
    if (*s == '.') {
        s++;
        spec->hasPrecision = true;
        if (*s == '*') {
            const int precision = va_arg(*args, int);
            spec->hasPrecision = precision >= 0;
            spec->precision = precision < 0 ? 0 : precision;
            s++;
        } else {
            for (; isDigit(*s); s++) {
                spec->precision = spec->precision * 10 + (*s - '0');
            }
        }
    }
    switch (*s) {
    case 'h':
        spec->length = s[1] == 'h' ? kChar : kShort;
        s += s[1] == 'h' ? 2 : 1;
        break;
    case 'l':
        spec->length = s[1] == 'l' ? kLongLong : kLong;
        s += s[1] == 'l' ? 2 : 1;
        break;
    case 'L':
        spec->length = kLongDouble;
        s++;
        break;
    case 'j':
        spec->length = kIntmax;
        s++;
        break;
    case 'z':
        spec->length = kSize;
        s++;
        break;
    case 't':
        spec->length = kPtrdiff;
        s++;
        break;
    default:
        break;
    }
    spec->conversion = *s;
    switch (*s) {
    case 'd':
    case 'i':
    case 'o':
    case 'u':
    case 'x':
    case 'X':
        return spec->length == kLongDouble ? nullptr : s + 1;
    case 'f':
    case 'F':
    case 'e':
    case 'E':
    case 'g':
    case 'G':
    case 'a':
    case 'A':
        return spec->length == kNoLength || spec->length == kLongDouble || spec->length == kLong ? s + 1 : nullptr;
    case 'c':
    case 's':
        return spec->length == kNoLength || spec->length == kLong ? s + 1 : nullptr;
    case 'p':
        return spec->length == kNoLength ? s + 1 : nullptr;
    case 'n':
        return spec->length == kLongDouble ? nullptr : s + 1;
    default:
        return nullptr;
    }
}

// The host printf spec for one conversion, with `hostLength` in place of the legacy length modifier.
void hostSpec(char* out, size_t size, const Spec& spec, const char* hostLength) {
    int used = std::snprintf(out, size, "%%%s", spec.flags);
    if (spec.hasWidth) {
        used += std::snprintf(out + used, size - used, "%d", spec.width);
    }
    if (spec.hasPrecision) {
        used += std::snprintf(out + used, size - used, ".%d", spec.precision);
    }
    std::snprintf(out + used, size - used, "%s%c", hostLength, spec.conversion);
}

template < typename T >
void writeHost(Output& out, const Spec& spec, const char* hostLength, T value) {
    char format[48];
    hostSpec(format, sizeof(format), spec, hostLength);
    char buffer[512];  // MSL's conversion buffer is 512 bytes as well
    const int count = std::snprintf(buffer, sizeof(buffer), format, value);
    if (count > 0) {
        write(out, buffer, static_cast< size_t >(count) < sizeof(buffer) ? static_cast< size_t >(count) : sizeof(buffer) - 1);
    }
}

void writePadded(Output& out, const Spec& spec, size_t count, const char* narrow, const char16_t* wide) {
    const size_t fill = spec.hasWidth && static_cast< size_t >(spec.width) > count ? spec.width - count : 0;
    if (!spec.leftJustify) {
        pad(out, fill);
    }
    if (narrow != nullptr) {
        write(out, narrow, count);
    } else {
        for (size_t i = 0; i < count; i++) {
            const char c = static_cast< char >(wide[i] & 0xFF);
            write(out, &c, 1);
        }
    }
    if (spec.leftJustify) {
        pad(out, fill);
    }
}

void formatSigned(Output& out, const Spec& spec, va_list* args) {
    switch (spec.length) {
    case kChar:
        writeHost(out, spec, "hh", va_arg(*args, int));
        break;
    case kShort:
        writeHost(out, spec, "h", va_arg(*args, int));
        break;
    case kLongLong:
        writeHost(out, spec, "ll", va_arg(*args, long long));
        break;
    case kIntmax:
        writeHost(out, spec, "j", va_arg(*args, intmax_t));
        break;
    case kSize:
        writeHost(out, spec, "z", va_arg(*args, size_t));
        break;
    case kPtrdiff:
        writeHost(out, spec, "t", va_arg(*args, ptrdiff_t));
        break;
    default:  // none, and Wii `long` (32 bits)
        writeHost(out, spec, "", va_arg(*args, int));
        break;
    }
}

void formatUnsigned(Output& out, const Spec& spec, va_list* args) {
    switch (spec.length) {
    case kChar:
        writeHost(out, spec, "hh", va_arg(*args, unsigned int));
        break;
    case kShort:
        writeHost(out, spec, "h", va_arg(*args, unsigned int));
        break;
    case kLongLong:
        writeHost(out, spec, "ll", va_arg(*args, unsigned long long));
        break;
    case kIntmax:
        writeHost(out, spec, "j", va_arg(*args, uintmax_t));
        break;
    case kSize:
        writeHost(out, spec, "z", va_arg(*args, size_t));
        break;
    case kPtrdiff:
        writeHost(out, spec, "t", va_arg(*args, ptrdiff_t));
        break;
    default:  // none, and Wii `long` (32 bits)
        writeHost(out, spec, "", va_arg(*args, unsigned int));
        break;
    }
}

void storeCount(const Spec& spec, va_list* args, size_t count) {
    switch (spec.length) {
    case kChar:
        *va_arg(*args, signed char*) = static_cast< signed char >(count);
        break;
    case kShort:
        *va_arg(*args, short*) = static_cast< short >(count);
        break;
    case kLongLong:
        *va_arg(*args, long long*) = static_cast< long long >(count);
        break;
    case kIntmax:
        *va_arg(*args, intmax_t*) = static_cast< intmax_t >(count);
        break;
    case kSize:
        *va_arg(*args, size_t*) = count;
        break;
    case kPtrdiff:
        *va_arg(*args, ptrdiff_t*) = static_cast< ptrdiff_t >(count);
        break;
    default:  // none, and Wii `long` (32 bits)
        *va_arg(*args, int*) = static_cast< int >(count);
        break;
    }
}

void format(Output& out, const char* s, va_list* args) {
    while (*s != '\0') {
        const char* percent = std::strchr(s, '%');
        if (percent == nullptr) {
            write(out, s, std::strlen(s));
            return;
        }
        write(out, s, percent - s);
        if (percent[1] == '%') {
            write(out, "%", 1);
            s = percent + 2;
            continue;
        }
        Spec spec;
        const char* next = parse(percent + 1, args, &spec);
        if (next == nullptr) {
            // MSL: an invalid conversion ends formatting with the rest of the format copied literally.
            write(out, percent, std::strlen(percent));
            return;
        }
        switch (spec.conversion) {
        case 'd':
        case 'i':
            formatSigned(out, spec, args);
            break;
        case 'o':
        case 'u':
        case 'x':
        case 'X':
            formatUnsigned(out, spec, args);
            break;
        case 'f':
        case 'F':
        case 'e':
        case 'E':
        case 'g':
        case 'G':
        case 'a':
        case 'A':
            if (spec.length == kLongDouble) {
                writeHost(out, spec, "L", va_arg(*args, long double));
            } else {
                writeHost(out, spec, "", va_arg(*args, double));
            }
            break;
        case 'p':
            writeHost(out, spec, "", va_arg(*args, void*));
            break;
        case 'c': {
            const char c = static_cast< char >(va_arg(*args, int));
            writePadded(out, spec, 1, &c, nullptr);
            break;
        }
        case 's':
            if (spec.length == kLong) {
                const char16_t* wide = va_arg(*args, const char16_t*);
                if (wide == nullptr) {
                    wide = u"";  // MSL: a null %ls prints nothing
                }
                size_t count = 0;
                while (wide[count] != 0 && (!spec.hasPrecision || count < static_cast< size_t >(spec.precision))) {
                    count++;
                }
                writePadded(out, spec, count, nullptr, wide);
            } else {
                const char* narrow = va_arg(*args, const char*);
                if (narrow == nullptr) {
                    narrow = "";  // MSL: a null %s prints nothing (the host prints "(null)")
                }
                const size_t count = spec.hasPrecision ? strnlen(narrow, spec.precision) : std::strlen(narrow);
                writePadded(out, spec, count, narrow, nullptr);
            }
            break;
        case 'n':
            storeCount(spec, args, out.length);
            break;
        }
        s = next;
    }
}

int formatTo(char* dst, size_t n, const char* formatString, va_list args) {
    Output out = {dst, n, 0};
    va_list copy;
    va_copy(copy, args);
    format(out, formatString, &copy);
    va_end(copy);
    if (dst != nullptr && n > 0) {
        dst[out.length < n ? out.length : n - 1] = '\0';
    }
    return static_cast< int >(out.length);
}
}  // namespace

extern "C" {

int petari_msl_vsnprintf(char* dst, size_t n, const char* formatString, va_list args) {
    return formatTo(dst, n, formatString, args);
}

int petari_msl_snprintf(char* dst, size_t n, const char* formatString, ...) {
    va_list args;
    va_start(args, formatString);
    const int count = formatTo(dst, n, formatString, args);
    va_end(args);
    return count;
}

int petari_msl_vsprintf(char* dst, const char* formatString, va_list args) {
    return formatTo(dst, SIZE_MAX, formatString, args);
}

int petari_msl_sprintf(char* dst, const char* formatString, ...) {
    va_list args;
    va_start(args, formatString);
    const int count = formatTo(dst, SIZE_MAX, formatString, args);
    va_end(args);
    return count;
}

}  // extern "C"
