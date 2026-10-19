# FROZEN WIP — exact integer flag/CR proof support
Task: parent B; only tools/team/rdflags.py + tools/team/test_rdflags.py owned; root owns rdchecked.py/rdclass.py integration, Sol2 rdhelpers.py, Sol1 rdpool.py.
Touched/frozen files: this STATUS.md only. Neither assigned Python file exists yet; no source/header/config/tool/benign/git edits made in this task.
Done: read PROOF-THROUGHPUT.md, current decoder/value/effect DAGs and tests, Astra401 rows, Foo391 seed witness; concise API approved by lead.
API: decode_flag(line) -> ordinary use/def/kind/args/const descriptor or None; FlagAlgebra(value).evaluate(d, uses) -> tuple of interned result IDs or None; explicit flags mode only, root wires.
Design: shared, deterministic, node-bounded ROBDD exact 32-bit values; opaque producer IDs seed independent bits; canonical results intern through existing value callback; distinguish 4-bit CR fields from 32-bit GPR/XER.
First scope: numeric li/lis, masks/OR/rotate, rlwimi old-destination + seed, cntlzw, subfic; CR cror/crxor/creqv partial-field updates and Rc signed LT/GT/EQ + XER.SO. Keep all unmodified XER bits, update subfic.CA exactly.
Integration obligation: only explicitly modeled nontrapping pure ops may replace instruction-count nodes; every defined value/XER/CR must reach checked consumers/live boundaries. Do not drop seed values still used elsewhere.
Keep existing CFG/memory/FP/call effects and multiplicity; side-specific call clobbers remain independent. No CFG folding initially; joins stay opaque/checked. Symbolic relocations need ordinary decoder fallback, not numeric interpretation.
Left: implement module/tests, adversarial wrong-bit/mask/width/seed/live-CR/XER/call/join cases, positive idioms; benchmark actual Player rows with source/object/actual-DOL bindings; append only demonstrated trick to NOTES.
Tests run: none (no implementation). No new proof certificates or row exits. No defaults changed. Unsupported shapes currently all new flag semantics.
Resume: read this file + scratch-astra/PROOF-THROUGHPUT.md; inspect current tools/team/rdchecked.py (concurrent root changes expected), then implement only assigned files.
Validate: python3 -m unittest discover -s tools/team -p 'test_rd*.py'; actual inputs scratch-astra/rdscan401/rows.txt and run.log, ranked MarioFaint::start, MarioFoo::close, MarioYoshiShot::start/close.
Prior Flow CPP remains frozen for root landing; do not modify any Player source. Root must revalidate all derived proofs before separate tools landing.
