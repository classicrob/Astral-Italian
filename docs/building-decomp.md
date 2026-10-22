# Building the Wii decompilation

These instructions preserve the upstream Garigari build/dependency guide, adapted
for this fork. They build the Wii decompilation, not a native Windows/Linux game.
For the macOS port, see [native/README.md](../native/README.md); for playing,
see [PLAY.md](../PLAY.md).

You need your own Super Mario Galaxy 2 USA Rev 0 (`SB4E01`) disc. No game assets
are included. Run commands from the repository root. `configure.py` generates
the Ninja build; decomp-toolkit (`dtk`) handles the original binary and
objdiff measures matching. Tool versions are pinned in `configure.py` and
required tools are downloaded by the build when no local override is supplied.

Dependencies
============

Windows
--------

On Windows, it's **highly recommended** to use native tooling. WSL or msys2 are **not** required.  
When running under WSL, [objdiff](#diffing) is unable to get filesystem notifications for automatic rebuilds.

- Install [Python](https://www.python.org/downloads/) and add it to `%PATH%`.
  - Also available from the [Windows Store](https://apps.microsoft.com/store/detail/python-311/9NRWMJP3717K).
- Download [ninja](https://github.com/ninja-build/ninja/releases) and add it to `%PATH%`.
  - Quick install via pip: `pip install ninja`

macOS
------

- Install [ninja](https://github.com/ninja-build/ninja/wiki/Pre-built-Ninja-packages):

  ```sh
  brew install ninja
  ```

[wibo](https://github.com/decompals/wibo), a minimal 32-bit Windows binary wrapper, will be automatically downloaded and used.

Linux
------

- Install [ninja](https://github.com/ninja-build/ninja/wiki/Pre-built-Ninja-packages).

[wibo](https://github.com/decompals/wibo), a minimal 32-bit Windows binary wrapper, will be automatically downloaded and used.

Building
========

- Clone the repository:

  ```sh
  git clone https://github.com/classicrob/Astral-Italian.git
  cd Astral-Italian
  ```

- Using [Dolphin Emulator](https://dolphin-emu.org/), extract your game to `orig/SB4E01`.
![](../assets/dolphin-extract.png)
  - To save space, the only necessary files are the following. Any others can be deleted.
    - `sys/main.dol`
- Configure:

  ```sh
  python configure.py
  ```

  The supported version is `SB4E01` (USA, Rev 0); `--version SB4E01` selects it explicitly.
- Build:

  ```sh
  ninja
  ```

Diffing
=======

Once the initial build succeeds, an `objdiff.json` should exist in the project root.

Download the latest release from [encounter/objdiff](https://github.com/encounter/objdiff). Under project settings, set `Project directory`. The configuration should be loaded automatically.

Select an object from the left sidebar to begin diffing. Changes to the project will rebuild automatically: changes to source files, headers, `configure.py`, `splits.txt` or `symbols.txt`.

![](../assets/objdiff.png)

