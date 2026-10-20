#!/usr/bin/env python3
"""Contract tests only. Mock receipts/opaque test bytes are NOT game verification."""
import copy
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location("unlocked", ROOT / "native/tools/make_unlocked_save.py")
u = importlib.util.module_from_spec(spec)
spec.loader.exec_module(u)


def inventory(run_id):
    # Synthetic 242-record fixture, NOT game metadata or runtime verification.
    # Multiple scenarios can map to one actual star; ScenarioNo is not counted.
    stars = [dict(galaxy=g, star_id=i, world=w, green=green, engine_validated=True,
                  scenario_numbers=scenarios, provenance="synthetic test mapping")
             for g, i, w, green, scenarios in [("TestA", 1, 1, False, [1, 4]),
                 ("TestA", 2, 1, True, [7]), ("TestB", 1, 6, False, [2]),
                 ("TestS", 1, 7, False, [8])]]
    stars.extend(dict(galaxy=f"TestGreen{i:03}", star_id=1, world=7, green=True,
                      engine_validated=True, scenario_numbers=[9], provenance="synthetic test mapping")
                 for i in range(238))
    profiles = {v: {"supported": True, "checks": [dict(id=c, applicability="required",
                 provenance="synthetic getter") for c in sorted(u.REQUIRED_CHECKS[v])]}
                for v in u.VARIANTS}
    return dict(schema_version=1, game="SB4E01", run_id=run_id, complete=True, engine_validated=True,
                provider="ScenarioDataParser+GameDataAllGalaxyStorage", game_maximum=242,
                catalog_sha256="a"*64, provenance="synthetic unit test, NOT game metadata",
                stars=stars, profiles=profiles, slot=1, seed_owned=[], seed_owned_count=0)


