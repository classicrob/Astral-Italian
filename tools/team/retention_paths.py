"""Read-only debug-map path normalization. No deletion API is provided."""
import os
from pathlib import Path
import re


def canonical_path(path):
    path = Path(path)
    if not path.is_absolute():
        raise ValueError('absolute path required for retention closure')
    return Path(os.path.normpath(path)).resolve(strict=False)


def split_archive_reference(reference):
    match = re.fullmatch(r'(.+\.a)\(([^()]+)\)', reference)
    return (match[1], match[2]) if match else (reference, None)


def retained_reference(reference, original_root, retained_root):
    """Missing files remain required references; existence never decides closure."""
    file_path, member = split_archive_reference(reference)
    source = canonical_path(file_path)
    original, retained = canonical_path(original_root), canonical_path(retained_root)
    if source.is_relative_to(original):
        source = canonical_path(retained / source.relative_to(original))
    return source, member


def reference_closure(references, original_root, retained_root):
    return {str(retained_reference(path, original_root, retained_root)[0]) for path in references}
