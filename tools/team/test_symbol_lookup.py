import tempfile
import unittest
from pathlib import Path
from symbol_lookup import SYMBOLS, select_addresses, lookup_witness

class LookupTests(unittest.TestCase):
    def test_exact_symbols_exclude_aurora_main(self):
        text='0000000100000000 T _main\n'+'\n'.join(f'{0x100001000+i:016x} T {s}' for i,s in enumerate(SYMBOLS.values()))
        self.assertEqual(len(select_addresses(text)),3)
        with self.assertRaises(ValueError):select_addresses(text+'\n0000000100000000 T _petari_host_frame_seam')
        with self.assertRaises(ValueError):select_addresses(text.replace(SYMBOLS['game'],'_other'))
    def test_actual_lowercase_format_and_wrong_owner(self):
        with tempfile.TemporaryDirectory() as d:
            p=Path(d)/'native/app/frame_seam.cpp';p.parent.mkdir(parents=True);p.write_text('void x() {}')
            text=f"Line info: file '{p}', line 792, column 1, start file '{p}', start line 792"
            self.assertEqual(lookup_witness(text,[p])[0]['line'],792)
            with self.assertRaises(ValueError):lookup_witness(text.replace('frame_seam.cpp','main.cpp'),[p])
            with self.assertRaises(ValueError):lookup_witness(text.replace('line 792','line 0'),[p])
            with self.assertRaises(ValueError):lookup_witness('DW_AT_decl_line (792)',[p])
    def test_relative_exact_suffix_missing_and_ambiguous(self):
        with tempfile.TemporaryDirectory() as d:
            p=Path(d)/'native/app/frame_seam.cpp';p.parent.mkdir(parents=True);p.touch()
            text="Line info: file 'native/app/frame_seam.cpp', line 792, column 1"
            self.assertEqual(lookup_witness(text,[p])[0]['normalized_file'],str(p))
            with self.assertRaises(ValueError):lookup_witness(text,[p,p])
            p.unlink()
            with self.assertRaises(ValueError):lookup_witness(text,[p])
            with self.assertRaises(ValueError):lookup_witness(text.replace('native/app','native/app/../../native/app'),[p])

if __name__=='__main__':unittest.main()
