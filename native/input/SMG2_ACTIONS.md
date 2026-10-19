# SMG2 input source trace

This is a source audit for the native keyboard mapping, based on the game
code at `44cdbe23` (P1 base `3c133d77`). It does not certify every gameplay
path against the DOL, nor assert that every reconstructed state is complete.
No `src/`, `include/Game`, or SDK file is changed by the controls work.

Paths below are relative to the repository root. Search the named definition
rather than relying on line numbers after integration.

| Input / action | Concrete source path and predicate |
| --- | --- |
| Shared jump, spin, carry/throw/rush | `src/Game/Player/MarioInputConfig.cpp`: `_2=2`, `mSpinButton=2`, `_6=3`. `MarioActor::checkButtonType` in `MarioActor.cpp`: case 2 is `_F0C || _F00` (Nunchuk / Remote swing edges), case 3 A trigger, case 5 A hold, case 7 Z trigger, case 9 Z hold. `isRequestRush`, `isRequestThrow`, `isRequestSpin`, `isRequestJump` retain their existing state guards. |
| Separate motion channels | `MarioActor::updateControllerSwing` reads `MR::isCorePadSwing(0)` (trigger variant when swimming), A/B starts a ten-frame lockout. `updateControllerSwingLeft` reads `MR::isSubPadSwing(0)`, C/Z starts its ten-frame lockout. `src/Game/Util/GamePadUtil.cpp::isPadSwing` ORs the two readers. |
| Gesture thresholds / units | `src/Game/System/WPad.cpp` constructs separate acceleration/swing readers and assigns `mSubPadSwing->mDistanceSwingThreshold=2.0f`. `WPadHVSwing::updateSwing` compares acceleration against 20 reports earlier. `WPadAcceleration` selects KPAD's core or `ex_status.fs.acc` stream and sets sensitivity 0.15. `src/RVL_SDK/kpad/KPAD.c` clamps each Nunchuk axis to 2.1 g. Native WPAD encodes the new diagonal impulse with the existing Nunchuk 200-count/g conversion; the Remote waveform is unchanged. |
| Yoshi tongue / eat / spit | `src/Game/Player/YoshiTongueAction.cpp::fn_8041DFD0` requests command-stream pointing and reads `testCorePadTriggerB(0)`, takes lock-on targets and enters tongue states. `fn_8041E210` and `fn_8041E470` cover tongue/grab progression; the former and another target state read B hold. Auto-eat also exists (`tryStartNearestAutoEat`): not every eat needs a B press. |
| Yoshi flutter | `src/Game/Player/MarioSMG2_B.inc::fn_803B12B0` runs the `踏ん張り` animation and flutter velocity update; it calls `fn_803B14E0` when the timer predicate fails or `testCorePadButtonA(0)` is false. Holding A is required. |
| Yoshi dismount | `MarioSMG2_B.inc::fn_803B0E60` requires Yoshi mode, a Yoshi parameter/availability predicate, `checkTrgA()` and `checkLvlZ()` before `fn_803B0EF0`. This binds Z hold + A trigger, not a new button. |
| Balloon Yoshi | `src/Game/Player/MarioYoshiBalloon.cpp` reads `checkLvlA()` to select/maintain state 3, named `YoshiBalloonBrake`, and returns to the jet state when released. |
| Spin Drill start | `MarioActor.cpp::tryThrow` requires `isRequestThrow()` (default shared shake), then calls `fn_803D0F60` on the carried sensor. `MarioActorSMG2_B.inc::fn_803D0F60` sends message 0x94; `src/Game/Util/ActorSensorUtil.cpp::isMsgItemRushBegin` identifies 0x94. `src/Game/MapObj/ItemDrill.cpp::receiveOtherMsg` handles it by calling `startDrill()`. |
| Spin Drill exit / cancel limit | `ItemDrill::exeDrillDiveEnd` ends the bind and enters Taken after its action ends. `receiveOtherMsg` handles RushCancel (0x95) by ending the bind and returning Taken. Callers of 0x95 include forced rush termination and rush transfer. This audit did not establish a player-button-to-manual-cancel path; no key is assigned on that speculation. |
| Cloud creation | `src/Game/MapObj/SpinCloudBlock.cpp::SpinCloudBlockGroup::movement` checks `mRespawnStep==0 && MR::isPlayerSwingActionStartNow()` and starts transforming the first available cloud. `SwingActionWatcher.cpp::movement` makes a rising edge from `MR::isPlayerSwingAction()`, through `PlayerUtil.cpp` to `MarioAccess.cpp::isSwingAction` (actual player spin state, not a new button). |
| Rock roll / steer / jump / stop | `src/Game/Ride/RollingRockStateParent.cpp::receiveOtherMsg` handles RushBegin by `tryStartRoll` (requires Rock mode); the player's default request is shared shake. `RollingRock::calc...` helpers read the Nunchuk stick. `RollingRockStateRoll.cpp::tryJump` reads `testCorePadTriggerA(0)`; `exeJump` buffers A; `tryEnd` requires its Jump nerve and `testSubPadTriggerZ(0)`. `tryPadSwing` uses `isPadSwing(0)` for an in-air acceleration state. Z is not documented as an unconditional stop in every roll state. |
| Bee flight | `src/Game/Player/Mario.cpp::beeMarioOnAir` requires `checkLvlA()`, remaining flight time, and the inhibit-time gate before applying flight behavior. Direction comes from the existing world pad direction. |
| Boo | `src/Game/Player/MarioTeresa.cpp::checkAccel` uses A trigger; directional update uses `getWorldPadDir`. `MarioActor.cpp::updateSwingAction`, selected action 4, calls `startTeresaDisappear`. The selected action table is resource-dependent; presence of this state is not proof that a normal SMG2 stage offers Boo Mario. |
| Spring | `src/Game/Player/MarioRabbit.cpp::update` tests `mActor->isRequestJump()` for high-bounce animation in the landing window; it also calls `fn_803BDE10`, whose input-config `_0=7` maps to Z trigger. Existing timing/state gates are retained. |
| Fire | `MarioActor.cpp::updateSwingAction` requires the shared rush/spin request. Selected action 2 calls `updateFireSpin` when spin is enabled and the player is not swimming. `MarioActorSMG2_B.inc::updateFireSpin` sets the fireball delay and throw animation. No extra Fire-specific key. |
| Co-star connection | `src/Game/MapObj/SupportPlayDirector.cpp::movement` reads `isConnectedWPad(1)` outside its pad-mode gate; connection alone does not override the game's mode checks. |
| Co-star grab / spin | `src/Game/NPC/SupportTicoStateFunction.cpp::isPressA`, `isTriggerA`, `isSwing` gate on `isEnable()` and read A hold, A trigger, and `isPadSwing(1)`. `SupportTico.cpp::tryAttachToTarget` checks `isStarPointerPointing2POnPressButton`, target validity, forbidden area and group capacity. `StarPointerUtil.cpp` routes these pointer helpers to channel 1 plus A. |
| Co-star B / shooting | `src/Game/MapObj/StarPieceDirector.cpp` reads B trigger from `mPointerPort`, with pointer validity, item count, and game-state guards; `StarPointerLayout.cpp` carries a per-channel pointer. Channel 1 B is not sent as channel 0 B. |
| Co-star assist jump | `MarioActor.cpp::isRequestJump2P` and `isRequestSpinJump2P` read valid 2P pointing plus A trigger on channel 1; `isKeepJump2P` reads A hold on channel 1. |
| World map | `src/Game/System/WorldMapResultTU11.cpp::WorldMapOwner::isDecideTrigger` reads D-pad up or A trigger; `tryDecide` and destination selection read A; `isStickTilted` reads player stick X/Y. `WorldMapResultTU22.cpp` reads D-pad down in its zoom path. `WorldMapResultTU18.cpp` has an A-or-B path. These do not require another keyboard scheme. Starship's on-foot Mario uses the shared player input. |
| Ride shake / tilt | `src/Game/Ride/Plant.cpp`, `Creeper.cpp`, `Fluff.cpp`, `TubeSlider.cpp` read `isPadSwing(0)`. `JumpBranch.cpp` accepts A trigger or core swing. `SphereAccelSensorController.cpp` has the imported native Ball steering hint. No Ray ride claim is made for SMG2; its inherited input mode is retained. |

The 2P sound/percussion helper also reads 1/2, D-pad and +/− on channel 1.
Those are sound prompts rather than the requested grab/spin/shoot controls;
this change does not claim to provide every second-Remote accessory button.

## New native action names

Only missing device inputs get new actions: `NunchukShake` (H),
`CoStarToggle` (F2), `CoStarA` (U), `CoStarB` (Y), `CoStarShake` (N).
All imported defaults remain byte-for-byte identical in serialized form.
The second model is mouse-aimed and has no movement stick; analog trigger
bindings still work as remappable buttons. Co-star mouse aiming intentionally
shares the first pointer position; it does not provide independent two-mouse
multiplayer. Button streams and accelerometers are independent.
