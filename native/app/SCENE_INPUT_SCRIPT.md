# Scene-spanning keyboard diagnostic

Set `PETARI_INPUT_SCRIPT_SCENE_SPAN=1` with the existing `PETARI_INPUT_SCRIPT` path. This is opt-in test input; it does not select stages, move actors, change game state, or alter bindings. With the option absent the existing script behavior is unchanged.

The script still waits for steady normal gameplay before its first line. In addition to `<frames> [key ...]`, it accepts `scene <scene-name> [stage-name]`. Such a line releases all keys and waits for an initialized matching scene. Loading pauses the frame counter and releases held keys. If loading interrupted a held-key line, its remaining frames are discarded. A completed previous line never consumes the following line. Non-Game scene input has empty player/camera CSV cells.

For example, after an ordinary route has reached ScenarioSelect:

```
scene ScenarioSelect
120
6 Backspace
scene Game WorldMap01Galaxy
180
```

This example describes input sequencing only; it does not establish the game's route or promise that Backspace produces that destination. Confirm each actual scene transition in the log. Key releases use the normal input packet path; leave idle frames after scene guards so ordinary queued reports can drain. Host-pointer mapping is outside this diagnostic's coverage.

Scene/line logs identify which scene received each key line and when a transition released an interrupted line. Capture labels identify a phase and presentation index; they do not certify that surface and XFB images show the identical game frame.
