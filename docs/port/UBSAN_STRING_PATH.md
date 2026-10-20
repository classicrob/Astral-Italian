# U073: extension removal without null pointer arithmetic

`MR::removeExtensionString` keeps its Wii implementation verbatim. The native
condition handles a missing slash or dot before comparing positions in the
same destination buffer or adding one to the slash pointer.

The original DOL at `8005F4E4..8005F508` compares unsigned address words,
adds one to the slash address word, and optionally writes a zero byte at the
last dot. A missing separator has address zero. The native condition preserves
those cases, including the original behavior that `.hidden` without a slash
loses its extension while `dir/.hidden` stays unchanged. Return values still
refer to the last slash in the destination buffer, or are null.

Run `python3 native/tests/string_extension_native_test.py`. It extracts the
actual production function and checks 8,294 paths/truncation capacities against
an independent 32-bit address-word oracle under ASan/UBSan. Building the same
body's original Wii branch as a native negative control reproduces U073.

The caller must supply the original function's valid writable destination and
non-null source string. This does not define behavior for invalid buffers.
Wii validation uses fresh isolated StringUtil objects and the current
`tools/team/objsame.py`; no full game or gameplay test is claimed.
