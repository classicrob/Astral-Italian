# MacioGalaxy2 team playbook

Private fork of SMGCommunity/Garigari (SMG2 decomp, SB4E01). Goal: decompile SMG2 fast, reusing
Petari (SMG1), so it can later be ported natively the way MacioGalaxy ports SMG1.

**Never contact upstream** (no PRs, issues, or Discord posts to Garigari/Petari). Their AI policy
covers their repo; this fork is ours and stays private.

## Why we match bytes

The native port only needs correct code, but objdiff is our test oracle: a 100% match proves the
function is right with no playtesting. So aim for a match. If a function stalls (about 3 honest
attempts), you may leave it if **the only remaining differences are register allocation,
instruction order, or stack offsets**. Different calls, constants, branches, or field offsets
mean the code is wrong, and that is never acceptable. Mark the function `// NONMATCHING: <why>`
and move on. Breadth beats perfection.

## Tools (run from your worktree root)

- `python3 tools/team/sg.py units Game/MapObj` ranks units by unmatched bytes that have a Petari counterpart.
- `python3 tools/team/sg.py unit Game/MapObj/Rock` builds one unit and lists each function's match %,
  size, Petari size (`SAME` = identical size, usually a near-verbatim port) and Petari unit.
- `python3 tools/team/sg.py diff Game/MapObj/Rock init__4Rock` shows the target vs our instructions
  (mismatches only, with context).
- `python3 tools/team/sg.py petari <symbol>` finds the function in Petari.
- Pristine Petari source: `~/MacioGalaxy2-team/Petari` (use this, not ~/MacioGalaxy, which has port hooks).
- `ninja build/SB4E01/src/Game/MapObj/Rock.o` compiles one unit (sg.py does this for you).
- All Game units are already registered in `configure.py` as `NonMatching`. Creating `src/<unit>.cpp`
  is enough to start; don't add or reorder entries. When a unit fully matches (code **and** data),
  flip its line to `Matching`. `ninja` then verifies the whole DOL hash.

## Compiler

SMG2 game code uses `mwcc Wii/1.0` (Petari: `GC/3.0a3`). Petari code often needs small changes to
match: inlining, ordering, and new SMG2 fields. **Whatever trick works, write it down in the shared
notes right away** (below), so other lanes don't rediscover it.

## Shared live files (outside git, visible to every worktree)

- `~/MacioGalaxy2-team/NOTES.md`: matching knowledge. Append only (`cat >> ... <<'EOF'`), short entries:
  `- [lane] <pattern>: <fix>`. Read it at the start of each work session and when stuck.
- `~/MacioGalaxy2-team/HEADERS.md`: append a line whenever you change a shared class layout
  (LiveActor, NameObj, Util, MR:: helpers, etc.): `- [lane] Class: what changed, commit`.
- `~/MacioGalaxy2-team/BOARD.md`: lane assignments and priorities, maintained by the team lead.

## Lanes and ownership

Each lane works in its own worktree (`tools/team/new-worktree.sh <lane>`, `~/MacioGalaxy2-wt/<lane>`)
and only creates or edits `src/` files in its own directories. Shared headers may be edited by anyone.
Keep those edits minimal and additive, and land them quickly. The core lane has the final say on core
class layouts. Within a lane, the lead gives each subagent disjoint units (never two agents on one file).

## Landing

Commit small and often, then `tools/team/land.sh` from your worktree: it rebases on main, builds
(DOL hash check), fast-forwards main under a lock and pushes. Land at least every ~30 to 45 minutes,
and immediately after any shared-header change. Commit messages: `<Unit>: <what> (N/M fns matched)`.

## No redundant review

A byte match is its own proof: don't re-review matched work. Only review NONMATCHING functions when
they are suspicious (a logic diff, not register allocation).

## Disk

Never let free space on / drop below 40 GB (`df -h /`). A worktree with its build takes about 0.5 GB.
