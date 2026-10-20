"""Synthetic contract tests only. These never certify a real gameplay capture."""
import contextlib
import copy
import csv
import hashlib
import io
import json
from pathlib import Path
import tempfile
import unittest
from unittest import mock

import replay_gate as gate
import tracediff as td


def frames(count=3, actors=1, player=True):
    result = []
    for frame in range(count):
        row = dict.fromkeys(gate.frame_columns(), '')
        row.update({field: '0' for field in row if field.endswith('_valid')})
        row.update(frame=str(frame), scene_epoch='0', scene='Game', stage='SyntheticGalaxy', scenario='1', init_state='6',
                   scene_valid='1', player_valid=str(int(player)), actor_count_valid='1', rng_valid='1', animation_valid='1',
                   actor_registered=str(actors), actor_living=str(actors), state_ids='[1,2]', state_flags3='0;1;0',
                   anim_name='SyntheticAnimation', mode='0', dead='0')
        for field in ('movement_lo movement_hi flags14_lo flags14_hi draw flags20 flags24_lo flags24_hi prev_draw hp rng_seed').split():
            row[field] = '0'
        for field in td.FLOATS & row.keys():
            row[field], row[field + '_bits'] = '0', '0x00000000'
        result.append(row)
    return td.Trace(gate.frame_columns(), {(n,): row for n, row in enumerate(result)}, [])


def sidecars(main):
    actors, census = [], []
    for (frame,), main_row in main.rows.items():
        context = {field: main_row[field] for field in gate.CONTEXT}
        count = int(main_row['actor_registered'])
        receipt = dict(context, schema_version='1', ready='1', registered=str(count), living=str(count),
                       emitted=str(count), identity_unresolved='0', identity_ambiguous='0')
        census.append(receipt)
        for index in range(count):
            player = index == 0 and main_row['player_valid'] == '1'
            row = dict.fromkeys(gate.ACTOR_COLUMNS, '')
            row.update(context, schema_version='1', producer_handle=str(index + 1), identity_kind='player' if player else 'link',
                       identity_status='source-key', name_valid='1', name_hex='73796e746865746963', dead='0',
                       link_valid='0' if player else '1', link_zone='-1' if player else '0', link_context='-1' if player else '0',
                       link_id='-1' if player else str(index))
            row['actor_id'] = 'player' if player else f'link:0:0:{index}:' + row['name_hex']
            for field in gate.ACTOR_FLOATS:
                row[field], row[field + '_bits'] = '0', '0x00000000'
            actors.append(row)
    return actors, census


def projection(main, actors=None, census=None):
    actual, receipts = sidecars(main) if actors is None or census is None else (actors, census)
    return gate.validate_actors(main, actual if actors is None else actors, receipts if census is None else census, len(main.rows))


def write_json(path, value):
    path.write_text(json.dumps(value))


def write_csv(path, columns, rows):
    with path.open('w', newline='') as stream:
        writer = csv.DictWriter(stream, fieldnames=columns)
        writer.writeheader()
        writer.writerows(rows)


