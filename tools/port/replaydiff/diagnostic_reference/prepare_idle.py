"""Materialize the fixed 616-frame neutral suffix for the proposed diagnostic."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--contract', type=Path, required=True)
    p.add_argument('--contract-sha256', required=True)
    p.add_argument('--output', type=Path, required=True)
    a = p.parse_args()
    if hashlib.sha256(a.contract.read_bytes()).hexdigest() != a.contract_sha256:
        raise ValueError('contract changed')
    spec = importlib.util.spec_from_file_location('idle_contract', a.contract)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    with a.output.open('x') as stream:
        for row in module.generate(frames=616, idle=617):
            if any(s['hold'] or s['trig'] or s['release'] for s in row['samples']):
                raise ValueError('neutral suffix contains button edge')
            stream.write(json.dumps(row, sort_keys=True, separators=(',', ':')) + '\n')
    print(json.dumps({'frames':616, 'reads':2464, 'sha256':hashlib.sha256(a.output.read_bytes()).hexdigest(),
                      'connection':'source synthetic neutral; actual suffix WPADProbe export required',
                      'qualification':'BLOCKED', 'qualified_frames':None}))


if __name__ == '__main__':
    main()
