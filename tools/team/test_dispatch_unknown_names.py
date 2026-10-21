"""Pure synthetic names-only diagnostics; no process/signal/main/exec calls."""
import hashlib
import json
import unittest
import dispatch_exec_gate as gate

class UnknownNamesModels(unittest.TestCase):
    def observe(self,actual,requested=None):
        return gate.context_observation({'cwd':'/same','env':{} if requested is None else requested},'/same',actual)
    def test_only_differing_names_no_values_or_value_pins(self):
        value=self.observe({'ADDED_NAME':'private-value','SAME':'equal','CHANGED':'secret-a'}, {'REMOVED_NAME':'other-private','SAME':'equal','CHANGED':'secret-b'})
        rows=value['outside_allowlist']['different_names']
        self.assertEqual([r['name'] for r in rows],['ADDED_NAME','CHANGED','REMOVED_NAME'])
        self.assertEqual(set(rows[0]),{'name','actual_present','requested_present'})
        raw=json.dumps(value)
        for text in ('private-value','other-private','secret-a','secret-b',hashlib.sha256(b'private-value').hexdigest()):
            self.assertNotIn(text,raw)
        self.assertNotIn('SAME',[r['name'] for r in rows])
        self.assertFalse(value['env_equal']);self.assertTrue(value['cwd_equal'])
    def test_json_escaping_roundtrip(self):
        name='A\n\t"\\\u2603'
        value=self.observe({name:'NEVER_EXPORT'})
        raw=json.dumps(value,sort_keys=True,ensure_ascii=True)
        self.assertNotIn('NEVER_EXPORT',raw)
        self.assertIn('\\n',raw);self.assertIn('\\u2603',raw)
        self.assertEqual(json.loads(raw)['outside_allowlist']['different_names'][0]['name'],name)
    def test_count_exact_boundary_and_overflow(self):
        value=self.observe({('K%02d'%i)+'x'*125:'value' for i in range(16)})
        self.assertEqual(len(value['outside_allowlist']['different_names']),16)
        with self.assertRaisesRegex(RuntimeError,'count exceeds'):
            self.observe({'K%d'%i:'value' for i in range(17)})
    def test_character_and_escaped_byte_overflow(self):
        with self.assertRaisesRegex(RuntimeError,'size/type exceeds'):
            self.observe({'x'*129:'value'})
        # Each name is <=128chars and <=512bytes, but escaped aggregate >8192.
        with self.assertRaisesRegex(RuntimeError,'escaped names exceed'):
            self.observe({str(i)+'\x01'*125:'value' for i in range(16)})
    def test_limits_and_strict_equality_not_normalized(self):
        value=self.observe({'MixedCase':'v'},{'MIXEDCASE':'v'})
        self.assertFalse(value['env_equal'])
        self.assertEqual(len(value['outside_allowlist']['different_names']),2)
        self.assertLessEqual(len(json.dumps(value).encode()),gate.CONTEXT_RECORD_CAP)
        self.assertEqual(value['limits']['unknown_differing_names']['maximum_count'],16)

if __name__=='__main__':unittest.main()
