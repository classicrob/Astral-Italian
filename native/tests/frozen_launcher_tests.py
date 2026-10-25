#!/usr/bin/env python3
"""Exercise frozen-only player launchers without starting a game."""
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

PLAY = Path(__file__).resolve().parents[1] / "play"


class FrozenLauncherTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.home = Path(self.temp.name)
        self.root = self.home / "MacioGalaxy2-play"
        self.disc = self.home / "disc"
        (self.disc / "files").mkdir(parents=True)
        self.env = {**os.environ, "HOME": str(self.home), "ASTRAL_DISC": str(self.disc)}
        for key in ("ASTRAL_APP", "ASTRAL_USER", "ASTRAL_LOG", "ASTRAL_CAPTURES"):
            self.env.pop(key, None)

    def app(self, parent, frozen=True):
        app = parent / "Petari.app"
        exe = app / "Contents/MacOS/Petari"
        exe.parent.mkdir(parents=True)
        exe.write_text("#!/bin/sh\nexit 99\n")
        exe.chmod(0o755)
        if frozen:
            (parent / "FROZEN.txt").write_text("Fixture freeze\n")
        return app

    def run_play(self, *args, app=None):
        env = dict(self.env)
        if app is not None:
            env["ASTRAL_APP"] = str(app)
        return subprocess.run(["sh", str(PLAY / "play.sh"), *args], env=env,
                              text=True, capture_output=True)

    def test_default_and_named_frozen_dry_launch(self):
        self.app(self.root)
        self.app(self.root / "build5.4.2")
        self.app(self.root / "build3")
        for args in (("--dry-run",), ("--build", "default", "--dry-run"),
                     ("--build", "build4", "--dry-run"), ("--build", "build3", "--dry-run")):
            result = self.run_play(*args)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("  run  ", result.stdout)
            expected = self.root / "Petari.app" if "build4" in args else self.root / ("build3" if "build3" in args else "build5.4.2") / "Petari.app"
            self.assertIn(str(expected / "Contents/MacOS/Petari"), result.stdout)

    def test_no_argument_menu_selects_default_build5_4_2(self):
        self.app(self.root)
        app = self.app(self.root / "build5.4.2")
        result = subprocess.run(["sh", str(PLAY.parents[1] / "Play Astral Italian.command")],
                                input="q\n", env=self.env, text=True, capture_output=True)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("Frozen build: " + str(app.parent.resolve()), result.stdout)

    def test_unfrozen_rejected_before_state_creation(self):
        app = self.app(self.root, frozen=False)
        result = self.run_play("--dry-run", app=app)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("missing FROZEN.txt", result.stderr)
        self.assertFalse((self.root / "user").exists())

    def test_external_marked_app_and_symlink_escape_rejected(self):
        outside = self.app(self.home / "developer-build")
        self.root.mkdir()
        alias = self.root / "Petari.app"
        alias.symlink_to(outside, target_is_directory=True)
        for app in (outside, alias):
            result = self.run_play("--dry-run", app=app)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("under ~/MacioGalaxy2-play", result.stderr)
        self.assertFalse((self.root / "user").exists())

    def test_discovery_omits_unfrozen_and_external_apps(self):
        self.app(self.root)
        for name in ("build5.4.2", "build5.4.1", "build5.4", "build5.3.2", "build5.2.1", "build5.1", "build3"):
            self.app(self.root / name)
        self.app(self.root / "unfrozen", frozen=False)
        outside = self.app(self.home / "outside")
        (self.root / "escaped").mkdir()
        (self.root / "escaped/Petari.app").symlink_to(outside, target_is_directory=True)
        (self.root / "escaped/FROZEN.txt").write_text("Not sufficient\n")
        result = self.run_play("--list-builds")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("5.4.2 (default; build5.4.2)", result.stdout)
        self.assertIn("5.4.1 (fallback; build5.4.1)", result.stdout)
        self.assertIn("5.4 (fallback; build5.4)", result.stdout)
        self.assertIn("5.3.2 (fallback; build5.3.2)", result.stdout)
        self.assertIn("5.2.1 with Cappy (fallback; build5.2.1)", result.stdout)
        self.assertEqual(result.stdout.count("build5.4.2)"), 1)  # listed once, not again from the directory glob
        self.assertEqual(result.stdout.count("build5.4.1)"), 1)
        self.assertEqual(result.stdout.count("build5.4)"), 1)
        self.assertIn("build4", result.stdout)
        self.assertIn("build3", result.stdout)
        self.assertNotIn("unfrozen", result.stdout)
        self.assertNotIn("escaped", result.stdout)

    def test_executable_symlink_and_build_traversal_rejected(self):
        app = self.app(self.root)
        exe = app / "Contents/MacOS/Petari"
        external = self.home / "external-exe"
        exe.rename(external)
        exe.symlink_to(external)
        self.assertNotEqual(self.run_play("--dry-run", app=app).returncode, 0)
        self.assertNotEqual(self.run_play("--build", "../outside", "--dry-run").returncode, 0)

    def test_frozen_smoke_rejects_unfrozen_without_launch(self):
        app = self.app(self.root, frozen=False)
        result = subprocess.run(["sh", str(PLAY / "smoke_frozen.sh")],
                                env={**self.env, "ASTRAL_APP": str(app)},
                                text=True, capture_output=True)
        self.assertEqual(result.returncode, 2)
        self.assertIn("missing FROZEN.txt", result.stderr)
        self.assertFalse((self.root / "smoke").exists())


if __name__ == "__main__":
    unittest.main()
