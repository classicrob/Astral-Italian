# Super Mario Galaxy 2 native controls

The keyboard and mouse defaults match MacioGalaxy. SMG2's power-ups reuse the
same Wii buttons and gestures; they do not get separate jump, spin, or tongue
keys. The input layer sends WPAD reports through the SDK's KPAD processing.
These mappings describe the input path, not a claim that every action has
been exercised in a running SMG2 stage.

## Mario, menus, and the Star Pointer

The last column is the action name used by `controls.txt` and the Controls
page. A game action can share a Wii button with other actions.

| Action | Wii input | Default keyboard / mouse | Remappable name |
| --- | --- | --- | --- |
| Move / steer / navigate | Nunchuk stick | WASD | `StickUp`, `StickLeft`, `StickDown`, `StickRight` |
| Jump / confirm / advance text | A | Space | `A` |
| Start; confirm elsewhere | A+B at the title prompt; A elsewhere | Return / keypad Enter | `Start` |
| Spin / shake the Remote | Remote acceleration | F | `Shake` |
| Shake the Nunchuk separately | Nunchuk acceleration | H | `NunchukShake` |
| Crouch; ground pound in the air | Nunchuk Z | Left or Right Shift | `NunchukZ` |
| Move Star Pointer; collect Star Bits | Remote IR pointing | Mouse motion | Pointer motion is positional, not a button binding |
| Shoot Star Bits; B in menus | B | Left mouse / B / Backspace | `B` |
| Hold a pointer target / Pull Star | Point and hold A | Hold right mouse (or Space) | `A` |
| Rotate camera | Remote D-pad left / right | Q / E; Left / Right arrows | `DpadLeft`, `DpadRight` |
| Recenter camera | Nunchuk C | C | `NunchukC` |
| First-person view; leave it | D-pad up; down (or A) | Up; Down (or Space) | `DpadUp`, `DpadDown`, `A` |
| Pause / resume | + or − | Escape / Minus | `Plus`, `Minus` |
| Back / cancel | B | Backspace | `B` |
| Native Home menu / remapping | HOME | F1 or grave (`) | `Home` |
| Remote 1 / 2 | 1 / 2 | 1 / 2 | `One`, `Two` |
| Walk slowly | Partial stick | Hold Left Alt + WASD | `Walk` |
| Manually tilt the Remote | Remote orientation | Hold Tab + WASD | `TiltHold`, movement actions |
| Manually toggle upright posture | Remote orientation | T | `PostureToggle` |

A right mouse press is also A: away from an interactive target it can jump.
Likewise B and Backspace are real B inputs, not menu-only shortcuts.
Return adds B only while the title's A+B prompt is reported by the game;
it stays plain A elsewhere so that it does not cancel a menu selection.

## SMG2 actions and power-ups

Use the power-up or item first. The original game still decides when a
button is allowed, whether a target is valid, and whether energy or charges
remain. A press does not bypass those checks.

| Action | Wii input | Keyboard / mouse | Remappable name |
| --- | --- | --- | --- |
| Mount Yoshi / Yoshi spin action | Shake Remote or Nunchuk | F (or H) | `Shake`, `NunchukShake` |
| Aim Yoshi's tongue | Point at the target | Mouse motion | Pointer position |
| Extend tongue / eat / grab; spit a held target | B, with pointing when required | Left mouse or B | `B` |
| Keep a tongue grab | Hold B where the target permits it | Hold left mouse or B | `B` |
| Yoshi jump / flutter | A; hold A for flutter | Space; hold Space | `A` |
| Dismount Yoshi | Z+A | Shift+Space | `NunchukZ` + `A` |
| Balloon Yoshi: brake flight | Hold A | Hold Space | `A` |
| Spin Drill: start drilling while carrying | Shake (the player's rush/spin request) | F (or H) | `Shake`, `NunchukShake` |
| Spin Drill: surface / finish drilling | Item state and collision, not a separate button | Automatic emergence | None |
| Cloud Mario: spin / create a cloud when allowed | Shake | F (or H) | `Shake`, `NunchukShake` |
| Rock Mario: enter roll / spin | Shake | F (or H) | `Shake`, `NunchukShake` |
| Rock Mario: steer / jump | Stick / A | WASD / Space | Movement actions / `A` |
| Rock Mario: end roll during its jump state | Z | Shift | `NunchukZ` |
| Bee Mario: fly | Hold A | Hold Space | `A` |
| Bee Mario: steer / spin | Stick / shake | WASD / F (or H) | Movement actions / `Shake`, `NunchukShake` |
| Boo Mario: rise / move | A / stick | Space / WASD | `A` / movement actions |
| Boo Mario: turn transparent | Shake | F (or H) | `Shake`, `NunchukShake` |
| Spring Mario: high bounce / ground pound | A at the bounce / Z | Space / Shift | `A`, `NunchukZ` |
| Fire Mario: fireball | Shake | F (or H) | `Shake`, `NunchukShake` |
| Star Ball | Hold Remote upright and tilt; A jumps | WASD tilt while the ride hint is active; Space jumps | Movement actions, `A` |
| Vines, plants, Launch Stars, other shake prompts | Remote or Nunchuk shake, as read by the object | F (or H) | `Shake`, `NunchukShake` |

The reconstructed Spin Drill code receives `ItemRushBegin` and `RushCancel`
messages. The start uses the shared player rush input; normal emergence
runs at the end of the drill animation. A separate manual drill-cancel key
is **not established by the current source trace**, so none is invented.
Boo's state code is present, but this table does not assert that the form is
available in an ordinary SMG2 stage.

The main spin configuration accepts either device's shake, but the two
accelerometers remain separate. F keeps its existing Remote flick. H emits
a Nunchuk flick and can satisfy a reader of `isSubPadSwing(0)` specifically.
Holding either key is one flick, not automatic repeat. A pending Nunchuk
flick waits after C/Z; a pending Remote flick waits after A/B, matching
Mario's respective button lockouts. Focus loss cancels waiting flicks.

## Co-star Luma (player 2)

F2 connects or disconnects a second virtual Wii Remote (WPAD channel 1).
It starts disconnected, so the original single-player setup is unchanged.
Allow about 200 ms for its connection before using it. The two pointers
share the mouse position; this is a single-mouse assist mode, not two
independent cursors. Player 1's mouse buttons keep their original actions.

| Co-star action | Wii input | Default | Remappable name |
| --- | --- | --- | --- |
| Enable / disable the second Remote | Connect / disconnect Remote 2 | F2 | `CoStarToggle` |
| Aim / collect pointer items | Remote 2 IR pointing | Mouse motion | Shared pointer position |
| Grab / hold / assist on a valid target | Remote 2 A | U (hold where required) | `CoStarA` |
| Shoot Star Bits / B on Remote 2 | Remote 2 B | Y | `CoStarB` |
| Shake / spin a grabbed target when supported | Remote 2 acceleration | N | `CoStarShake` |

These are channel 1 reports, never extra channel 0 A/B presses. Actual
co-star interactions remain gated by the original game's target and state
checks. Enablement is session state, not saved in `controls.txt`; its keys
and buttons are saved. Co-star keys have no controller defaults but can be
assigned on the same Controls page.

## Starship Mario and world-map navigation

| Action | Wii input | Default | Remappable name |
| --- | --- | --- | --- |
| Walk on Starship Mario / select directions on the map | Nunchuk stick | WASD | Movement actions |
| Talk / confirm / travel to a selected destination | A | Space or Return; right mouse when pointing | `A`, `Start` |
| Point at a map destination / button | IR pointing | Mouse motion | Pointer position |
| Confirm a pointed destination | A | Space / right mouse | `A` |
| Up as a world-map decide shortcut | D-pad up | Up arrow | `DpadUp` |
| Back / cancel; zoom controls that present a B prompt | B | Backspace / B / left mouse | `B` |
| Down as a map zoom shortcut where offered | D-pad down | Down arrow | `DpadDown` |

In particular, the map's A path uses the same buttons as gameplay. Return
must not send A+B on the world map. Camera and movement restrictions in a
map event or cutscene are still the game's restrictions.

## Changing controls

Open **F1 → Controls**, select a slot, then press the desired key, mouse
button, or controller button. Each action has two keyboard/mouse slots and
two controller slots. Conflicts offer Swap, Keep both, or Cancel. Escape
cancels capture; Reset action and Reset all restore defaults. The menu
protects its last keyboard/mouse binding, so it remains reachable.

Changes apply immediately and are saved to `controls.txt` in the app's
user directory (normally `~/Library/Application Support/Petari`, overridden
by `--user`). A partial file preserves defaults for omitted actions. For
example, these remap only the new controls:

```text
NunchukShake=Key:H
CoStarToggle=Key:F2
CoStarA=Key:U,Mouse:X1
CoStarB=Key:Y,Mouse:X2
CoStarShake=Key:N
```

The names in the tables are the exact serialized names. Pointer position
itself is not a remappable button. An invalid file is reported and ignored;
the first successful save preserves it as `controls.txt.bad`.

## Controllers and optional native controls

Existing gamepad defaults are unchanged: left stick moves, right stick
points (R3 recenters it), bottom=A, right or RT=B, left or RB=Shake,
LT=Z, LB=C, D-pad=camera, top=Start, Start=Plus, Back=Minus, Guide=Home.
Buttons can be remapped using their layout-neutral names such as
`Pad:West` and `Pad:RightShoulder`. No existing gamepad input is reassigned
to one of the new actions.

Imported optional controls keep their defaults: G/V collect/shoot mods,
R cap throw, J/L and I/K orbit/pitch, Z/X zoom, middle mouse or Command
orbit drag, P photo mode, O screenshot, and a short Tab tap for the editor.
They act only when their corresponding native feature is enabled; see
[MODS.md](MODS.md). Their presence is not a claim that each imported mod
has been validated in SMG2. Do not repurpose these keys for a power-up.

## Input limits and source trace

Focus loss releases held controls and hides both pointers. macOS shortcuts
using Command stay with macOS. Tilt hides the pointer when the virtual
Remote is upright, as on Wii. The imported distance-to-screen camera zoom
(`CameraDPD`, Z plus moving the Remote in depth) has no keyboard equivalent.
The Star Ball hint drives tilt; the inherited Ray steering mode is retained
for compatibility but is not presented as an SMG2 ride.

The code trace for the action tables is recorded in
[input/SMG2_ACTIONS.md](input/SMG2_ACTIONS.md), including gaps in the
reconstructed source. Native input tests cover reports, remapping, and
channel separation; they are not a full gameplay or visual test.
