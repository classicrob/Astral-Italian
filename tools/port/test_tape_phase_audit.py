#!/usr/bin/env python3
import copy
import unittest
from tape_phase_audit import audit_actions, audit_inputs, audit_native, bootstrap, validate


def marker(f=3):
    return dict(input_frame=f, kind='controller-normal', scene='Game', stage='WorldMap01Galaxy', scenario=1,
                first_normal_update=True, valid=True, grounded=False, dead=False, position_bits=[0, 0, 0],
                rng_observed=True, rng_seed=99, map_wait=False, galaxy_map=False)


def fixture():
    phase = dict(id=0, first_input_frame=3, end_input_frame_exclusive=7, end_kind='recording-prefix', marker=marker(), checkpoints=[])
    meta = dict(schema=2, marker_protocol='game-scene-pre-read-v1', marker_error='', seams=7,
                launch=dict(stage='WorldMap01Galaxy', scenario=1), input_phases=[phase])
    reads = [dict(bootstrap(0, 120), frame=f, ordinal=0) for f in range(3, 7)]
    route = [dict(frame=str(f)) for f in range(8)]
    mapping = [dict(actual_frame=f, actual_ordinal=0, source='recorded', canonical_frame=f,
                    canonical_ordinal=0, phase=0, channel=0, capacity=120, returned=1, error=0, samples=1) for f in range(3, 7)]
    report = dict(finished=True, stopped='', anchor_policy='game-defined-scene-input-v2',
                  segments=[dict(scene_anchor_frame_offset=0)])
    return meta, route, reads, mapping, report


