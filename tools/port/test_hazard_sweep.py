#!/usr/bin/env python3
"""Focused regression checks for the lexical native hazard sweep."""

import unittest

from hazard_sweep import Source, Sweep, format_for, mask, directory_for, lane_for


def scan(text, path='src/Game/Util/Probe.cpp', headers=None):
    source = Source(path, text)
    sources = {path: source}
    sources.update({name: Source(name, content) for name, content in (headers or {}).items()})
    sweep = Sweep(sources)
    sweep.scan(source)
    return list(sweep.findings.values()), sweep


class HazardSweepTests(unittest.TestCase):
    def test_numeric_casts_and_pointer_difference_are_suppressed(self):
        findings, _ = scan('''void f(void* p, void* q, f32 floatValue, u8* bytes) {
    u32 a = u32(floatValue);
    s32 b = (s32)floatValue;
    u32 c = (u32)bytes[2];
    u32 d = (u32)((const char*)p - (const char*)q);
    u32 e = (u32)sizeof(void*);
}''')
        self.assertFalse([f for f in findings if f['class'] in {'pointer-truncation', 'cast-unresolved'}], findings)

    def test_cast_forms_and_member_header_lookup(self):
        findings, _ = scan('''#include "Game/Util/Probe.hpp"
void f(void* p) {
    u32 a = (u32)p;
    s32 b = reinterpret_cast<s32>(p);
    int c = int(p);
    u32 d = (u32)&p;
    u32 e = (u32)mBuffer;
    u32 z = (u32)mValue;
}''', headers={'include/Game/Util/Probe.hpp': 'struct Probe { void* mBuffer; f32 mValue; };'})
        casts = [f for f in findings if f['class'] == 'pointer-truncation']
        self.assertEqual(len(casts), 5, findings)
        self.assertFalse(any(f['line'] == 8 for f in casts))

    def test_implicit_storage_and_reverse_pointer_use(self):
        findings, _ = scan('''void f(void* p) {
    u32 slot;
    slot = p;
    void* q = (void*)slot;
}''')
        self.assertIn('pointer-storage32', {f['class'] for f in findings})
        self.assertIn('integer-pointer-storage', {f['class'] for f in findings})

    def test_pointer_assignments_and_chained_numeric_calls(self):
        findings, _ = scan('''struct Video {
    static Video* getManager();
    u16 getEfbHeight();
};
void f(void* p, u32* word) {
    u32* value = (u32*)p;
    word = value;
    f32 height = (s32)Video::getManager()->getEfbHeight();
}''')
        self.assertFalse([f for f in findings if f['class'] in {'pointer-truncation', 'pointer-storage32'}], findings)

    def test_complex_numeric_expressions_are_never_proven_pointers(self):
        findings, _ = scan('''void f(u32** cursor, float speed, int count, float* values) {
    u32 a = **cursor;
    s32 b = count / speed;
    s32 c = (s32)((values[0] / speed) + 1);
    s32 d = (s32)(1000.0f * speed);
    u32 e = count * sizeof(float);
}''')
        self.assertFalse([f for f in findings if f['confidence'] == 'high'], findings)

    def test_comments_literals_offsets_and_native_branches(self):
        findings, _ = scan('''// (u32)ptr
const char* text = "(u32)ptr";
struct Probe { /* 0x04 */ void* mPointer; };
void f(void* p) {
#ifdef GALAXY_NATIVE
    uintptr_t slot = (uintptr_t)p;
#else
    u32 slot = (u32)p;
#endif
}''')
        self.assertFalse(findings, findings)
        self.assertEqual(mask('a/* hi */b\n"union {}"').count('\n'), 1)

    def test_byte_punning_and_pointer_union(self):
        findings, _ = scan('''union Word { u32 word; u8 bytes[4]; };
union { struct { u8 r; u8 g; u8 b; u8 a; }; u32 rgba; } color;
struct Entry { union { s32 value; f32 real; void* pointer; }; };
void f(u8* bytes, f32 value) {
    u32 word = *(u32*)bytes;
    u32 bits = *reinterpret_cast<u32*>(&value);
}''')
        self.assertEqual(sum(f['class'] == 'endian-byte-union' for f in findings), 2)
        self.assertEqual(sum(f['class'] == 'union-pointer-alias' for f in findings), 1)
        self.assertEqual(sum(f['class'] == 'endian-raw-access' for f in findings), 1, findings)

    def test_packed_and_serialized_bitfield_groups(self):
        findings, _ = scan('''#pragma pack(push, 2)
typedef struct RFLiCharData { u16 face : 3; u16 color : 3; } RFLiCharData;
#pragma pack(pop)
''', path='libs/RVLFaceLib/include/RFLi_Types.h')
        self.assertEqual(sum(f['class'] == 'packed-layout' for f in findings), 1)
        fields = next(f for f in findings if f['class'] == 'bitfield-layout')
        self.assertEqual(fields['fields'], ['face:3', 'color:3'])
        self.assertEqual(fields['confidence'], 'high')

    def test_format_inventory_and_ownership(self):
        findings, sweep = scan('struct MsbHeader { u16 bom; s32 fileSize; };', path='libs/libms/include/libms.h')
        self.assertEqual(sweep.inventory[0]['format'], 'LMS/MSBT/MSBF')
        self.assertEqual(findings[0]['class'], 'endian-disc-record')
        self.assertEqual(format_for('libs/libms/include/libms.h', 'MsbInfo'), (None, None))
        self.assertEqual(directory_for('include/Game/Player/MarioActor.hpp'), 'Game/Player')
        self.assertEqual(lane_for('Game/Player'), 'astra')
        self.assertEqual(lane_for('Game/MapObj'), 'objects')
        self.assertEqual(directory_for('libs/JSystem/include/JSystem/JKernel/JKRArchive.hpp'), 'JSystem/JKernel')
        self.assertEqual(lane_for('JSystem/JKernel'), 'm1')

    def test_wrapped_record_retained_in_inventory_without_hazard(self):
        findings, sweep = scan('struct JMapItem { JMapBigEndian<u32> mHash; };', path='include/Game/Util/JMapInfo.hpp')
        self.assertFalse(findings)
        self.assertEqual(sweep.inventory[0]['state'], 'conversion evidence present')

    def test_legacy_branch_does_not_pollute_native_declarations(self):
        findings, _ = scan('''#ifdef GALAXY_NATIVE
void* slot;
#else
u32 slot;
#endif
void f() { u32 value = (u32)slot; }
''')
        self.assertEqual(sum(f['class'] == 'pointer-truncation' for f in findings), 1)

    def test_wrapper_and_type_arguments_are_not_casts(self):
        findings, _ = scan('''struct Record {
    JSU_BE(u32) count;
    JSU_BE_UNALIGNED(int) offset;
};
void function(u32);
void f() { int size = sizeof(u32); }
''')
        self.assertFalse([f for f in findings if f['class'] in {'cast-unresolved', 'pointer-truncation'}], findings)

    def test_nested_serialized_wrappers_and_runtime_containers(self):
        findings, sweep = scan('''struct TOffset { JSU_BE(u32) offset; };
struct TOffsetData { TOffset mOffset; };
struct THeader { TOffsetData data; };
class JASBNKParser { void* handle; };
''', path='libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp')
        self.assertFalse([f for f in findings if f['class'] == 'endian-disc-record'], findings)
        self.assertEqual({r['record'] for r in sweep.inventory}, {'TOffset', 'TOffsetData', 'THeader'})


if __name__ == '__main__':
    unittest.main()