class GateValidationTests(unittest.TestCase):
    def test_complete_main_schema_and_ordered_state(self):
        trace = frames()
        gate.validate_frames(trace, 3, 'SyntheticGalaxy', 1)
        trace.rows[(1,)]['state_ids'] = '[2,1]'
        report = td.compare(frames(), trace, expected_frames=3)
        self.assertEqual(report['first_divergence']['frame'], 1)
        self.assertIn('state_ids', [d['column'] for d in report['first_divergence']['columns']])

    def test_missing_equal_coverage_is_blocked(self):
        for field in ('scene_valid', 'player_valid', 'rng_valid', 'actor_count_valid', 'animation_valid'):
            with self.subTest(field=field):
                trace = frames()
                trace.rows[(0,)][field] = '0'
                with self.assertRaises(gate.Blocked):
                    gate.validate_frames(trace, 3, 'SyntheticGalaxy', 1)

    def test_no_later_favorable_origin(self):
        trace = frames()
        trace.rows[(0,)]['player_valid'] = '0'
        with self.assertRaisesRegex(gate.Blocked, 'origin is never moved'):
            gate.validate_frames(trace, 3, 'SyntheticGalaxy', 1)

    def test_truncated_and_partial_schema_fail(self):
        for trace in (frames(2), frames()):
            if len(trace.rows) == 3:
                trace.columns = trace.columns[:-1]
            with self.assertRaises(gate.Blocked):
                gate.validate_frames(trace, 3, 'SyntheticGalaxy', 1)

    def test_full_state_types_flags_and_widths(self):
        for field, value in [('state_ids', '[true]'), ('state_ids', 'null'), ('state_ids', '[' + ','.join(['1'] * 65) + ']'),
                             ('state_flags3', '0;2;0'), ('mode', '65536'), ('hp', '-1'), ('dead', '2'),
                             ('pending_state_id', '4294967296'), ('rng_seed', ''), ('x_bits', ''), ('ticks', '0')]:
            with self.subTest(field=field, value=value):
                trace = frames()
                trace.rows[(0,)][field] = value
                with self.assertRaises((gate.Blocked, td.TraceError)):
                    gate.validate_frames(trace, 3, 'SyntheticGalaxy', 1)

    def test_actor_identical_and_local_handles_not_cross_run_keys(self):
        trace = frames()
        a, c = sidecars(trace)
        b = copy.deepcopy(a)
        for row in b:
            row['producer_handle'] = '900'
        left, right = projection(trace, a, c), projection(trace, b, c)
        report = td.compare(left, right, actor_keys=('record_kind', 'record_id'), policy=gate.actor_policy(left.columns), expected_frames=3)
        self.assertEqual(report['exit_code'], 0, report['invalid_errors'])
        self.assertNotIn('producer_handle', ' '.join(left.columns))

    def test_actual_zero_census_is_explicit_not_synthetic_actor(self):
        trace = frames(actors=0, player=False)
        records = projection(trace)
        self.assertEqual(len(records.rows), 3)
        self.assertEqual({row['record_kind'] for row in records.rows.values()}, {'census'})
        report = td.compare(records, records, actor_keys=('record_kind', 'record_id'), policy=gate.actor_policy(records.columns), expected_frames=3)
        self.assertEqual(report['exit_code'], 0, report['invalid_errors'])
        # Whole gameplay admission still requires observed player/state coverage.
        with self.assertRaises(gate.Blocked):
            gate.validate_frames(trace, 3, 'SyntheticGalaxy', 1)

    def test_nonready_receipt_preserved_with_blank_counts(self):
        trace = frames(actors=0, player=False)
        for row in trace.rows.values():
            row.update(scene='Logo', init_state='0', actor_count_valid='0', actor_registered='', actor_living='')
        receipts = sidecars(frames(actors=0, player=False))[1]
        for row, main in zip(receipts, trace.rows.values()):
            row.update({field: main[field] for field in gate.CONTEXT}, ready='0', registered='', living='')
        records = projection(trace, [], receipts)
        report = td.compare(records, records, actor_keys=('record_kind', 'record_id'), policy=gate.actor_policy(records.columns), expected_frames=3)
        self.assertEqual(report['exit_code'], 0, report['invalid_errors'])

    def test_missing_duplicate_or_context_census_blocked(self):
        for change in ('missing', 'duplicate', 'context'):
            with self.subTest(change=change):
                trace = frames()
                actors, census = sidecars(trace)
                if change == 'missing':
                    census.pop()
                elif change == 'duplicate':
                    census[1] = copy.deepcopy(census[0])
                else:
                    census[1]['stage'] = 'Wrong'
                with self.assertRaises(gate.Blocked):
                    projection(trace, actors, census)

    def test_false_complete_zero_census_cannot_hide_actor(self):
        trace = frames()
        actors, census = sidecars(trace)
        census[0].update(registered='0', living='0', emitted='0')
        with self.assertRaises(gate.Blocked):
            projection(trace, actors, census)
        with self.assertRaises(gate.Blocked):
            projection(trace, actors[1:], census)

    def test_wrong_player_bits_and_death_fail_within_producer(self):
        for field, value in [('x_bits', '0x80000000'), ('dead', '1')]:
            with self.subTest(field=field):
                trace = frames()
                actors, census = sidecars(trace)
                actors[0][field] = value
                if field == 'x_bits':
                    actors[0]['x'] = '-0'
                else:
                    census[0]['living'] = trace.rows[(0,)]['actor_living'] = '0'
                with self.assertRaisesRegex(gate.Blocked, 'same-producer'):
                    projection(trace, actors, census)

    def test_missing_or_duplicate_typed_player_not_name_fallback(self):
        trace = frames(actors=2)
        actors, census = sidecars(trace)
        actors[0].update(identity_kind='link', link_valid='1', link_zone='0', link_context='0', link_id='0',
                         actor_id='link:0:0:0:' + actors[0]['name_hex'])
        with self.assertRaisesRegex(gate.Blocked, 'exactly one typed player'):
            projection(trace, actors, census)
        actors, census = sidecars(trace)
        actors[1].update(identity_kind='player', actor_id='player')
        with self.assertRaisesRegex(gate.Blocked, 'duplicate'):
            projection(trace, actors, census)

    def test_counts_living_and_all_registered_entries(self):
        for field in ('registered', 'living', 'emitted'):
            with self.subTest(field=field):
                trace = frames()
                actors, census = sidecars(trace)
                census[0][field] = '0'
                with self.assertRaises(gate.Blocked):
                    projection(trace, actors, census)

    def test_every_non_source_key_status_is_hold(self):
        for status in ('unresolved', 'ambiguous', 'key-change', 'key-reuse', ''):
            with self.subTest(status=status):
                trace = frames()
                actors, census = sidecars(trace)
                actors[0]['identity_status'] = status
                with self.assertRaisesRegex(gate.Blocked, 'HOLD'):
                    projection(trace, actors, census)

    def test_lifecycle_key_reuse_and_key_change_are_hold(self):
        trace = frames(actors=2)
        for kind in ('change', 'reuse', 'absence'):
            with self.subTest(kind=kind):
                actors, census = sidecars(trace)
                if kind == 'change':
                    actors[3].update(link_id='3', actor_id='link:0:0:3:' + actors[3]['name_hex'])
                elif kind == 'reuse':
                    actors[3]['producer_handle'] = '99'
                else:
                    actors.pop(3)
                    census[1].update(registered='1', living='1', emitted='1')
                    trace.rows[(1,)].update(actor_registered='1', actor_living='1')
                with self.assertRaises(gate.Blocked):
                    projection(trace, actors, census)
                trace = frames(actors=2)

    def test_signed_links_name_bytes_and_float_integrity(self):
        for changes in (dict(link_valid='0'), dict(link_zone='32768'), dict(name_hex='AA'), dict(name_hex='00'),
                        dict(name_valid='0'), dict(x='NaN'), dict(x='1'), dict(scale_x_bits='0x7f800000')):
            with self.subTest(changes=changes):
                trace = frames(actors=2)
                actors, census = sidecars(trace)
                actors[1].update(changes)
                with self.assertRaises((gate.Blocked, td.TraceError)):
                    projection(trace, actors, census)

    def test_first_actor_difference_and_preceding_history(self):
        trace = frames(130, actors=2)
        a, census = sidecars(trace)
        b = copy.deepcopy(a)
        b[2 * 125 + 1].update(x='2', x_bits='0x40000000')
        left, right = projection(trace, a, census), projection(trace, b, census)
        report = td.compare(left, right, actor_keys=('record_kind', 'record_id'), policy=gate.actor_policy(left.columns), history=120, expected_frames=130)
        self.assertEqual(report['exit_code'], 1)
        self.assertEqual(report['first_divergence']['frame'], 125)
        self.assertEqual(report['first_divergence']['record_kind'], 'actor')
        self.assertEqual(len(report['history']), 120)
        self.assertEqual(report['history'][0]['frame'], 5)

    def test_unmatched_actor_is_divergence_not_dropped(self):
        left = projection(frames(actors=1))
        right = projection(frames(actors=2))
        report = td.compare(left, right, actor_keys=('record_kind', 'record_id'), policy=gate.actor_policy(left.columns), expected_frames=3)
        self.assertEqual(report['exit_code'], 1)
        self.assertGreater(report['summary']['diverging_rows'], 0)
        self.assertTrue(any(row['column'] == '<row>' for row in report['first_divergence']['columns']))

    def test_duplicate_json_is_blocked(self):
        with self.assertRaises(gate.Blocked):
            gate.json_value('{"status":"complete","status":"invalid"}')


    def test_name_storage_bound_includes_nul(self):
        for length in (1023, 1024):
            trace = frames()
            actors, census = sidecars(trace)
            for row in actors:
                row['name_hex'] = '61' * length
            if length == 1023:
                projection(trace, actors, census)
            else:
                with self.assertRaisesRegex(gate.Blocked, 'name_hex'):
                    projection(trace, actors, census)


    def test_first_observed_handle_order_is_not_registration_identity(self):
        trace = frames(actors=2)
        actors, census = sidecars(trace)
        for row in actors:
            row['producer_handle'] = '100' if row['identity_kind'] == 'player' else '1'
        projection(trace, actors, census)

    def test_new_handle_cannot_decrease_below_any_prior_frame(self):
        trace = frames(actors=2)
        actors, census = sidecars(trace)
        for row in actors:
            if row['identity_kind'] == 'link':
                row['producer_handle'] = '100' if row['frame'] == '0' else '2'
                if row['frame'] != '0':
                    row.update(link_id='9', actor_id='link:0:0:9:' + row['name_hex'])
        with self.assertRaisesRegex(gate.Blocked, 'monotonic serial'):
            projection(trace, actors, census)

    def test_serials_are_global_while_identity_history_resets_per_epoch(self):
        for kind in ('valid', 'reuse', 'lower'):
            with self.subTest(kind=kind):
                trace = frames(actors=2)
                actors, census = sidecars(trace)
                actors[1]['producer_handle'] = '100'
                for frame in (1, 2):
                    trace.rows[(frame,)]['scene_epoch'] = census[frame]['scene_epoch'] = '1'
                    for index in (0, 1):
                        row = actors[2 * frame + index]
                        row['scene_epoch'] = '1'
                        row['producer_handle'] = str(101 + index) if kind == 'valid' else ('1' if kind == 'reuse' else str(2 + index))
                if kind == 'valid':
                    projection(trace, actors, census)  # same keys, fresh serials in new epoch
                else:
                    with self.assertRaises(gate.Blocked):
                        projection(trace, actors, census)


