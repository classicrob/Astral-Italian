"""Stream a validated native KPAD tape without reconstructing sample history."""
import hashlib
from pathlib import Path
import tempfile

from dolphin_collect import ObserverError


def digest(path):
    value = hashlib.sha256()
    with Path(path).open('rb') as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b''):
            value.update(block)
    return value.hexdigest()


class RawSuffix:
    def __init__(self, path, expected_sha256, frames, directory, contract, observed_connection=False):
        self.path = Path(path)
        self.frames = frames
        self.expected_sha256 = expected_sha256
        self.observed_connection = observed_connection
        if digest(self.path) != expected_sha256:
            raise ObserverError('raw input SHA256 mismatch')
        self.receipt = dict(schema_version=1, status='incomplete', path=str(self.path),
                            sha256=expected_sha256, frames=frames, reads=0,
                            origin='first loop after ready anchor; row 0 input affects trace row 1',
                            history='all supplied edges, sample bits and predecessor deltas retained')
        self.receipt.update(
            connection_policy='observed read-only probe' if observed_connection else 'require source connection',
            connection_difference_reads=0, first_connection_difference=None,
            kpd_sha256_scope='re-encoded source, not observed delivery',
            connection_scope='raw suffix only; prior boot connections are scripted synthetic',
            connection_phase='post-KPADReadEx; intra-call timing is not represented by native tape',
            within_read_connection_transitions=0, first_within_read_transition=None)
        previous_tempdir = tempfile.tempdir
        try:
            tempfile.tempdir = str(directory)
            validated = Path(directory) / 'validated-inputs.kpd'
            with validated.open('xb') as stream:
                contract.encode_rows(contract.rows_from_jsonl(self.path), frames, stream)
            self.receipt['kpd_sha256'] = digest(validated)
        finally:
            tempfile.tempdir = previous_tempdir
        if digest(self.path) != expected_sha256:
            raise ObserverError('raw input changed during validation')
        self.rows = iter(contract.rows_from_jsonl(self.path))
        self.next = next(self.rows, None)
        self.completed_frames = 0
        self.closed = False
        self.source_start = None

    def start(self, source_start):
        if self.source_start is not None:
            raise ObserverError('raw input suffix already started')
        self.source_start = source_start
        self.receipt['source_start'] = source_start

    def take(self, source_frame, ordinal, channel, capacity):
        frame = source_frame - self.source_start
        expected = (frame, ordinal, channel, capacity)
        row = self.next
        if row is None or tuple(row[k] for k in ('frame', 'read_ordinal', 'channel', 'requested_length')) != expected:
            raise ObserverError(f'raw input read mismatch at {expected}')
        self.next = next(self.rows, None)
        self.receipt['reads'] += 1
        return dict(row, frame=source_frame)

    def connection(self, row, observed):
        if observed != row['connection']:
            evidence = dict(source_frame=row['frame'], suffix_frame=row['frame'] - self.source_start,
                            read_ordinal=row['read_ordinal'], channel=row['channel'],
                            expected=row['connection'], observed=observed,
                            classification='input-delivery boundary; not gameplay divergence')
            if not self.observed_connection:
                self.receipt['connection_mismatch'] = evidence
                raise ObserverError('actual WPADProbe connection differs from raw tape: ' + str(evidence))
            self.receipt['connection_difference_reads'] += 1
            if self.receipt['first_connection_difference'] is None:
                self.receipt['first_connection_difference'] = evidence
        return dict(row, connection=observed) if self.observed_connection else row

    def end_frame(self, source_frame):
        frame = source_frame - self.source_start
        if frame != self.completed_frames or not 0 <= frame < self.frames:
            raise ObserverError('raw suffix frame accounting mismatch')
        if self.next is not None and self.next['frame'] <= frame:
            raise ObserverError(f'unconsumed raw input at suffix frame {frame}')
        self.completed_frames += 1

    def close(self, complete=False):
        if self.closed:
            return
        self.closed = True
        try:
            if complete:
                if self.source_start is None or self.completed_frames != self.frames or self.next is not None:
                    raise ObserverError('incomplete raw input suffix')
                if digest(self.path) != self.expected_sha256:
                    raise ObserverError('raw input changed during replay')
                self.receipt['status'] = 'captured'
        finally:
            self.rows.close()
