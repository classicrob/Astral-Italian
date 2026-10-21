# Built-in pad recording wire format

The scene's Dreamer playback path obtains a `.pad` resource from its Ghost archive
and passes the unchanged resource bytes to `PadDataReader`. These are Wii-format
bytes, independently of the host order used by live `KPADReadEx` samples. This
change concerns that built-in recording path; it does not interpret an external
headless harness tape format.

`PadRecorder.cpp` decodes the recorded header's five 32-bit scalar fields, the
user-file length, and each packet's packed sequence/size/sample-count word under
`GALAXY_NATIVE`. Header byte fields and opaque payloads retain their byte order.
The original Wii statements remain in the other preprocessing branch.

Delta decompression must retain its previous sample as **wire bytes**. After
reconstructing a sample, the native decoder converts only the returned sample's
scalar representations into the actual `KPADStatus` fields. It decodes the
common fields, MotionPlus fields, and the active extension selected by the
recorded device byte: freestyle, classic (including the SDK's 16–19 extension
codes), or balance board. Inactive extension bytes stay unchanged. Float and
double representations use integer loads and `memcpy`; the decoder performs no
floating-point arithmetic, reassociation or numeric conversion.

Run the focused production-TU regression, under the team's native build lock:

```
python3 native/tests/padrecorder_wire_tests.py --output /path/to/results.json
```

An optional `--baseline-source /path/to/original/PadRecorder.cpp` also compiles
and rejects the original native implementation. The fixtures are synthetic;
retail resources are neither embedded nor required. They cover all header words,
opaque header bytes, packet metadata, multiple samples per packet, byte deltas
across packets, raw history/channel independence, active/inactive extensions,
MotionPlus, signed-zero/infinity/NaN representations, save-sink byte count,
invalid header/sequence paths and restart state. The negative controls omit or
reverse individual conversions and corrupt the raw-history invariant.

The harness includes the complete production `PadRecorder.cpp` and its canonical
types. Scene registration, aligned allocation, cache maintenance and the save
sink are inert test providers. This is a recorder/wire/state regression, not a
claim to exercise scene registration, game save parsing or live gameplay.
