from pathlib import Path
import struct
import tempfile
import unittest
from stage_clear_witness import ClearWitness, decode
from dolphin_collect import ObserverError


class ClearWitnessTests(unittest.TestCase):
    def raw(self):
        raw = bytearray(0x4c)
        raw[0x12:0x12+len(b'YosshiHomeGalaxy')] = b'YosshiHomeGalaxy'
        raw[0x42:0x44] = b'\1\1'
        struct.pack_into('>i', raw, 0x48, 1)
        return bytes(raw)

    def test_retail_offsets(self):
        result = decode(self.raw())
        self.assertEqual(result, dict(result_stage='YosshiHomeGalaxy', stage_result=True,
                                      new_power_star=True, cleared_star_id=1))
        with self.assertRaises(ObserverError):
            decode(self.raw()[:-1])
        raw = bytearray(self.raw()); raw[0x43] = 2
        with self.assertRaises(ObserverError): decode(raw)

    def test_ship_alone_never_clear_and_partial_never_success(self):
        class Remote:
            def read(self, address, size): return bytes(0x4c)
        with tempfile.TemporaryDirectory() as directory:
            witness = ClearWitness(Path(directory)); witness.bound = True; witness.holder = 0x80001000
            witness.sample(Remote(), dict(frame=10, stage='MarioFaceShipGalaxy', scenario=2, init_state=6, scene='Game'))
            receipt = witness.close(True)
            self.assertFalse(receipt['reproduces_clear'])
        with tempfile.TemporaryDirectory() as directory:
            witness = ClearWitness(Path(directory))
            witness.receipt.update(yoshi_star_result={'frame': 1}, starship_ready={'frame': 2})
            self.assertFalse(witness.close(False)['reproduces_clear'])
            self.assertFalse(witness.path.exists())

    def test_result_then_ship_required(self):
        raw = self.raw()
        class Remote:
            def read(self, address, size): return raw
        with tempfile.TemporaryDirectory() as directory:
            witness = ClearWitness(Path(directory)); witness.bound = True; witness.holder = 0x80001000
            witness.sample(Remote(), dict(frame=10, stage='YosshiHomeGalaxy', scenario=1, init_state=6, scene='Game'))
            witness.sample(Remote(), dict(frame=20, stage='MarioFaceShipGalaxy', scenario=2, init_state=6, scene='Game'))
            self.assertTrue(witness.close(True)['reproduces_clear'])


if __name__ == '__main__': unittest.main()