class FixtureQualificationTests(unittest.TestCase):
    """Invented metadata below is adversarial input, not producer evidence."""
    def setUp(self):
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        self.root = Path(temp.name)
        self.fixture = self.root / 'fixture.json'

    def check_blocked(self, fixture):
        write_json(self.fixture, fixture)
        result = gate.evaluate(self.fixture)
        self.assertEqual((result['status'], result['exit_code']), ('BLOCKED', 2), result)
        self.assertIsNone(result['measured_diverging_frames'])
        self.assertIsNone(result['measured_diverging_actor_records'])
        self.assertIsNone(result['first_divergence'])
        self.assertIsNone(result['measured_diverging_records'])
        self.assertIn('\nstatus: BLOCKED\n', gate.markdown(result))
        return result

    def spec(self, path):
        return dict(path=str(path), sha256=td.sha256(path))

    def build_adversarial_fixture(self, count=3, actor_count=1):
        # Deliberately fake producer records let negative tests reach later gates.
        # Admission remains explicitly synthetic and can NEVER pass evaluate().
        fixture = dict(schema_version=1, gate_kind='native-vs-dolphin-gameplay', synthetic=False,
                       stage='SyntheticGalaxy', scenario=1, expected_frames=count)
        for side in gate.SIDES:
            folder = self.root / side
            folder.mkdir()
            main = frames(count, actors=actor_count)
            actors, census = sidecars(main)
            path = folder / 'trace.csv'
            write_csv(path, main.columns, list(main.rows.values()))
            write_csv(Path(str(path) + '.actors.csv'), gate.ACTOR_COLUMNS, actors)
            write_csv(Path(str(path) + '.actor-frames.csv'), gate.CENSUS_COLUMNS, census)
            origin = dict(schema_version=1, phase='post-endFrame', stage='SyntheticGalaxy', scenario=1,
                          source_frame=0, source_scene_epoch=0, source_ticks=0, source_vi_count=0,
                          rng_valid=True, rng_seed=0, rng_forced=False)
            write_json(Path(str(path) + '.sync.json'), origin)
            executable, producer, inputs = folder / 'not-an-executable', folder / 'not-a-producer', folder / 'inputs.jsonl'
            executable.write_text('synthetic NONEXECUTABLE fixture')
            producer.write_text('synthetic NONEXECUTABLE collector')
            input_count = count - 1 if side == 'native' else count
            inputs.write_text(''.join(json.dumps(row) + '\n' for row in gate.input_contract().generate(frames=input_count)))
            policy = dict(tick_hz=60750000, rate_numerator=60000, rate_denominator=1001, epoch_ticks=0,
                          fpcr_fz=True, audio_output='off', audio_processing='virtual-ai-dsp')
            manifest = dict(schema_version=1, frames=count, input_frames=input_count, command=['not-an-executable'], timing_policy=policy)
            paths = dict(frames=path, actors=Path(str(path) + '.actors.csv'), census=Path(str(path) + '.actor-frames.csv'),
                         sync=Path(str(path) + '.sync.json'), manifest=folder / 'manifest.json',
                         executable=executable, producer=producer, input=inputs)
            if side == 'native':
                manifest.update(status='complete', exit_status=0, trace_frames=count, columns=main.columns,
                                inputs_from_sync=True, input_origin='first loop after ready anchor',
                                executable_sha256=td.sha256(executable), input_sha256=td.sha256(inputs), sync_origin=origin)
                paths['completion'] = Path(str(path) + '.complete')
                paths['completion'].write_text(str(count) + '\n')
                paths['timing'] = Path(str(path) + '.timing.csv')
                write_csv(paths['timing'], ['frame', 'vi_count', 'ticks'], [dict(frame=i, vi_count=i, ticks=i) for i in range(count)])
            else:
                manifest.update(status='captured; repeat qualification required', requested_frames=count, source_frames=count,
                                frame_seam='804be280', caller_lr='804b6ac4', collector_sha256=td.sha256(producer),
                                dolphin_sha256=td.sha256(executable), columns_sha256=hashlib.sha256(','.join(main.columns).encode()).hexdigest(),
                                sync=origin, input_method='scripted filtered KPAD injection', input_reads=4 * count,
                                input_export=dict(schema_version=1, status='captured', path=inputs.name, sha256=td.sha256(inputs),
                                                  bytes=inputs.stat().st_size, read_count=4 * count))
            manifest['actors'] = dict(schema_version=1, status='captured', frames=count, actor_rows=count * actor_count, identity_holds=0,
                                      files=[dict(path=paths[field].name, sha256=td.sha256(paths[field])) for field in ('actors', 'census')])
            write_json(paths['manifest'], manifest)
            fixture[side] = {key: self.spec(value) for key, value in paths.items()}
        admission = self.root / 'admission.json'
        write_json(admission, dict(schema_version=1, decision='admit-gameplay', synthetic=True))
        fixture['admission'] = self.spec(admission)
        return fixture

    def mutate_artifact(self, fixture, side, field, mutate):
        path = Path(fixture[side][field]['path'])
        value = json.loads(path.read_text())
        mutate(value)
        write_json(path, value)
        fixture[side][field] = self.spec(path)

    def test_missing_input_produces_blocked_report_files_not_zero(self):
        output = self.root / 'new-output'
        with contextlib.redirect_stdout(io.StringIO()):
            result = gate.main(['--fixture', str(self.fixture), '--out', str(output)])
        self.assertEqual(result, 2)
        report = json.loads((output / 'gate.json').read_text())
        self.assertIsNone(report['measured_diverging_frames'])
        self.assertIn('BLOCKED', (output / 'gate.md').read_text())
        with self.assertRaises(SystemExit), contextlib.redirect_stderr(io.StringIO()):
            gate.main(['--fixture', str(self.fixture), '--out', str(output)])

    def test_synthetic_fixture_rejected_before_comparison(self):
        result = self.check_blocked(dict(schema_version=1, gate_kind='native-vs-dolphin-gameplay', synthetic=True))
        self.assertIn('synthetic', result['errors'][0])

    def test_role_label_or_dolphin_repeat_only_not_a_gate(self):
        for value in (dict(gate_kind='dolphin-vs-dolphin'), dict(gate_kind='native-vs-dolphin-gameplay', role='native')):
            result = self.check_blocked(dict(schema_version=1, synthetic=False, stage='SyntheticGalaxy', scenario=1, expected_frames=3, **value))
            self.assertTrue(result['errors'])

    def test_admitted_but_unavailable_provider_is_blocked(self):
        fixture = self.build_adversarial_fixture()
        fixture['native'].pop('actors')
        self.assertIn('capture artifacts', self.check_blocked(fixture)['errors'][0])

    def test_full_synthetic_pair_and_repeat_still_cannot_qualify(self):
        fixture = self.build_adversarial_fixture()
        report = self.check_blocked(fixture)
        self.assertIn('synthetic evidence', report['errors'][0])

    def test_prepared_diagnostic_and_incomplete_producers_are_blocked(self):
        for side, status in [('native', 'prepared'), ('native', 'diagnostic-complete'), ('dolphin', 'incomplete')]:
            with self.subTest(side=side, status=status):
                folder = tempfile.TemporaryDirectory()
                self.addCleanup(folder.cleanup)
                self.root = Path(folder.name)
                self.fixture = self.root / 'fixture.json'
                fixture = self.build_adversarial_fixture()
                self.mutate_artifact(fixture, side, 'manifest', lambda record: record.update(status=status))
                self.assertIn('complete', self.check_blocked(fixture)['errors'][0])

    def test_completion_marker_actual_count_required(self):
        fixture = self.build_adversarial_fixture()
        marker = Path(fixture['native']['completion']['path'])
        marker.write_text('2')
        fixture['native']['completion'] = self.spec(marker)
        self.assertIn('completion marker', self.check_blocked(fixture)['errors'][0])

    def test_hash_mismatch_and_partial_capture_rejected(self):
        fixture = self.build_adversarial_fixture()
        original = fixture['native']['actors']['sha256']
        fixture['native']['actors']['sha256'] = '0' * 64
        self.assertIn('hash mismatch', self.check_blocked(fixture)['errors'][0])
        fixture['native']['actors']['sha256'] = original
        Path(fixture['native']['actors']['path'] + '.partial').write_text('not complete')
        self.assertIn('partial', self.check_blocked(fixture)['errors'][0])

    def test_unknown_timing_policy_is_never_filled_in(self):
        fixture = self.build_adversarial_fixture()
        self.mutate_artifact(fixture, 'dolphin', 'manifest', lambda record: record['timing_policy'].update(rate_numerator=None))
        self.assertIn('timing_policy', self.check_blocked(fixture)['errors'][0])

    def test_input_actual_reads_not_full_prefix_hash_or_role(self):
        fixture = self.build_adversarial_fixture()
        path = Path(fixture['dolphin']['input']['path'])
        rows = [json.loads(line) for line in path.read_text().splitlines()]
        rows[4]['samples'][0]['hold'] = 8
        path.write_text(''.join(json.dumps(row) + '\n' for row in rows))
        fixture['dolphin']['input'] = self.spec(path)
        self.mutate_artifact(fixture, 'dolphin', 'manifest', lambda record: record['input_export'].update(sha256=td.sha256(path), bytes=path.stat().st_size))
        self.assertIn('actual post-anchor input reads differ', self.check_blocked(fixture)['errors'][0])

    def test_input_window_missing_channel_or_loop_is_blocked(self):
        fixture = self.build_adversarial_fixture()
        path = Path(fixture['dolphin']['input']['path'])
        rows = [json.loads(line) for line in path.read_text().splitlines() if json.loads(line)['frame'] != 2]
        path.write_text(''.join(json.dumps(row) + '\n' for row in rows))
        fixture['dolphin']['input'] = self.spec(path)
        self.mutate_artifact(fixture, 'dolphin', 'manifest', lambda record: (record.update(input_reads=8), record['input_export'].update(sha256=td.sha256(path), bytes=path.stat().st_size, read_count=8)))
        self.assertIn('EVERY loop', self.check_blocked(fixture)['errors'][0])

    def test_zero_census_and_wrong_player_sidecar_fail_whole_gate(self):
        fixture = self.build_adversarial_fixture()
        path = Path(fixture['native']['actors']['path'])
        rows = gate.table(path, gate.ACTOR_COLUMNS)
        rows[0].update(x='-0', x_bits='0x80000000')
        write_csv(path, gate.ACTOR_COLUMNS, rows)
        fixture['native']['actors'] = self.spec(path)
        self.mutate_artifact(fixture, 'native', 'manifest', lambda record: record['actors']['files'][0].update(sha256=td.sha256(path)))
        self.assertIn('same-producer player', self.check_blocked(fixture)['errors'][0])


    def rebind_actor_fixture(self, fixture, side, change):
        # Test-only deliberate re-binding of planted capture bytes.
        path = Path(fixture[side]['actors']['path'])
        rows = gate.table(path, gate.ACTOR_COLUMNS)
        change(rows)
        write_csv(path, gate.ACTOR_COLUMNS, rows)
        fixture[side]['actors'] = self.spec(path)
        self.mutate_artifact(fixture, side, 'manifest', lambda record: record['actors']['files'][0].update(sha256=td.sha256(path)))

    def test_mocked_admission_orchestration_identical_mechanics_only(self):
        fixture = self.build_adversarial_fixture()
        write_json(self.fixture, fixture)
        # No runtime bypass exists; mock is limited to this synthetic unit test.
        with mock.patch.object(gate, 'admission', return_value=dict(reviewer='SYNTHETIC TEST DOUBLE', rationale='mechanics only')):
            report = gate.evaluate(self.fixture)
        self.assertEqual((report['status'], report['exit_code']), ('PASS', 0), report['errors'])
        self.assertEqual(report['measured_diverging_frames'], 0)
        self.assertEqual(report['measured_diverging_actor_records'], 0)
        self.assertIsNone(report['first_divergence'])
        self.assertEqual(report['prehistory'], [])
        self.assertEqual(report['measured_diverging_records'], 0)
        self.assertIn('\nstatus: PASS\n', gate.markdown(report))

    def test_mocked_admission_first_actor125_and_full_prehistory_mechanics(self):
        fixture = self.build_adversarial_fixture(count=130, actor_count=2)
        def poison(rows):
            rows[2 * 125 + 1].update(x='2', x_bits='0x40000000')
        self.rebind_actor_fixture(fixture, 'native', poison)
        write_json(self.fixture, fixture)
        with mock.patch.object(gate, 'admission', return_value=dict(reviewer='SYNTHETIC TEST DOUBLE', rationale='mechanics only')):
            report = gate.evaluate(self.fixture)
        self.assertEqual((report['status'], report['exit_code']), ('DIVERGED', 1), report['errors'])
        self.assertEqual(report['measured_diverging_frames'], 0)
        self.assertEqual(report['measured_diverging_actor_records'], 1)
        self.assertEqual(report['measured_diverging_records'], 1)
        self.assertIn('\nstatus: 1 divergences\n', gate.markdown(report))
        self.assertEqual(report['first_divergence']['frame'], 125)
        self.assertEqual(report['first_divergence']['kind'], 'actor')
        self.assertEqual(len(report['prehistory']), 120)
        self.assertEqual((report['prehistory'][0]['frame'], report['prehistory'][-1]['frame']), (5, 124))
        self.assertEqual(len(report['prehistory'][0]['native']['records']), 3)

    def test_mocked_admission_repeat_difference_is_diagnostic_only(self):
        fixture = self.build_adversarial_fixture(actor_count=2)
        self.rebind_actor_fixture(fixture, 'dolphin_repeat', lambda rows: rows[1].update(x='0.00000011920928955078125', x_bits='0x34000000'))
        write_json(self.fixture, fixture)
        with mock.patch.object(gate, 'admission', return_value=dict(reviewer='SYNTHETIC TEST DOUBLE', rationale='mechanics only')):
            report = gate.evaluate(self.fixture)
        self.assertEqual(report['status'], 'BLOCKED')
        self.assertIsNone(report['measured_diverging_actor_records'])
        repeat = report['diagnostics']['repeat_qualification']['actors']
        self.assertEqual(repeat['exit_code'], 0)  # Within cross-run tolerance, but repeat must also preserve bits.
        self.assertIsNotNone(repeat['first_bit_difference'])

    def test_missing_actor_receipt_blocks_plausible_complete_files(self):
        fixture = self.build_adversarial_fixture()
        self.mutate_artifact(fixture, 'native', 'manifest', lambda record: record.pop('actors'))
        self.assertIn('receipt is unavailable', self.check_blocked(fixture)['errors'][0])

    def test_actor_receipt_counts_status_hash_and_unique_files(self):
        for change in (lambda record: record['actors'].update(status='incomplete'),
                       lambda record: record['actors'].update(frames=2),
                       lambda record: record['actors'].update(actor_rows=0),
                       lambda record: record['actors'].update(identity_holds=1),
                       lambda record: record['actors']['files'][0].update(sha256='0' * 64),
                       lambda record: record['actors']['files'].__setitem__(1, record['actors']['files'][0])):
            with self.subTest(change=change):
                with tempfile.TemporaryDirectory() as folder:
                    self.root, self.fixture = Path(folder), Path(folder) / 'fixture.json'
                    fixture = self.build_adversarial_fixture()
                    self.mutate_artifact(fixture, 'dolphin', 'manifest', change)
                    self.assertIn('actor', self.check_blocked(fixture)['errors'][0])

    def test_failed_sync_report_is_retained_as_diagnostic(self):
        fixture = self.build_adversarial_fixture()
        self.mutate_artifact(fixture, 'dolphin', 'manifest', lambda record: record['timing_policy'].update(epoch_ticks=5))
        report = self.check_blocked(fixture)
        self.assertFalse(report['diagnostics']['synchronization']['native/dolphin']['compatible'])
        self.assertIsNone(report['measured_diverging_frames'])



    def test_observed_export_uses_actual_source_frames_not_missing_input_frames(self):
        fixture = self.build_adversarial_fixture()
        for side in ('dolphin', 'dolphin_repeat'):
            self.mutate_artifact(fixture, side, 'manifest', lambda record: (
                record.update(input_method='read-only original KPADReadEx and verified WPADProbe-equivalent snapshot'),
                record.pop('input_frames')))
        self.assertIn('synthetic evidence', self.check_blocked(fixture)['errors'][0])

    def test_export_source_loop_length_and_native_suffix_mapping_are_explicit(self):
        fixture = self.build_adversarial_fixture()
        self.mutate_artifact(fixture, 'native', 'manifest', lambda record: record.pop('inputs_from_sync'))
        self.assertIn('explicit committed inputs-from-sync', self.check_blocked(fixture)['errors'][0])
        self.mutate_artifact(fixture, 'native', 'manifest', lambda record: record.update(inputs_from_sync=True, input_source_start=900))
        self.assertIn('input source start', self.check_blocked(fixture)['errors'][0])
        self.mutate_artifact(fixture, 'native', 'manifest', lambda record: record.pop('input_source_start'))
        self.mutate_artifact(fixture, 'dolphin', 'manifest', lambda record: record.update(source_frames=2))
        self.assertIn('source-loop length', self.check_blocked(fixture)['errors'][0])

    def test_missing_or_false_input_export_receipt_is_not_script_provenance(self):
        fixture = self.build_adversarial_fixture()
        self.mutate_artifact(fixture, 'dolphin', 'manifest', lambda record: record.pop('input_export'))
        self.assertIn('exported-input receipt', self.check_blocked(fixture)['errors'][0])
        self.mutate_artifact(fixture, 'dolphin', 'manifest', lambda record: record.update(
            input_export=dict(schema_version=1,status='captured',path='intent.json',sha256='0'*64,bytes=1,read_count=12)))
        self.assertIn('exported input path/hash', self.check_blocked(fixture)['errors'][0])



    def test_mocked_admission_changed_after_hash_is_blocked(self):
        fixture = self.build_adversarial_fixture()
        write_json(self.fixture, fixture)
        original_compare = gate.compare_pair
        def mutate_after_compare(*args):
            result = original_compare(*args)
            Path(fixture['native']['producer']['path']).write_text('SYNTHETIC concurrent mutation')
            return result
        with mock.patch.object(gate, 'admission', return_value=dict(reviewer='SYNTHETIC TEST DOUBLE', rationale='mechanics only')), \
                mock.patch.object(gate, 'compare_pair', side_effect=mutate_after_compare):
            report = gate.evaluate(self.fixture)
        self.assertEqual(report['status'], 'BLOCKED')
        self.assertIsNone(report['measured_diverging_records'])
        self.assertIn('input changed during gate', report['errors'][0])

    def test_mocked_admission_record_count_is_not_unique_bug_count(self):
        fixture = self.build_adversarial_fixture()
        path = Path(fixture['native']['frames']['path'])
        trace = td.read_trace(path)
        trace.rows[(1,)]['rng_seed'] = '1'
        write_csv(path, trace.columns, list(trace.rows.values()))
        fixture['native']['frames'] = self.spec(path)
        write_json(self.fixture, fixture)
        with mock.patch.object(gate, 'admission', return_value=dict(reviewer='SYNTHETIC TEST DOUBLE', rationale='mechanics only')):
            report = gate.evaluate(self.fixture)
        self.assertEqual(report['status'], 'DIVERGED', report['errors'])
        self.assertEqual(report['measured_diverging_frames'], 1)
        self.assertEqual(report['measured_diverging_actor_records'], 0)
        self.assertEqual(report['measured_diverging_records'], 1)
        self.assertIn('\nstatus: 1 divergences\n', gate.markdown(report))


    def install_automatic_suffix(self, fixture, side='dolphin'):
        path = Path(fixture[side]['manifest']['path']).parent / 'sync-input-script.json'
        write_json(path, dict(version=1, frames=fixture['expected_frames'] - 1, segments=[]))
        self.mutate_artifact(fixture, side, 'manifest', lambda record: record.update(
            live_input_revisions=[dict(suffix_source_start=1)], sync_input_source_start=1,
            sync_input_script_sha256=td.sha256(path)))
        return path

    def test_automatic_suffix_is_actual_hash_bound_admission_artifact(self):
        fixture = self.build_adversarial_fixture()
        for side in ('dolphin', 'dolphin_repeat'):
            self.install_automatic_suffix(fixture, side)
        write_json(self.fixture, fixture)
        def mocked_admission(_fixture, _root, pins):
            for side in ('dolphin', 'dolphin_repeat'):
                script = Path(fixture[side]['manifest']['path']).parent / 'sync-input-script.json'
                self.assertEqual(pins[side + '.sync_input_script'], dict(path=str(script.resolve()), sha256=td.sha256(script)))
            return dict(reviewer='SYNTHETIC TEST DOUBLE', rationale='mechanics only')
        with mock.patch.object(gate, 'admission', side_effect=mocked_admission):
            report = gate.evaluate(self.fixture)
        self.assertEqual(report['status'], 'PASS', report['errors'])  # Test mechanics, never real qualification.

    def test_automatic_suffix_does_not_bypass_independent_admission(self):
        fixture = self.build_adversarial_fixture()
        self.install_automatic_suffix(fixture)
        report = self.check_blocked(fixture)
        self.assertIn('synthetic evidence', report['errors'][0])
        self.assertIn('dolphin.sync_input_script', report['inputpins'])

    def test_real_mixed_duplicate_or_unknown_revisions_still_block(self):
        auto = dict(suffix_source_start=1)
        edited = dict(first_future_frame=2, sha256='0' * 64)
        for revisions in ([edited], [auto, edited], [auto, auto], [dict(auto, extra=True)],
                          [dict(unknown=1)], [1], None, {}, True):
            with self.subTest(revisions=revisions), tempfile.TemporaryDirectory() as folder:
                self.root, self.fixture = Path(folder), Path(folder) / 'fixture.json'
                fixture = self.build_adversarial_fixture()
                self.install_automatic_suffix(fixture)
                self.mutate_artifact(fixture, 'dolphin', 'manifest', lambda record: record.update(live_input_revisions=revisions))
                self.check_blocked(fixture)

    def test_automatic_suffix_requires_exact_strict_source_clock(self):
        for field in ('marker', 'manifest'):
            for value in (0, 2, True, '1'):
                with self.subTest(field=field, value=value), tempfile.TemporaryDirectory() as folder:
                    self.root, self.fixture = Path(folder), Path(folder) / 'fixture.json'
                    fixture = self.build_adversarial_fixture()
                    self.install_automatic_suffix(fixture)
                    def change(record):
                        if field == 'marker':
                            record['live_input_revisions'][0]['suffix_source_start'] = value
                        else:
                            record['sync_input_source_start'] = value
                    self.mutate_artifact(fixture, 'dolphin', 'manifest', change)
                    self.check_blocked(fixture)

    def test_automatic_suffix_requires_actual_copy_and_declared_hash(self):
        for mode in ('missing', 'stale', 'undeclared', 'invalid-hash'):
            with self.subTest(mode=mode), tempfile.TemporaryDirectory() as folder:
                self.root, self.fixture = Path(folder), Path(folder) / 'fixture.json'
                fixture = self.build_adversarial_fixture()
                path = self.install_automatic_suffix(fixture)
                if mode == 'missing':
                    path.unlink()
                elif mode == 'stale':
                    path.write_text('SYNTHETIC stale copy')
                elif mode == 'undeclared':
                    self.mutate_artifact(fixture, 'dolphin', 'manifest', lambda record: record.pop('sync_input_script_sha256'))
                else:
                    self.mutate_artifact(fixture, 'dolphin', 'manifest', lambda record: record.update(sync_input_script_sha256='not-a-hash'))
                self.check_blocked(fixture)

    def test_suffix_metadata_requires_marker_and_scripted_producer(self):
        fixture = self.build_adversarial_fixture()
        self.install_automatic_suffix(fixture)
        self.mutate_artifact(fixture, 'dolphin', 'manifest', lambda record: record.update(live_input_revisions=[]))
        self.assertIn('missing its exact revision marker', self.check_blocked(fixture)['errors'][0])
        self.mutate_artifact(fixture, 'dolphin', 'manifest', lambda record: record.update(
            live_input_revisions=[dict(suffix_source_start=1)],
            input_method='read-only original KPADReadEx and verified WPADProbe-equivalent snapshot'))
        self.assertIn('live-edited', self.check_blocked(fixture)['errors'][0])

    def test_automatic_suffix_still_compares_actual_input_records(self):
        fixture = self.build_adversarial_fixture()
        self.install_automatic_suffix(fixture)
        path = Path(fixture['dolphin']['input']['path'])
        rows = [json.loads(line) for line in path.read_text().splitlines()]
        rows[4]['samples'][0]['hold'] = 8
        path.write_text(''.join(json.dumps(row) + '\n' for row in rows))
        fixture['dolphin']['input'] = self.spec(path)
        self.mutate_artifact(fixture, 'dolphin', 'manifest', lambda record: record['input_export'].update(
            sha256=td.sha256(path), bytes=path.stat().st_size))
        self.assertIn('actual post-anchor input reads differ', self.check_blocked(fixture)['errors'][0])


