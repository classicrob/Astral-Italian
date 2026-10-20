import unittest
import tracediff
from human_pair import gx_departure, raw_first


class HumanPairTests(unittest.TestCase):
    def test_gx_proxy_phase_and_rounding(self):
        trace = tracediff.Trace(['frame', 'player_valid', 'x', 'y', 'z'], {
            (0,): dict(player_valid='1', x='1.00027', y='2', z='3'),
            (1,): dict(player_valid='1', x='4.02', y='5', z='6')}, [])
        gx = [dict(valid='0')] * 2 + [dict(valid='1', x='1.000', y='2.000', z='3.000'),
                                                  dict(valid='1', x='4.000', y='5.000', z='6.000')]
        self.assertEqual(gx_departure(trace, gx, 2, 0)['trace_frame'], 0)
        result = gx_departure(trace, gx, 2, 0.0005)
        self.assertEqual((result['trace_frame'], result['gx_source_frame']), (1, 3))

    def test_invalid_gx_not_position_evidence(self):
        trace = tracediff.Trace(['frame'], {(0,): dict(player_valid='1', x='1', y='2', z='3')}, [])
        self.assertIsNone(gx_departure(trace, [dict(valid='0')], 0, 0))

    def test_raw_includes_bits_and_missing_rows(self):
        a = tracediff.Trace(['x_bits'], {(0,): {'x_bits': '0x80000000'}}, [])
        b = tracediff.Trace(['x_bits'], {(0,): {'x_bits': '0x00000000'}}, [])
        self.assertEqual(raw_first(a, b)['fields'][0]['field'], 'x_bits')


if __name__ == '__main__':
    unittest.main()