class InventoryTests(unittest.TestCase):
    def test_variant_counts_derive_from_validated_unique_identities(self):
        inv = inventory("run")
        self.assertEqual([len(u.make_plan(inv, v, "run")["stars"]) for v in u.VARIANTS], [2, 3, 242])
        self.assertEqual(u.make_plan(inv, "all-stars", "run")["stars"][1], {"galaxy": "TestA", "star_id": 2})

    def test_duplicate_actual_star_rejected(self):
        inv = inventory("run")
        inv["stars"][-1] = copy.deepcopy(inv["stars"][0])
        with self.assertRaisesRegex(ValueError, "duplicate star identity"):
            u.make_plan(inv, "all-stars", "run")

    def test_incomplete_unsafe_or_unresolved_mapping_rejected(self):
        for mutate in [lambda i:i.update(complete=False), lambda i:i.update(game_maximum=4),
                       lambda i:i["stars"][0].update(star_id=0),
                       lambda i:i["stars"][0].update(engine_validated=False),
                       lambda i:i["stars"][0].update(scenario_numbers=[True]),
                       lambda i:i["stars"][0].update(scenario_numbers=[1,1]),
                       lambda i:i["profiles"]["all-stars"].update(supported=False)]:
            inv = inventory("run"); mutate(inv)
            with self.assertRaises(ValueError): u.make_plan(inv, "all-stars", "run")

    def test_luigi_luma_applicability_requires_evidence_and_readback(self):
        inv = inventory("run")
        check = next(c for c in inv["profiles"]["all-stars"]["checks"] if c["id"] == "master-luma")
        check.update(applicability="not_applicable", reason="test-only inapplicability evidence")
        self.assertEqual(len(u.make_plan(inv, "all-stars", "run")["stars"]),242)
        del check["reason"]
        with self.assertRaisesRegex(ValueError, "not-applicable reason"): u.make_plan(inv, "all-stars", "run")

    def test_missing_or_waived_world_check_rejected(self):
        inv = inventory("run"); inv["profiles"]["postgame"]["checks"] = []
        with self.assertRaises(ValueError): u.make_plan(inv, "postgame", "run")
        inv = inventory("run"); inv["profiles"]["postgame"]["checks"][0].update(applicability="not_applicable", reason="no")
        with self.assertRaises(ValueError): u.make_plan(inv, "postgame", "run")

    def test_real_generation_refuses_unapproved_host_before_reading_seed(self):
        host = subprocess.CompletedProcess([], 0, stdout="M1-test-only\n")
        with patch.object(u.sys, "platform", "darwin"), patch.object(u.subprocess, "run", return_value=host), \
             patch.object(u, "tree_hashes") as inspect:
            with self.assertRaisesRegex(ValueError, "server-only"):
                u.generate("all-stars", "/nonexistent-seed", "/nonexistent-output", "/no-app", "/no-disc")
            inspect.assert_not_called()

    def test_approved_server_requires_exact_hostname_and_metadata_marker(self):
        # Synthetic host/README only; no game data or real executable.
        with tempfile.TemporaryDirectory() as temporary:
            home=Path(temporary)
            host=subprocess.CompletedProcess([],0,stdout="MacBook-Pro-52\n")
            with patch.object(u.sys,"platform","darwin"), patch.object(u.Path,"home",return_value=home), \
                 patch.object(u.subprocess,"run",return_value=host):
                with self.assertRaisesRegex(ValueError,"server-only"): u.require_server_execution()
                marker=home/"MacioGalaxy2-gamedata/README.txt"
                marker.parent.mkdir()
                marker.write_text("SYNTHETIC HOST GUARD TEST, NOT GAMEDATA")
                u.require_server_execution()
                host.stdout="MacBook-Pro-52-other\n"
                with self.assertRaisesRegex(ValueError,"server-only"): u.require_server_execution()
            with patch.object(u.sys,"platform","linux"):
                with self.assertRaisesRegex(ValueError,"approved macOS server"): u.require_server_execution()

    def test_game_data_paths_refuse_repo_build_team_and_attachment_areas(self):
        for path in (ROOT / "ignored-data", "/private/build/save", "/private/MacioGalaxy2-team/save",
                     "/private/.bb-machines/Attachments/catalog", "/private/Attachments/save"):
            with self.subTest(path=path), self.assertRaises(ValueError):
                u.require_private_data_path(path)
        with tempfile.TemporaryDirectory() as temporary:
            repo=Path(temporary)/"worktree"
            repo.mkdir()
            (repo/".git").write_text("gitdir: test-only")
            with self.assertRaisesRegex(ValueError,"repository/worktree"):
                u.require_private_data_path(repo/"private-catalog")

    def test_legacy_smg1_variants_rejected(self):
        for name in ("all-missions", "grand-finale", "complete-luigi"):
            with self.assertRaises(ValueError): u.make_plan(inventory("run"), name, "run")


