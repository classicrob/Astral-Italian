# Headless all-galaxy health sweep

Runs every galaxy and scenario on the disc through `petari_headless` (null GPU, virtual clock, fresh isolated user),
enters the stage exactly as `--stage/--scenario` does, waits for the game's own play-ready seam, then drives Mario
with a scripted keyboard through the real keyboard -> WPAD -> KPAD path and records, per loop, whether the game stays
healthy. It needs the extracted disc (`~/MacioGalaxy2-gamedata/SB4E01-extracted`, `nodtool extract SB4E01.iso`) and a
`petari_headless` (`sh tools/team/headlesslink.sh`, or an ASan+UBSan build with `-DPETARI_SANITIZERS=ON`).

```sh
python3 native/tools/headless_sweep.py run --headless <petari_headless> --out <new dir outside the checkout> --jobs 4
python3 native/tools/headless_sweep.py run ... --stages GoroRockGalaxy --scenarios 1-3     # a subset
python3 native/tools/headless_sweep.py run ... --script random:7 --frames 5400             # seeded random player
python3 native/tools/headless_sweep.py run ... --player-mode bee --powerup-map powerup_map.json   # power-up
python3 native/tools/headless_sweep.py report --out <dir>      # re-classify logs with the current parser
python3 native/tools/powerup_map.py --json powerup_map.json    # which stages load which power-up's resource
python3 native/tools/sweep_sanitizers.py <dir>                 # group ASan/UBSan sites of a sweep
python3 native/tools/headless_determinism.py --headless A [--headless-b B] --out <dir> --scenarios 1   # run-to-run / two-binary compare
```

A run takes about 12 s (Release) or 30-60 s (ASan). Output per run: `runs/<Stage>-s<n>/{app.log,result.json}`, plus
`results.json`, `results.csv`, `table.md`.

## What a run does and checks

- Script: idle, walk, jump, spin (F), ground pound, camera Q/E/C, repeated for 3600 loops (60 s) by default;
  `random:<seed>` is a seeded random player (walk/jump/spin/pound/camera/pause); `pointer:<seed>` pokes the Star
  Pointer (`ptr x y`, `MouseLeft`) for galaxy-map scenes. A player A-tap runs while an opening demo is active.
- Every loop from play-ready on is a trace row (86 columns, `--trace`): Mario valid / finite / dead / state ids /
  animation / mode, rng seed, actor census counts.
- Outcomes: `PASS`, `PASS_NOTE` (passed; notes: Mario fell/died and the stage restarted, late loops, unexpected
  host-allocation lines), `NON_GAMEPLAY` (WorldMap01-07 galaxy-map UI scenes: no Mario, ran without a crash), `FAIL(class)`:
  `CRASH(sig)`, `ABORT`, `HARNESS`, `NOT_READY` (play-ready never reached in 60000 loops, with the last phase),
  `MARIO_INVALID` / `MARIO_NONFINITE` / `MARIO_OUT_OF_WORLD`, `STUCK` (no movement under input), `HEAP`, `TIMEOUT`,
  `MODE_NOT_APPLIED`, `INVALID_WORLDMAP_REQUEST` (a stage `WorldMapNNGalaxy` outside 01..08).
