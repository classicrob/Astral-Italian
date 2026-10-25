# M1A list 2: untested server handoff

Base: a69824a91fed4f43405c9164d3fa87d27b065697. Published on decomp3/m1a-wip at the lead's request. M1 quiet remains active: no compiles, ninja, or nativecheck were run for this handoff. Stay parked until the lead explicitly relays "M1 done".

Eight untested candidates are in pending/; manifest.json records each rationale and target evidence. They are patch artifacts, not applied source changes. Test one at a time from the base; the two loader variants and two BellyDragon variants are alternatives. From repository root, use `git apply -p0 --check decomp3-M1A-list2-wip/pending/NAME.patch`, then `git apply -p0 decomp3-M1A-list2-wip/pending/NAME.patch` in the server test tree. All eight pass this applicability check; that is not compilation or correctness validation.

The separate measured/king_offset_add.patch uses ordinary git paths (default -p1). It contains the retained pre-quiet BattanKing partial: 84.36735 -> 89.16327%, 196-byte function, zero newly exact bytes, native validation pending. king_const_parts was staged with that partial present, but its independent table hunk also applies to the base. Test/report this distinction explicitly.

Per-unit notes and cached target/source differences are in units/. OWNERSHIP-PROPOSAL.md and global-ownership.json support 276 bytes of proposed scalar ownership, not a code-match gain. No split, symbol, configuration, or header changes are included. Common owned .data payloads do not justify new orphan strings. All baseline scores and prior failed trials relevant to these candidates are summarized in units/.

Server validation: obtain report-mode before/after scores for every function and every data section; retain only non-regressing candidates. Confirm helper/call identity with the dispatch oracle, and run nativecheck where the native DB includes the file. Full target/data and native validation remain pending. No exact-byte gains are claimed by this WIP handoff.
