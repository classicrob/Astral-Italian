#!/usr/bin/env python3
"""Direct launch metadata must use the parsed request, before a catalog exists."""
from pathlib import Path
import unittest
ROOT=Path(__file__).resolve().parents[2]


class LaunchMetadata(unittest.TestCase):
    def test_parsed_launch_is_retained_after_artifact_setup_before_game_boot(self):
        source=(ROOT/'native/app/app_main.cpp').read_text()
        call='App::TapeRecord::setLaunch(launchStage, launchScenario);'
        self.assertEqual(source.count(call),1)
        self.assertLess(source.index('App::prepareRunArtifacts(locations);'),source.index(call))
        self.assertLess(source.index(call),source.index('int main('))
        self.assertLess(source.index(call),source.index('App::Host::startOS();'))

    def test_unresolved_catalog_request_is_not_used_for_launch_metadata(self):
        source=(ROOT/'native/app/app_main.cpp').read_text()
        self.assertNotIn('pendingStartupRequest()',source)
        self.assertIn('StartupStatus::PendingCatalog',(ROOT/'native/home_menu/launch_stage.cpp').read_text())


if __name__=='__main__':unittest.main()