class Audit(unittest.TestCase):
    def test_valid(self):
        m, r, k, mapping, report = fixture()
        self.assertEqual(validate(m, r, k)['active_input_loops'], 4)
        self.assertEqual(audit_inputs(k, k, mapping, report)['exact_recorded_reads'], 4)

    def reject_meta(self, fn):
        m, r, k, _, _ = fixture(); fn(m)
        with self.assertRaises(ValueError): validate(m, r, k)

    def test_missing_markers(self): self.reject_meta(lambda m: m.update(input_phases=[]))
    def test_unknown_schema(self): self.reject_meta(lambda m: m.update(schema=3))
    def test_unknown_protocol(self): self.reject_meta(lambda m: m.update(marker_protocol='guess'))
    def test_bad_recorder(self): self.reject_meta(lambda m: m.update(marker_error='missed first Normal'))
    def test_wrong_first(self): self.reject_meta(lambda m: m['input_phases'][0].update(first_input_frame=4))
    def test_outside_prefix(self): self.reject_meta(lambda m: m['input_phases'][0].update(end_input_frame_exclusive=9))
    def test_short_open_prefix(self): self.reject_meta(lambda m: m.update(seams=8))
    def test_invented_resume(self): self.reject_meta(lambda m: m['input_phases'][0]['marker'].update(first_normal_update=False))
    def test_nonfinite(self): self.reject_meta(lambda m: m['input_phases'][0]['marker'].update(position_bits=[0x7fc00000, 0, 0]))
    def test_unobserved_checkpoint(self):
        self.reject_meta(lambda m: m['input_phases'][0]['checkpoints'].append(dict(kind='map-wait', state=marker(5))))

    def test_actual_pointer_bit(self):
        _, _, canonical, mapping, report = fixture(); actual = copy.deepcopy(canonical)
        data = bytearray(actual[0]['samples'][0]); data[35] ^= 1; actual[0]['samples'] = [bytes(data)]
        with self.assertRaises(ValueError): audit_inputs(canonical, actual, mapping, report)

    def test_comparison_only_shift(self):
        _, _, canonical, mapping, report = fixture(); mapping[0]['canonical_frame'] += 1
        with self.assertRaises(ValueError): audit_inputs(canonical, canonical, mapping, report)

    def test_dropped_first_read(self):
        _, _, canonical, mapping, report = fixture()
        with self.assertRaises(ValueError): audit_inputs(canonical, canonical[1:], mapping, report)

    def test_extra_actual(self):
        _, _, canonical, mapping, report = fixture(); actual = copy.deepcopy(canonical)
        actual.append(dict(bootstrap(0, 120), frame=7, ordinal=0))
        with self.assertRaises(ValueError): audit_inputs(canonical, actual, mapping, report)

    def test_failed_native_not_promoted(self):
        _, _, canonical, mapping, report = fixture(); report['finished'] = False
        with self.assertRaises(ValueError): audit_inputs(canonical, canonical, mapping, report)

    def test_explicit_neutral(self):
        _, _, canonical, mapping, report = fixture(); actual = copy.deepcopy(canonical)
        actual.append(dict(bootstrap(0, 120), frame=7, ordinal=0))
        mapping.append(dict(actual_frame=7, actual_ordinal=0, source='neutral', canonical_frame=None,
                            canonical_ordinal=None, phase=None, channel=0, capacity=120, returned=1, error=0, samples=1))
        self.assertEqual(audit_inputs(canonical, actual, mapping, report)['exact_neutral_reads'], 1)

    def test_gap_cannot_discard_controls(self):
        for at, value in ((3, 1), (94, 1), (99, 1)):
            m, r, k, _, _ = fixture()
            read = dict(bootstrap(0, 120), frame=1, ordinal=0)
            sample = bytearray(read['samples'][0]); sample[at] = value; read['samples'] = [bytes(sample)]
            with self.assertRaises(ValueError): validate(m, r, [read] + k)

    def test_gap_neutral_sensor_and_signed_zero_allowed(self):
        m, r, k, _, _ = fixture()
        read = dict(bootstrap(0, 120), frame=1, ordinal=0)
        sample = bytearray(read['samples'][0]); sample[96] = 128; read['samples'] = [bytes(sample)]
        validate(m, r, [read] + k)

    def test_mod_actions_are_consumed_at_same_game_frame(self):
        _, _, _, mapping, _ = fixture()
        self.assertEqual(audit_actions({(3, 2): (1, 1)}, {(3, 2): (1, 1)}, mapping, 8, 8), 1)
        with self.assertRaises(ValueError): audit_actions({(3, 2): (1, 1)}, {(4, 2): (1, 1)}, mapping, 8, 8)
        with self.assertRaises(ValueError): audit_actions({(1, 2): (1, 0)}, {}, mapping, 8, 8)

    def test_gap_cannot_discard_mod_action(self):
        m, r, k, _, _ = fixture()
        with self.assertRaises(ValueError): validate(m, r, k, {(1, 2): (1, 0)})

    def test_native_accounting_and_marker_binding(self):
        m, _, _, _, report = fixture(); phase = m['input_phases'][0]
        report.update(mode='whole-session', interactive_position_bit_exact=True, marker_protocol=m['marker_protocol'], tape_schema=2, legacy_policy=False)
        report['segments'][0].update(tape_frames=4, rows_compared=5, identity_rows_skipped=0, pointer_rows_compared=4,
            identity_offset_checked=True, identity_offset_passed=True, identity_offset_limit=30, identity_frame_offset=0, play_ready_required=False)
        report['input_phases'] = [dict(id=0, recorded_marker=marker(), actual_marker=marker(), marker_checked=True, marker_passed=True,
            rng_verified=True, rng_readback=99, frame_offset=0, exit_required=False, end_input_frame_exclusive=7,
            checkpoints_required=0, checkpoints_checked=0)]
        audit_native(report, m)
        for field, value in (('pointer_rows_compared', 3), ('rows_compared', 4), ('identity_frame_offset', 31)):
            bad = copy.deepcopy(report); bad['segments'][0][field] = value
            with self.assertRaises(ValueError): audit_native(bad, m)
        bad = copy.deepcopy(report); bad['input_phases'][0]['actual_marker']['position_bits'][0] = 1
        with self.assertRaises(ValueError): audit_native(bad, m)
        bad = copy.deepcopy(report); bad['input_phases'][0]['rng_readback'] = 98
        with self.assertRaises(ValueError): audit_native(bad, m)


if __name__ == '__main__': unittest.main()
