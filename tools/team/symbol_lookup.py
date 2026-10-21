"""Validate representative final-image symbols and existing DWARF lookup output."""
from pathlib import Path
import re

SYMBOLS = {
    'native': '_petari_host_frame_seam',
    'game': '__ZN5Mario6updateEv',
    'gx': '__ZN6aurora2gx6renderERKNS0_8DrawDataERKN4wgpu17RenderPassEncoderE',
}


def select_addresses(nm_text):
    found = {scope: [] for scope in SYMBOLS}
    for line in nm_text.splitlines():
        match = re.fullmatch(r'([0-9a-fA-F]{16})\s+[tT]\s+(\S+)', line)
        if not match:
            continue
        for scope, symbol in SYMBOLS.items():
            if match[2] == symbol:
                found[scope].append('0x' + match[1])
    if any(len(rows) != 1 for rows in found.values()):
        raise ValueError('each exact final-image symbol must occur once')
    return {scope: rows[0] for scope, rows in found.items()}


def lookup_witness(text, expected_files):
    """Accept a positive Line info witness owned by one explicitly pinned file.

    dwarfdump may print a home-relative file in Line info. Only an exact suffix
    of an operator-supplied absolute path is admitted; no filesystem search or
    guessed source substitution is performed.
    """
    expected = [Path(p).resolve(strict=False) for p in expected_files]
    if not expected or any(not Path(p).is_absolute() for p in expected_files):
        raise ValueError('explicit absolute source ownership required')
    witnesses = []
    for line in text.splitlines():
        match = re.fullmatch(r"Line info: file '(.+?)', line ([0-9]+), column ([0-9]+)(?:,.*)?", line)
        if not match or int(match[2]) == 0:
            continue
        raw = Path(match[1])
        if '..' in raw.parts:
            raise ValueError('noncanonical line file')
        owners = [p for p in expected if
                  (raw.is_absolute() and raw.resolve(strict=False) == p) or
                  (not raw.is_absolute() and len(raw.parts) >= 3 and p.parts[-len(raw.parts):] == raw.parts)]
        if len(owners) != 1:
            raise ValueError('line witness has wrong or ambiguous source ownership')
        if not owners[0].is_file():
            raise ValueError('owned source missing')
        witnesses.append({'reported_file':str(raw), 'normalized_file':str(owners[0]),
                          'line':int(match[2]), 'column':int(match[3])})
    if not witnesses:
        raise ValueError('no positive source-owned Line info witness')
    return witnesses
