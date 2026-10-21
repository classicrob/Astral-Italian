import unittest
from unlocked_load52 import load_witness
class LoadWitness(unittest.TestCase):
 def setUp(self):self.ui='\n'.join('[milestone] '+s for s in ('FileSelector.Title','FileSelector.FileSelectStart','FileSelector.FileSelect','FileSelector.DemoStartWait'));self.rows=[{'valid':'1','power_stars':'242'}]
 def test_actual_ui_without_direct_marker(self):self.assertTrue(load_witness('normal-boot-ui',self.ui,self.rows,['Petari','--user','copy']))
 def test_direct_adapter(self):self.assertTrue(load_witness('worldmap','FileSelector.OpenFile',self.rows,['Petari','--stage','WorldMap01Galaxy']))
 def test_wrong_ui_or_adapter(self):
  self.assertFalse(load_witness('normal-boot-ui','FileSelector.OpenFile',self.rows,['Petari']))
  self.assertFalse(load_witness('worldmap',self.ui,self.rows,['Petari','--stage','WorldMap01Galaxy']))
 def test_no_ui_stage_substitute(self):self.assertFalse(load_witness('normal-boot-ui',self.ui,self.rows,['Petari','--stage','MarioFaceShipGalaxy']))
 def test_missing_star_or_create(self):
  self.assertFalse(load_witness('normal-boot-ui',self.ui,[{'valid':'1','power_stars':'0'}],['Petari']))
  self.assertFalse(load_witness('normal-boot-ui',self.ui+'FileSelector.Create',self.rows,['Petari']))
 def test_unordered_ui(self):self.assertFalse(load_witness('normal-boot-ui','\n'.join(reversed(self.ui.splitlines())),self.rows,['Petari']))
if __name__=='__main__':unittest.main()