class PipelineTests(unittest.TestCase):
    def setUp(self):
        # All fixtures/receipts here are synthetic. Only the production host
        # guard is mocked; no real executable or game/save data is used locally.
        host_guard = patch.object(u, "require_server_execution")
        host_guard.start()
        self.addCleanup(host_guard.stop)
        self.temp = tempfile.TemporaryDirectory(); self.addCleanup(self.temp.cleanup)
        self.root=Path(self.temp.name); self.seed=self.root/"seed"; self.out=self.root/"published"
        save=self.seed/u.SAVE_RELATIVE; save.parent.mkdir(parents=True);save.write_bytes(b"opaque TEST seed, not a save format")
        (save.parent/"other.bin").write_bytes(b"preserve")
        self.app=self.root/"test-executable"; self.app.write_text("not executed: mocked subprocess only"); self.app.chmod(0o700)
        self.disc=self.root/"test-disc";self.disc.mkdir()
        self.modify_receipt = lambda r:r
        self.modify_inventory = lambda i:i
        self.work = None

    def game_double(self, app, disc, work, action, timeout):
        """Only protocol transport double; never launch/verify an actual game."""
        self.work=work
        (work/f"{action}.log").write_text("MOCK ONLY, NOT GAME SAVE VERIFICATION\n")
        run_id=json.loads((work/u.MARKER).read_text())["run_id"]
        if action == "inventory":
            u.write_json(work/u.INVENTORY,self.modify_inventory(inventory(run_id)));return
        req=u.read_json(work/u.REQUEST)
        (work/u.SAVE_RELATIVE).write_bytes(b"opaque MOCK result, NOT a game save")
        receipt=dict(schema_version=1,game="SB4E01",provider="GameDataHolder+SaveDataHandleSequence",
                     **{key:req[key] for key in ("run_id","variant","catalog_sha256","app_sha256","seed_save_sha256","inventory_sha256","slot")},
                     request_sha256=u.sha256(work/u.REQUEST),before_owned=[],before_owned_count=0,
                     after_owned=[dict(**r,bronze=False) for r in req["stars"]],owned_count=len(req["stars"]),
                     checks=[dict(id=c["id"],status="pass" if c["applicability"]=="required" else "not_applicable",
                                  provenance="synthetic getter") for c in req["checks"]],
                     game_data_corrupted=False,config_data_corrupted=False,save_sha256=u.sha256(work/u.SAVE_RELATIVE),
                     **{c:True for c in u.ROUND_TRIP_CHECKS})
        u.write_json(work/u.RECEIPT,self.modify_receipt(receipt))

    def generate(self, variant="all-stars"):
        with patch.object(u,"run_game",side_effect=self.game_double):
            return u.generate(variant,self.seed,self.out,self.app,self.disc)

    def test_mock_transport_preserves_seed_and_records_nand_ledger(self):
        before=u.tree_hashes(self.seed/"NAND")
        result=self.generate()
        self.assertEqual(result["selected_count"],242);self.assertEqual(result["game_owned_count"],242)
        ledger=u.read_json(self.out/u.LEDGER)
        self.assertEqual(ledger["before"],before)
        self.assertEqual(ledger["after_inventory"],before)
        self.assertEqual(ledger["after_game_save"],u.tree_hashes(self.out/"NAND"))
        self.assertEqual(ledger["changed"],[str(u.SAVE_RELATIVE.relative_to("NAND"))])
        self.assertEqual(u.sha256(self.out/u.LEDGER),result["nand_ledger_sha256"])
        self.assertEqual(u.tree_hashes(self.seed/"NAND"),before)
        self.assertFalse((self.out/u.MARKER).exists());self.assertFalse(self.work.exists())
        self.assertEqual(u.sha256(self.out/u.SAVE_RELATIVE),result["save_sha256"])

    def test_refuse_existing_output(self):
        self.out.mkdir();(self.out/"keep").write_text("untouched")
        with self.assertRaisesRegex(ValueError,"already exists"):self.generate()
        self.assertEqual((self.out/"keep").read_text(),"untouched")

    def test_refuse_symlinks_and_personal_directory(self):
        (self.seed/"NAND"/"linked").symlink_to(self.root)
        with self.assertRaisesRegex(ValueError,"symlink"):self.generate()
        with self.assertRaisesRegex(ValueError,"personal"):u.reject_personal((Path.home()/"Library/Application Support/Petari/a").resolve())

    def test_game_failure_timeout_or_only_legacy_log_never_publish(self):
        for code in [1,0]:
            def fail(app,disc,work,action,timeout):
                (work/f"{action}.log").write_text("PETARI UNLOCKED SAVE: VERIFIED (grand-finale,120)\n")
                if code:raise ValueError("game failed")
            with patch.object(u,"run_game",side_effect=fail),self.assertRaises(ValueError):
                u.generate("all-stars",self.seed,self.out,self.app,self.disc)
            self.assertFalse(self.out.exists())
        with patch.object(u.subprocess,"run",side_effect=subprocess.TimeoutExpired("test",1)):
            work=self.root/"timeout";work.mkdir()
            with self.assertRaisesRegex(ValueError,"timed out"):u.run_game(self.app,self.disc,work,"inventory",1)

    def test_each_failed_game_verification_refuses_publication(self):
        for field in u.ROUND_TRIP_CHECKS+("game_data_corrupted","config_data_corrupted"):
            self.modify_receipt=lambda r,f=field:{**r,f:not r[f]}
            with self.subTest(field=field),self.assertRaises(ValueError):self.generate()
            self.assertFalse(self.out.exists())

    def test_wrong_identity_count_hash_or_semantic_check_refuses(self):
        mutations=[lambda r:{**r,"run_id":"stale"},lambda r:{**r,"owned_count":241},
                   lambda r:{**r,"save_sha256":"0"*64},lambda r:{**r,"after_owned":[]},
                   lambda r:{**r,"request_sha256":"0"*64},
                   lambda r:{**r,"checks":[{**c,"status":"fail"} for c in r["checks"]]}]
        for mutation in mutations:
            self.modify_receipt=mutation
            with self.assertRaises(ValueError):self.generate()
            self.assertFalse(self.out.exists())

    def test_unresolved_profile_does_not_launch_mutation(self):
        self.modify_inventory=lambda i:{**i,"profiles":{}}
        with self.assertRaisesRegex(ValueError,"unsupported"):self.generate()
        self.assertFalse((self.work/"all-stars.log").exists())

    def test_inventory_cannot_change_nand(self):
        original=self.game_double
        def mutate(*args):
            original(*args)
            if args[3]=="inventory":(args[2]/u.SAVE_RELATIVE).write_bytes(b"changed")
        with patch.object(u,"run_game",side_effect=mutate),self.assertRaisesRegex(ValueError,"inventory action modified"):
            u.generate("all-stars",self.seed,self.out,self.app,self.disc)
        self.assertFalse(self.out.exists())

    def test_nand_changes_outside_save_refused(self):
        original=self.game_double
        def mutate(*args):
            original(*args)
            if args[3]!="inventory":(args[2]/u.SAVE_RELATIVE).with_name("other.bin").write_bytes(b"bad")
        with patch.object(u,"run_game",side_effect=mutate),self.assertRaisesRegex(ValueError,"outside GameData"):
            u.generate("all-stars",self.seed,self.out,self.app,self.disc)
        self.assertFalse(self.out.exists())

    def test_output_race_never_overwrites(self):
        original=self.game_double
        def race(*args):
            original(*args)
            if args[3]!="inventory":self.out.mkdir();(self.out/"keep").write_text("concurrent")
        with patch.object(u,"run_game",side_effect=race),self.assertRaises(FileExistsError):
            u.generate("all-stars",self.seed,self.out,self.app,self.disc)
        self.assertEqual((self.out/"keep").read_text(),"concurrent")
        self.assertFalse((self.out/"unlocked-save.json").exists())

    def test_bronze_or_unselected_star_loss_refused(self):
        def bronze(r):r["after_owned"][0]["bronze"]=True;return r
        self.modify_receipt=bronze
        with self.assertRaisesRegex(ValueError,"bronze"):self.generate()
        def loss(r):r.update(before_owned=[dict(galaxy="TestA",star_id=2,bronze=False)],before_owned_count=1);return r
        self.modify_receipt=loss
        with self.assertRaisesRegex(ValueError,"seed snapshot"):self.generate("worlds1-6")

    def test_high_progress_seed_cannot_launch_smaller_exact_profile(self):
        for variant in ("worlds1-6", "postgame"):
            def high_progress(inv):
                inv.update(seed_owned=[dict(galaxy="TestA",star_id=2,bronze=False)],seed_owned_count=1)
                return inv
            self.modify_inventory=high_progress
            with self.subTest(variant=variant), self.assertRaisesRegex(ValueError,"out-of-profile"):
                self.generate(variant)
            self.assertFalse((self.work/f"{variant}.log").exists())
            self.assertFalse(self.out.exists())

    def test_self_consistent_partial_inventory_cannot_fake_all_stars(self):
        inv=inventory("run")
        inv["stars"]=inv["stars"][:4]
        inv["game_maximum"]=4
        with self.assertRaisesRegex(ValueError,"242 actual identities"):
            u.make_plan(inv,"all-stars","run")

    def test_executable_request_or_seed_changes_refuse_publication(self):
        for kind in ("executable", "request", "seed"):
            original=self.game_double
            def mutate(*args):
                original(*args)
                if args[3] != "inventory":
                    path={"executable": self.app, "request": args[2]/u.REQUEST,
                          "seed": self.seed/u.SAVE_RELATIVE}[kind]
                    path.write_bytes(b"unexpected mutation")
            with self.subTest(kind=kind), patch.object(u,"run_game",side_effect=mutate), self.assertRaises(ValueError):
                u.generate("all-stars",self.seed,self.out,self.app,self.disc)
            self.assertFalse(self.out.exists())


if __name__ == "__main__":unittest.main()
