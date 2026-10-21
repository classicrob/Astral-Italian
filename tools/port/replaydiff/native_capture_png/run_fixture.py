#!/usr/bin/env python3
"""Fail closed until the shared owned-process lifecycle API is qualified."""
import argparse
import json


def reuse_gate():
    return dict(status='FAIL_CLOSED',allocation_required=True,spawned=False,
                lock_acquired=False,output_created=False,lifecycle_qualified=False,
                visual_qualification='BLOCKED',qualified_frames=None,
                reason='Shared lifecycle API has no qualified spawn/signal/whole-group cleanup contract. No fixture reuse is authorized.',
                historical_encoder_receipt='checked-png-leaf-r3/receipt.json: 29 native +7 decode/hash controls only',
                historical_runner_revision='a5641ae5b269cff6b5de26ce235813318c5c646d',
                unqualified_shared_api='tools/team/dispatch_preflight.py@abc13027; do not load or use')


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output',required=True,help='reserved; no output is created while gate is closed')
    parser.parse_args(argv)
    print(json.dumps(reuse_gate()))
    return 2


if __name__ == '__main__':
    raise SystemExit(main())
