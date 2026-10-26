# Unsupported cutscene-skip toggle (5.4.3)

The donor state machine is present, but SMG2 WPadButton does not call
`petari_skip_pad`. `native/STUBS.md:23-24` and
`docs/port/HOOK_TRANSFER.md:131,173` defer the game feature. This change does
not implement it: the real SMG2 port remains a feature backlog item.

Only enable requests for `SkipCutscenes` are refused, with a stderr reason.
`setEnabled` owns the refusal; both file parsing and the environment publish
through it. Valid stale files still load, all other settings retain their
semantics, and save retains `SkipCutscenes=off` for compatibility. Enum IDs
and raw-tape mod IDs are unchanged. There is no SDK/game-source delta.

HBM delegates label construction and row toggles to `mods_menu.cpp`, both
using the supported registry. The dependency indices use the same row space.
A hidden enum slot cannot retarget the following visible row.

`native_supported_mods` / `petari_supported_mods_tests` checks refusal plus
its log for direct/parse/file/env paths, valid-file startup, defaults/override
and save behavior, every other mod, stable IDs, the actual Menu list, and
actual Menu toggles of PointToMove (immediately after the hidden donor) and
LevelEditor (the final compacted row). Tests use a fresh temporary settings
root and restore environment variables; they never open user caches.

Validation here is SOURCE ONLY while SOL A owns the M1. PORT performs the
native server build/test; no native pass is claimed before that result.
