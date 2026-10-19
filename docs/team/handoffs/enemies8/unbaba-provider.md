# Unbaba initial-rise slope storage

`gUnbabaInitialRiseSlope` is the existing externally linked `const volatile f32` definition at `807E3080`. Correct its symbol extent from 8 to 4 bytes. Keep its address, float type, `align:4`, and the Unbaba `.sbss2` split `807E3080..807E3088` unchanged. The source input section has alignment 8; its 4-byte scalar plus alignment space reaches the existing next input at3088. No second field or storage is added.

The actual DOL instruction `80215E84:C04243E0` loads exactly 4 bytes using the startup r2 base807DECA0. A relocation-only scan of all 3486 target ELFs found this single symbol-specific runtime use, with no upper-word use or symbol-address materialization. Takobo/TakoSho neighbors exhibit the same 4-byte scalar / 8-byte aligned input pattern. Startup zeroes the whole `.sbss2` span807E2FC0..807E3188, covering both scalar and padding.

The prior size 8 belonged to the unnamed catalog entry before the current scalar source and name existed. Target ELF extent generated from that catalog is not independent original type evidence. This correction represents the supported scalar plus alignment allocation; it does not claim recovery of an original C++ declaration from stripped data. Whole-section exposure through RSO prevents an unrestricted absence-of-indirect-access theorem. Preserve the padding, neighbors, section and stack endpoints.

Evidence: `~/MacioGalaxy2-team/scratch-enemies/sol8/unbaba-provider/REPORT.md`, source/ELF symbols, complete target relocation census, actual DOL scans and BSS initializer witnesses. The associated Rise caller proof reads an arbitrary common runtime 4-byte word and never substitutes constant zero. This catalog correction does not certify unrelated provider behavior or callees.
