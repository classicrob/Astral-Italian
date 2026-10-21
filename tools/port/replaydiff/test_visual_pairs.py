import csv
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

from visual_pairs import digest, inspect_provider, metrics, validate_idle


class VisualTests(unittest.TestCase):
    def test_identical_and_different_metrics(self):
        from PIL import Image
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory); a = root / 'a.png'; b = root / 'b.png'
            Image.new('RGB', (16, 16), (20, 100, 200)).save(a)
            Image.new('RGB', (16, 16), (20, 100, 200)).save(b)
            result = metrics(a, b)
            self.assertEqual(result['luma_block_ssim_8x8'], 1)
            self.assertEqual(result['normalized_rgb_mae'], 0)
            Image.new('RGB', (16, 16), (200, 10, 0)).save(b)
            self.assertGreater(metrics(a, b)['normalized_rgb_rmse'], .5)
            Image.new('RGB', (17, 16)).save(b)
            with self.assertRaises(ValueError): metrics(a, b)

    def test_missing_presentation_and_frame_mismatch_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            image = root / 'image.png'; image.write_bytes(b'synthetic')
            sync = root / 'sync.json'; sync.write_text(json.dumps(dict(phase='post-endFrame', stage='Synthetic', scenario=1, source_frame=100)))
            trace = root / 'trace.csv'
            with trace.open('w') as stream:
                writer = csv.DictWriter(stream, fieldnames=['frame', 'scene', 'stage', 'scenario', 'init_state', 'player_valid'])
                writer.writeheader()
                for frame in (0, 60): writer.writerow(dict(frame=frame, scene='Game', stage='Synthetic', scenario=1, init_state=6, player_valid=1))
            def bind(path): return dict(path=str(path), sha256=digest(path))
            provider = dict(image=bind(image), sync=bind(sync), trace=bind(trace), capture_source_frame=160)
            with patch('visual_pairs.private_path', side_effect=Path):
                with self.assertRaises(ValueError): inspect_provider(provider, 'Synthetic', 1, 60)
                receipt = root / 'presentation.json'; receipt.write_text('{}')
                provider.update(presentation_binding='frame-id-verified', presentation_receipt=bind(receipt))
                with self.assertRaises(ValueError): inspect_provider(provider, 'Synthetic', 1, 60)
                receipt.write_text(json.dumps(dict(schema_version=1, status='captured', stage='Synthetic', scenario=1,
                  ready_source_frame=100, capture_source_frame=160, rendered_source_frame=160,
                  image_sha256=digest(image), render_fence_complete=True, rendered_frame_id=32, readback_frame_id=31,
                  method='native-renderer-readback')))
                provider['presentation_receipt'] = bind(receipt)
                with self.assertRaises(ValueError): inspect_provider(provider, 'Synthetic', 1, 60)

    def test_nonidle_input_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'input.jsonl'; path.write_text('{}\n')
            with self.assertRaises(ValueError): validate_idle(path)


if __name__ == '__main__': unittest.main()
