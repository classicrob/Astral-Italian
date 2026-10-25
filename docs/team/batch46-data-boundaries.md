# Batch 46 data-boundary accounting

Core commit 678a7fe4 recovers 395,806 matched data bytes by correcting original allocation boundaries and ownership. Matched code remains 6,003,748 / 6,269,292 bytes (95.764366%); all 50,355 function scores are unchanged.

The total_data denominator decreases by 1,026 bytes: 1,023 bytes across 247 linker-padding tails, plus three trailing zero bytes in vi3in1. Every omitted byte was verified zero in the original DOL. These linker alignment bytes are excluded by adjacent skip splits, with the next unit starts preserved. They are not claimed as recovered source payload. Matched data becomes 979,726 / 1,701,482 bytes (57.580746%).

The original and rebuilt Wii DOL SHA1 both equal 7d79810889824edd01c4ff02624713060a69818c. Five sg.py --report samples (BckCtrl, gki_ppc, JMATrigonometric, StaffRoll, vi3in1) agree with the canonical report for all 108 functions and 21 sections. Their recompiled objects are byte-identical. The lead independently verified eight initialized payloads and five BSS allocations against the DOL/source allocations.

Only config/SB4E01/splits.txt, symbols.txt and config.yml changed among tracked configuration files. JMATrigonometric.cpp only gains the three original table alignment declarations. Compiler flags, configure.py, objdiff generation, scoring options and report settings remain unchanged. Generated unit inventory follows the corrected ownership. Strict dispatch, native syntax/link and the four-stage headless sweep pass; sweep outcomes match batch 43. OSThread's incomplete BSS proposal remains excluded.
