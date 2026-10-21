"""PURE SOURCE PROPOSAL ONLY. Not imported by any active launch/guard/plan.

No environment reads, child launches, signal calls, default/platform value or
admission authorization. Callers must obtain reviewed operator bytes separately.
"""
import hashlib
import json

KEY = '__CF_USER_TEXT_ENCODING'
BASE_KEYS = frozenset(('HOME','PATH','LANG','LC_ALL','TMPDIR',
                      'PYTHONCOERCECLOCALE','PYTHONUTF8','PYTHONNOUSERSITE'))
VALUE_CAP = 512


def canonical(value):
    return json.dumps(value,sort_keys=True,separators=(',',':'),ensure_ascii=True).encode()


def value_bytes(value):
    if type(value) is not str:
        raise ValueError('named platform value must be a string')
    raw=value.encode('utf-8',errors='strict')
    if len(raw)>VALUE_CAP or b'\0' in raw:
        raise ValueError('named platform value exceeds representable bound')
    return raw


def observe_named_only(selected):
    """Proposed metadata representation; input may contain ONLY the exact key.

    Does not read os.environ. A future separately reviewed launcher/child would
    pass a selected mapping, never an entire environment. Missing != empty.
    """
    if type(selected) is not dict or not set(selected)<= {KEY}:
        raise ValueError('only exact named platform key may be observed')
    if KEY not in selected:
        return {'key':KEY,'present':False}
    raw=value_bytes(selected[KEY])
    return {'key':KEY,'present':True,'representation':'strict UTF-8 hex',
            'value_hex':raw.hex(),'bytes':len(raw),'sha256':hashlib.sha256(raw).hexdigest()}


def require_operator_mapping(record_bytes,operator_sha256,base_env):
    """Proposed exact external operator record, deliberately unwired.

    A synthetic accepted model is not approval of an actual platform value.
    """
    if type(record_bytes) is not bytes or len(record_bytes)>16384:
        raise ValueError('bounded operator record bytes required')
    if hashlib.sha256(record_bytes).hexdigest()!=operator_sha256:
        raise ValueError('operator record pin mismatch')
    record=json.loads(record_bytes)
    if set(record)!={'schema','env','env_sha256','named_value'} or record['schema']!=1:
        raise ValueError('exact operator record schema required')
    if type(base_env) is not dict or set(base_env)!=BASE_KEYS:
        raise ValueError('exact existing eight-key base required')
    env=record['env']
    if type(env) is not dict or set(env)!=BASE_KEYS|{KEY}:
        raise ValueError('exact full nine-key mapping required')
    if any(type(v) is not str for v in env.values()) or any(env[k]!=base_env[k] for k in BASE_KEYS):
        raise ValueError('operator base mapping differs')
    if hashlib.sha256(canonical(env)).hexdigest()!=record['env_sha256']:
        raise ValueError('full environment pin mismatch')
    if record['named_value']!=observe_named_only({KEY:env[KEY]}):
        raise ValueError('named exact value representation/pin mismatch')
    return dict(env)


def require_exact_observed(actual,expected):
    """Model exact parent/child/postack/postexec checks: no ignored extras."""
    if type(actual) is not dict or set(expected)!=BASE_KEYS|{KEY} or actual!=expected:
        raise ValueError('exact full operator mapping differs')