class AdmissionContractTests(unittest.TestCase):
    def setUp(self):
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        self.root = Path(temp.name)
        self.artifact = self.root / 'synthetic-reviewed-input'
        self.artifact.write_text('synthetic unit-test artifact, not gameplay evidence')
        self.evidence = self.root / 'synthetic-review'
        self.evidence.write_text('synthetic manual-review contract test')
        self.pins = dict(example=dict(path=str(self.artifact), sha256=td.sha256(self.artifact)))
        self.record = dict(schema_version=1, decision='admit-gameplay', synthetic=False,
                           reviewer='SYNTHETIC UNIT TEST', rationale='helper mechanics only; never gate evidence',
                           bindings=dict(example=td.sha256(self.artifact)),
                           coverage={field: dict(path=self.evidence.name, sha256=td.sha256(self.evidence)) for field in gate.COVERAGE})

    def check(self):
        path = self.root / 'admission.json'
        write_json(path, self.record)
        return gate.admission(dict(admission=dict(path=path.name, sha256=td.sha256(path))), self.root, self.pins)

    def test_admission_contract_requires_bound_artifacts_and_review_evidence(self):
        record = self.check()
        self.assertEqual(record['reviewer'], 'SYNTHETIC UNIT TEST')
        self.assertEqual(len(self.pins), 8)

    def test_stale_or_partial_bindings_rejected(self):
        for bindings in ({}, dict(example='0' * 64), dict(example=td.sha256(self.artifact), unrelated='0' * 64)):
            with self.subTest(bindings=bindings):
                self.record['bindings'] = bindings
                with self.assertRaisesRegex(gate.Blocked, 'bind EVERY'):
                    self.check()

    def test_coverage_not_a_role_label_or_list(self):
        for coverage in ({}, list(gate.COVERAGE), {'role': 'native'}):
            with self.subTest(coverage=coverage):
                self.record['coverage'] = coverage
                with self.assertRaisesRegex(gate.Blocked, 'coverage review'):
                    self.check()

    def test_empty_review_and_stale_evidence_cannot_admit(self):
        self.evidence.write_text('')
        with self.assertRaisesRegex(gate.Blocked, 'hash mismatch'):
            self.check()
        for spec in self.record['coverage'].values():
            spec['sha256'] = td.sha256(self.evidence)
        with self.assertRaisesRegex(gate.Blocked, 'empty admission evidence'):
            self.check()


if __name__ == '__main__':
    unittest.main()
