import copy
import unittest
from route_to_kpad_script import convert


def rows():
    result = []
    for i in range(8):
        result.append(dict(frame=str(i), stage='FileSelect' if i < 2 else 'RedBlueExGalaxy',
                           scenario='1', valid='1' if i != 4 else '0', dead='1' if i >= 6 else '0',
                           move_x='0.70711', move_y='0.70711', x='1.000', y='2.000', z='3.000',
                           jump='1' if i in (3, 4) else '0', spin='1' if i in (4, 5) else '0',
                           crouch='0', camera_left='0', camera_right='0', camera_center='0',
                           camera_up='0', camera_down='0'))
    return result


class ConversionTests(unittest.TestCase):
    def test_target_anchor_and_phase(self):
        script, receipt = convert(rows())
        self.assertEqual(receipt['anchor_source_frame'], 2)
        self.assertEqual(receipt['input_source_start'], 3)
        self.assertEqual(script['frames'], 5)
        self.assertEqual(receipt['trace_frames'], 6)
        self.assertEqual(script['segments'][0]['buttons'], ['A'])

    def test_dead_and_invalid_preserved(self):
        script, receipt = convert(rows())
        self.assertEqual(receipt['invalid_frames_retained'], 1)
        self.assertEqual(receipt['dead_frames_retained'], 2)
        self.assertEqual(sum(s['end'] - s['start'] for s in script['segments']), 5)

    def test_spin_and_analog(self):
        script, receipt = convert(rows())
        self.assertEqual(receipt['spin_rising_frames'], [1])
        for frame in range(script['frames']):
            item = next(s for s in script['segments'] if s['start'] <= frame < s['end'])
            self.assertEqual(item['stick'], [0.70711, 0.70711])
            self.assertEqual(item['acc'], [3, -1, 0] if frame in (1, 2) else [0, -1, 0])

    def test_all_buttons_and_prefix(self):
        source = rows()
        for key in ['jump', 'crouch', 'camera_left', 'camera_right', 'camera_center', 'camera_up', 'camera_down']:
            source[3][key] = '1'
        script, receipt = convert(source, limit=1)
        self.assertEqual(set(script['segments'][0]['buttons']), {'A', 'Z', 'LEFT', 'RIGHT', 'C', 'UP', 'DOWN'})
        self.assertEqual(receipt['input_source_end'], 3)

    def test_end_on_stage_change(self):
        source = rows()
        source[6]['stage'] = 'Other'
        script, receipt = convert(source)
        self.assertEqual(script['frames'], 3)
        self.assertEqual(receipt['input_source_end'], 5)

    def test_reject_invalid(self):
        for key, value in [('frame', '99'), ('valid', '2'), ('move_x', 'nan'), ('move_y', '1.1')]:
            source = copy.deepcopy(rows())
            source[4][key] = value
            with self.subTest(key=key), self.assertRaises(ValueError):
                convert(source)

    def test_missing_anchor(self):
        with self.assertRaises(ValueError):
            convert(rows(), stage='Unknown')


if __name__ == '__main__':
    unittest.main()
