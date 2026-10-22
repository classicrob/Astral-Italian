"""Small geometry and report-to-SVG checks; no game data or build required."""
import importlib.util
from pathlib import Path
import unittest
import xml.etree.ElementTree as ET

spec = importlib.util.spec_from_file_location(
    'progress_treemap', Path(__file__).parents[1] / 'progress_treemap.py')
treemap = importlib.util.module_from_spec(spec)
spec.loader.exec_module(treemap)


class ProgressTreemapTests(unittest.TestCase):
    def test_area_bounds_and_no_overlaps(self):
        weights = [('a', 90), ('b', 20), ('c', 10), ('d', 1), ('zero', 0)]
        rectangles = list(treemap.squarify(weights, 4, 8, 100, 60))
        self.assertEqual(len(rectangles), 4)
        for key, x, y, w, h in rectangles:
            self.assertAlmostEqual(w * h, dict(weights)[key] / 121 * 6000)
            self.assertGreaterEqual(min(w, h), 0)
            self.assertGreaterEqual(x, 4)
            self.assertGreaterEqual(y, 8)
            self.assertLessEqual(x + w, 104.000001)
            self.assertLessEqual(y + h, 68.000001)
        for i, (_, x, y, w, h) in enumerate(rectangles):
            for _, xx, yy, ww, hh in rectangles[i + 1:]:
                overlap = max(0, min(x + w, xx + ww) - max(x, xx))
                overlap *= max(0, min(y + h, yy + hh) - max(y, yy))
                self.assertAlmostEqual(overlap, 0)

    def test_svg_units_colours_and_escaping(self):
        report = {'units': [
            {'name': name, 'measures': {'total_code': size, 'matched_code_percent': pct}}
            for name, size, pct in [('main/Game/Util/A&B', '100', 100),
                                    ('main/JSystem/JMath', '50', 50),
                                    ('main/RVL_SDK/os', '25', 0),
                                    ('main/zero', '0', 100)]]}
        svg = treemap.render(report)
        root = ET.fromstring(svg)
        ns = {'s': 'http://www.w3.org/2000/svg'}
        rects = [r for r in root.findall('s:rect', ns) if r.find('s:title', ns) is not None]
        self.assertEqual(len(rects), 3)
        self.assertEqual({r.attrib['fill'] for r in rects},
                         {'#7ac943', '#333333', treemap.colour(50)})
        self.assertIn('A&amp;B', svg)
        self.assertNotIn('main/zero', svg)
        self.assertEqual(treemap.folder('main/Game/Util/A'), 'Game/Util')
        self.assertEqual(treemap.folder('main/JSystem/JMath'), 'JSystem')
        self.assertEqual(svg, treemap.render(report))
        with self.assertRaises(ValueError):
            treemap.render({'units': []})


if __name__ == '__main__':
    unittest.main()
