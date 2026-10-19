# FROZEN WIP — weak / pool-peruse

Stopped on lead request, 2026-10-19. No further edits or trials after this status.

## Task / ownership
Task C: sound opt-in per-use pool witnesses. Assigned production files: tools/team/rdpool.py and new tools/team/test_rdpool_peruse.py. Root owns rdchecked.py/rdclass.py integration and CLI --pool-uses; other owners retain rdhelpers.py/rdflags.py. No source/header/config/benign/git writes authorized for this task.

## Exact touched files
This task changed no production files. tools/team/rdpool.py remains unchanged; tools/team/test_rdpool_peruse.py does not exist. Only new handoff file: this STATUS.md. No NOTES entry: no new implemented or verified trick.
Frozen rdpool.py SHA256: 79cf017bfb12caae2d7b5aa12590eeb3ff48d2eb9e9f08c31baf3e07662c52d7.
SpinDriver.cpp and SpinDriver.hpp remain unchanged/frozen (prior report accepted): source SHA256 ca170fa68fcd883623a725cfa16fa03c98056e1576ccdd2014a47e32506ccfa3; header SHA256 6a0ea89720d4697d92c199163f2056488f95e3d1206286602d14bfb2780234e9.
Working-tree modifications belonging to other tasks were observed and not touched; root will preserve/land them.

## Done / left
Read PROOF-THROUGHPUT.md, current rdpool.py, existing test_rdpool.py, reject_derived_string_bases and rdclass integration. Inspected the stable Player pool inputs. No implementation, trial, new tests or benchmark started. No API proposal sent before stop; candidate below is UNAPPROVED design, not a completed feature.
Remaining: coordinate API, implement owned files only, adversarial tests, fixed-input before/after benchmark, root integration + independent all-lane revalidation. Do not publish proofs before integration/revalidation.

## Candidate API / integration boundary (unapproved)
Keep PoolComparator.verify(token, displacement, width=None) and all defaults unchanged. Add a separate opt-in per-use verifier/session in rdpool.py, with explicit load-width, complete terminated-string, and bounded-record use operations. Root's CFG guard must defer pointer formation until actual use, account for EVERY tainted use on all paths, and reject unknown consumers, returned pointers, stores/spills/escapes, unknown indexes and mutable/unknown storage. A successful byte span alone is never a complete-function certificate.
Each operation should bind unique target/source origins, exact source/object bounds, target DOL bytes and canonical ADDR32 relocation identity; evidence records exact span and hashes. Pointer loads may return a fresh origin token only after verifying the complete pointer word and canonical referent. Pointee extents must be checked at every actual use; no arbitrary tail or first-string shortcut. Constant index spans require complete bounded endpoints. Session closure must reject unresolved provenance/uses.
Needs agreement before implementation: precise immutable-storage evidence (ELF .data alone cannot distinguish immutable compiler strings/records from mutable arrays), trusted consumer summaries, record extent derivation and completeness protocol. Do not invent caller-supplied waiver flags or hand-approved address ranges. Reject unsupported cases conservatively.

## Current limitations / concrete findings
Existing verify falls back from missing object extent to entire section equality, causing unrelated tail rejects. Existing guard calls verify at addi pointer formation and does not prove full call-consumer extents or reject every return/escape. A new byte-range comparator alone cannot fix this; root's coordinated guard integration is required. Current PoolELF records section types/names, object extents and relocations, but no section flags/full symbol metadata. Existing test fixtures omit flags, so compatible defaults matter.
Stable inputs: ~/MacioGalaxy2-team/scratch-astra/rdscan401/{rows.txt,run.log,REVIEW.md,proposed.txt.pool.json}; 33 pool rejects among 80 tail rows. Full stable Player report: ~/MacioGalaxy2-team/trial/REALDIFF-astra400stable.md. Base helper landing reported by lead: 5c45a2ba. No bound per-use evidence or before/after row benchmark produced yet.
Required negative tests: wrong later string, width, record endpoint, pointer bytes/relocation identity; aliased/mutable/indexed/return/spill provenance; preserve default test behavior.

## Tests / exits
No new tests run for Task C; there is no implementation to validate. Read-only status/hash inspections exited 0. First combined inspection exited 1 because this new evidence directory did not yet exist; then created for handoff only. Prior SpinDriver final cc.py was OK; strict 37/61 -> 39/61 and 1932/8744 -> 2140/8744 bytes, with all 89 source allocated bodies/data/ref identities unchanged. Its default rdclass remains 0/2; see ../spindriver/REPORT.md and individual-proofs.md for bounded manual evidence and limitations.

## Resume commands (successor M1; adapt HOME/worktree path)
cd ~/MacioGalaxy2-wt/enemies
cat ~/MacioGalaxy2-team/scratch-astra/PROOF-THROUGHPUT.md
cat ~/MacioGalaxy2-team/scratch-enemies/sol7/pool-peruse/STATUS.md
sed -n '1,340p' tools/team/rdpool.py
sed -n '423,610p' tools/team/rdchecked.py
cat tools/team/test_rdpool.py
cat ~/MacioGalaxy2-team/scratch-astra/rdscan401/REVIEW.md
python3 -m unittest discover -s tools/team -p 'test_rd*.py'

After root agrees to API, implement rdpool.py + test_rdpool_peruse.py only. Root wires separate --pool-uses; do not edit root-owned integration files. Benchmark fixed source/object-bound inputs, not drifting live Player objects. No source/header/config/benign/git edits, and no new feature work on this server after freeze.