- Late loops: the wall time between loops is logged (`[sweep-stall]`, cause `resource-load` when disc reads took
  >= 1 ms in the loop, else `game-work`; the GX app's pipeline/texture causes do not exist without a GPU). Timing
  under concurrent load is not performance evidence.
- Power-ups: `--player-mode bee|fire|hopper|teresa|ice|foo|rock|cloud|yoshi|invincible|lifeup` calls the game's own
  `MR::changePlayerMode*` once at script start. The trace's `mode` column must show it (`MODE_NOT_APPLIED` otherwise).
  A mode is applied only where the scenario places an object that loads its resource (`powerup_map.py`; applying it
  elsewhere is not possible in the game).
- Boot routes: `--boot` starts the script at the first loop (title / file select / world map from the unlocked-save
  copy with `--existing-unlocked-user`); directives `wait <field>=<value> [timeout=N]`, `viewport W H`, `ptr X Y`
  and the keys `MouseLeft`/`MouseRight` drive a player-style path. `[sweep-scene]` log lines show every scene change.

## Results (main at the time of writing: 2026-10-23, Release build)

260 scenarios over 61 galaxies: 252 pass (7 clean, 245 with notes), 7 NON_GAMEPLAY (WorldMap01-07), 1 FAIL
(WorldMap08Galaxy: the disc has no GalaxyName message for it; its name exists only in unmounted debug archives;
`GrandGalaxyMapParts` builds it as aggregate data; unreachable in normal play, so not a game defect).
`n` = passed with notes (134 scenarios: Mario fell or died on the scripted route and the stage restarted).

| galaxy | s1 | s2 | s3 | s4 | s5 | s6 |
|---|---|---|---|---|---|---|
| AbekobeGalaxy | n | n | n | n |  |  |
| AncientExcavationGalaxy | n | n | n | n | n | n |
| BigGalaxy | n | n | n | n | n | n |
| BigTree2Galaxy | n | n | n | n | n | n |
| BigWaterFallGalaxy | n | n | n | n | n | n |
| ChallengeGliderGalaxy | n | n | n | n |  |  |
| ChaosGravityGalaxy | n | n | n | n |  |  |
| ChildRoomGalaxy | . |  |  |  |  |  |
| CloudGardenGalaxy | n | n | n | n | n | n |
| DigMineGalaxy | n | n | n | . | n | n |
| DimensionBigCastleGalaxy | n | n |  |  |  |  |
| FlowerHighlandGalaxy | n | n | n | n |  |  |
| GhostConveyorGalaxy | n | n | n | n | n | n |
| GoroRockGalaxy | n | n | n | n | n | n |
| HomeCenterGalaxy | n | n | n | n | n | n |
| HoneyBeeVillageGalaxy | n | n | n | n |  |  |
| IslandFleetGalaxy | n | n | n | n | n | n |
| JungleGliderGalaxy | n | n | n | n |  |  |
| KachikochiLavaGalaxy | n | n | n | n | n | n |
| KingOfKingsGalaxy | n | n | n | n |  |  |
| KoopaJrLv2Galaxy | n | n | n | n |  |  |
| LongForCastleGalaxy | n | n | n | n | n | n |
| MadnessOnimasuGalaxy | n | n | n | n |  |  |
| MagicGalaxy | n | n | n | n | . | . |
| MarioFaceShipGalaxy | n | n | n | n |  |  |
| MarioOnMarioGalaxy | n | n | n | n |  |  |
| MemoryRoadGalaxy | n | n | n | n |  |  |
| MokumokuValleyGalaxy | n | n | n | . | n | n |
| MysteryCandyGalaxy | n | n | n | n |  |  |
| OmoteuLandGalaxy | n | n | n | n | n | n |
| PeachCastleGalaxy | n | n | n |  |  |  |
| QuicksandGalaxy | n | n | n | n | n | n |
| RedBlueExGalaxy | n | n | n | n |  |  |
| SecretAthleticGalaxy | n | n | n | n |  |  |
| StaffRollGalaxy | n | . |  |  |  |  |
| StarCreekGalaxy | n | n | n | n | n | n |
| SwitchMoveBlockGalaxy | n | n | n | n |  |  |
| TamakoroPlanetGalaxy | n | n | n | n |  |  |
| TamakoroSliderGalaxy | n | n | n | n |  |  |
| TeresaLabyrinthGalaxy | n | n | n | n | n | n |
| ThunderFleetGalaxy | n | n | n | n | n | n |
| TimerSwitchingPlatformGalaxy | n | n | n | n |  |  |
| TropicalResortGalaxy | n | n | n | n | n | n |
| TwisterTowerGalaxy | n | n | n | n | n | n |
| UnderGroundDangeonGalaxy | n | n | n | . |  |  |
| VsKoopaGravityGalaxy | n | n | n | n |  |  |
| VsKoopaJrLv1Galaxy | n | n | n | n |  |  |
| VsKoopaJrLv3Galaxy | n | n | n | n |  |  |
| VsKoopaLv1Galaxy | n | n | n | n |  |  |
| VsKoopaLv3Galaxy | n | n | n | n |  |  |
| WanwanFactoryGalaxy | n | n | n | n | n | n |
| WhiteSnowGalaxy | n | n | n | n | n | n |
| WorldMap01Galaxy | g |  |  |  |  |  |
| WorldMap02Galaxy | g |  |  |  |  |  |
| WorldMap03Galaxy | g |  |  |  |  |  |
| WorldMap04Galaxy | g |  |  |  |  |  |
| WorldMap05Galaxy | g |  |  |  |  |  |
| WorldMap06Galaxy | g |  |  |  |  |  |
| WorldMap07Galaxy | g |  |  |  |  |  |
| WorldMap08Galaxy | X CRASH(SIGSEGV) |  |  |  |  |  |
| YosshiHomeGalaxy | n | n | n | n | n | n |

Other sweeps on the same build family: random-input fuzz (seeds 1, 11, 12; 5400 loops) and the eight power-up modes
over every stage that places them (bee 2, fire 7, hopper 1, invincible 5, rock 5, teresa 1, cloud 15, yoshi 23
scenarios) pass under ASan+UBSan with no new sanitizer site. Ice and Foo Mario have no placed MorphItem on this disc.

## Defects found by the sweeps (all fixed at the source, GALAXY_NATIVE-guarded, Wii objects unchanged)

| bucket | scenarios | fix |
|---|---|---|
| `Meramera::endRunaway` invalidates a sensor ("break") the actor does not have; the Wii runs `HitSensor::invalidate` on null this | VsKoopaLv1 x4 (crash) | `MR::invalidateHitSensor(actor, name)` null guard |
| `PlantGroup::placeOnRail` accumulates into an uninitialised caller vector -> stack garbage -> NaN in angle tables | 38 | native zero-init; JMA trig table narrowing follows fctiwz |
| flags read before initialisation: `FootPrint` mode byte, `ItemBlock::mNoCalcAnim`, `Mario::clear2DStick` | 48 / 23 / 14 | native initialisation |
| `MarioActor::fn_803CAA00`: `vecBlendSphere` leaves its output untouched for opposite vectors; caller local uninitialised | Abekobe x4, ChaosGravity x4 (wrong trajectory) | seed the local from the head vector |
| `CollisionArea::hitCheck` shift by -1; `MeisterTalkHolder::nextTalk` divide by zero; `JASChannel` pitch > 65535 | 1 / 1 / 3 | Wii `slw` / `divw` / `fctiwz`+low16 semantics |
| DSP renderer read the live reverb block while a game thread rewrote it (modulo by zero) | KoopaJrLv2 s3 (random input) | read the block once per pass |
| `Tamakoro::updateMoment`: pointer to a block-local vector read after the block (stack-use-after-scope) | TamakoroSlider s2 (random input) | hoist the vector |
| `CustomTagAlphaCtrl::update`: end frame = +inf float->int (fade control disabled) | ChallengeGlider (random input) | Wii `fctiwz` saturation, wrapping add |

Probes that found these: ASan+UBSan sweeps (`PETARI_SANITIZERS=ON`), a Release build with
`-ftrivial-auto-var-init=pattern` against one with `=zero` compared trace-for-trace (uninitialised locals), and the
run-to-run / `MallocScribble` comparison. Known non-game differences: the trace `scene_epoch` offset (observer
metadata), controller report phase after variable async boot (TamakoroSlider tilt), and music-beat phase
(TimerSwitchingPlatform, OmoteuLand s3).

## Not reachable headlessly (yet)

- Spin Drill (a rush item picked up from a placed object), Yoshi fruit/tongue eating and the Star Pointer
  interactions need navigation to the object or pointer scripting per stage; only the player modes themselves run.
- Prankster / Speedy / Purple / Daredevil / Cosmic comets: the comet scenarios are in the 260 (Purple 9, Red 11,
  Mimic 4, Dark 6, Exterminate 4, Quick 2), entered by scenario number; the comet's own mission rules were not checked.
- Rendering, shaders, audio output, real controllers: out of scope for this harness (see HARNESS.md).
- Stuck nerves: no generic detector; a stuck Mario shows as `STUCK`, a demo that never ends as `NOT_READY`.
