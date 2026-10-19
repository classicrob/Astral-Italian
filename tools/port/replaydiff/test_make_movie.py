import struct
import unittest

from make_movie import encode, packet, REVISION


class MovieTests(unittest.TestCase):
    def test_header_matches_pinned_packed_layout(self):
        movie = encode([{"polls": 7}, {"polls": 3, "buttons": ["A", "B"]}])
        self.assertEqual(movie[:13], b"DTM\x1aSB4E01\x01\x10\x00")
        self.assertEqual(struct.unpack_from("<QQ", movie, 13), (10, 10))
        self.assertEqual(struct.unpack_from("<Q", movie, 129), (946684800,))
        self.assertEqual(struct.unpack_from("<Q", movie, 237), (10 * 729000000,))
        self.assertEqual(movie[137], 0)
        self.assertEqual(movie[209:229], bytes.fromhex(REVISION))
        pos = 256
        polls = 0
        while pos < len(movie):
            pos += 1 + movie[pos]
            polls += 1
        self.assertEqual((pos, polls), (len(movie), 10))

    def test_buttons_and_camera_use_independent_fields(self):
        data = packet({"polls": 1, "buttons": ["A", "B", "RIGHT"], "ir": [512, 384]})
        self.assertEqual(data[0], len(data) - 1)
        self.assertEqual(data[1:4], bytes((0x15, 3, 8)))
        # Two invisible IR objects and a neutral Nunchuk remain present.
        self.assertEqual(data[-12:-6], b"\xff" * 6)
        self.assertEqual(data[-6:], bytes((128, 128, 128, 128, 179, 3)))

    def test_unknown_and_out_of_range_inputs_fail(self):
        for segment in ({"polls": 1, "buttons": ["TYPO"]},
                        {"polls": 1, "ir": [5, 300]},
                        {"polls": 1, "stick": [-1, 128]},
                        {"polls": 1, "typo": 1}):
            with self.assertRaises(ValueError):
                packet(segment)
        with self.assertRaises(ValueError):
            encode([{"polls": 0}])


if __name__ == "__main__":
    unittest.main()
