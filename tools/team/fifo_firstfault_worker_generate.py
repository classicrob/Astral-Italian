#!/usr/bin/env python3
"""Generate isolated worker-only diagnostics. No compiler/runtime/integration."""
import argparse
import difflib
import hashlib
import json
from pathlib import Path
import re
import sys

SOURCE_SHA = 'b1cf672a6c51dec9e588932876ce6f963b6052f1238fa19151d0edf022c940fc'
OBJECT_SHA = 'f50d2a595781a54826c15a3e2276127befcda516c9a7b8dfdf01307f4ec2d190'
BRIDGE_SHA = '771440cb3794f96b622e1a38f31642cd5d66eb463535010eebc4a9dd28df06a3'
TEMPLATE_SHA = '656f8e36ca342db4afa98f59850dad88bcb5cbf1e794d18b8d3dfc14f8f959e4'
OWNED = Path.home() / 'MacioGalaxy2-team/scratch-port/solP/fifo-worker-generated'
PRIVATE = Path.home() / 'MacioGalaxy2-gamedata/solP-fifo-firstfault'


def sha(b): return hashlib.sha256(b).hexdigest()
def require(ok, msg):
    if not ok: raise ValueError(msg)


def fresh(path, parent):
    require(not path.is_symlink(), 'symlink output')
    base = parent.absolute(); require(base.resolve() == base, 'symlink owned parent')
    path = path.absolute(); require(path.resolve() == path, 'symlink output ancestry')
    require(base in path.parents and not path.exists(), 'fresh confined child required')
    return path


def insert(text, anchor, code, tag, after=False):
    require(text.count(anchor) == 1, 'missing/nonunique anchor: ' + tag)
    i = text.index(anchor) + (len(anchor) if after else 0)
    block = '// P_FIFO_FF_BEGIN ' + tag + '\n' + code + '\n// P_FIFO_FF_END ' + tag + '\n'
    return text[:i] + block + text[i:]


def strip_hooks(text):
    return re.sub(r'^// P_FIFO_FF_BEGIN ([a-z_]+)\n.*?^// P_FIFO_FF_END \1\n', '', text, flags=re.M|re.S)


def transform(source, include_path):
    require('P_FIFO_FF_BEGIN' not in source, 'already instrumented')
    # Restrict process-local anchors: helpers also read command bytes.
    start = 'ProcessResult process(const u8* data, u32 size) noexcept {'
    finish = '[[noreturn]] static void handle_draw_overrun('
    require(source.count(start)==source.count(finish)==1, 'process boundaries')
    a,b=source.index(start),source.index(finish); require(a<b,'process scope order')
    body=source[a:b]
    body=insert(body,'  ByteReader reader{{data, size}};\n','  PWorkerFirstFault::SpanScope<ByteReader> p_ff_span(reader, g_gxState);','span',True)
    body=insert(body,'    const u8 cmd = reader.read<u8>();\n','    PWorkerFirstFault::command(cmd, reader.offset() - 1, reader.remaining());','raw',True)
    body=insert(body,'      handle_cp(addr, reader.read<u32>());\n','      PWorkerFirstFault::cp(addr, reader.offset(), g_gxState);','cp',True)
    body=insert(body,'      Log.warn("Ignoring nested GX_CMD_CALL_DL");','      PWorkerFirstFault::warning();','warning')
    body=insert(body,'      AURORA_ASSERT(array.data != nullptr, "indexed XF load from unmapped array {}", arrayType);','      PWorkerFirstFault::indexed(arrayType, srcArrayIdx, dstAddr, len, srcOffset, srcSize, array);','indexed')
    output=source[:a]+body+source[b:]
    output=insert(output,'[[noreturn]] static void handle_draw_overrun(size_t totalVtxBytes, const ByteReader& reader) noexcept {\n','  PWorkerFirstFault::draw_overrun();','overrun',True)
    output=insert(output,'static void draw_prim(GXPrimitive prim, GXVtxFmt fmt, u16 vtxCount, ByteReader& reader) noexcept {\n  ZoneScoped;\n','  PWorkerFirstFault::PrimitiveScope<ByteReader> p_ff_primitive(reader, static_cast<unsigned>(prim), static_cast<unsigned>(fmt), vtxCount, g_gxState);','primitive',True)
    output=insert(output,'  u32 totalVtxBytes = vtxCount * vtxSize;\n','  PWorkerFirstFault::primitive_bytes(vtxSize, totalVtxBytes);','primitive_bytes',True)
    output=insert(output,'    reader.skip(totalVtxBytes);\n','    PWorkerFirstFault::geometry_skip();','skip',True)
    output=insert(output,'  } else if (subCmd == GX_AURORA_LOAD_TEXOBJ) {','    PWorkerFirstFault::native_array(subCmd, attrIdx, array);','native_array')
    output='// P_FIFO_FF_BEGIN include\n#include '+json.dumps(str(include_path))+'\n// P_FIFO_FF_END include\n'+output
    require(strip_hooks(output)==source,'original source not byte-identical after hook removal')
    return output


def generate(args):
    raw=Path(args.source).read_bytes(); require(sha(raw)==SOURCE_SHA,'effective worker source hash')
    bridge_bytes=Path(args.bridge).read_bytes();require(sha(bridge_bytes)==BRIDGE_SHA,'typed source bridge hash')
    bridge=json.loads(bridge_bytes)
    require(bridge['mirror_sha256']==SOURCE_SHA and bridge['compiled_edge']['object_sha256']==OBJECT_SHA,'compiled edge binding')
    require(bridge['binary_sha256']=='f579a53439f338829d4a9aef8482f345fe2a992556229ab2b85c6745c59785b8','typed binary bridge')
    receipt=Path(bridge['typed_build_receipt']);require(sha(receipt.read_bytes())==bridge['typed_build_receipt_sha256'],'typed receipt drift')
    obj=Path(bridge['compiled_edge']['object']);require(sha(obj.read_bytes())==OBJECT_SHA,'actual compiled worker object drift')
    template=Path(__file__).with_name('fifo_firstfault_worker_capture.hpp.in').read_bytes();require(sha(template)==TEMPLATE_SHA,'template hash')
    out=fresh(Path(args.output),OWNED);private=fresh(Path(args.capture_dir),PRIVATE)
    header=template.decode().replace('@CAPTURE_PARENT@',json.dumps(str(private))).replace('@CAPTURE_NAME@',json.dumps('firstfault.log'))
    require('@CAPTURE_' not in header,'unresolved template')
    cpp=transform(raw.decode(),out/'fifo_firstfault_worker_capture.hpp')
    # Validate everything before creating either owned directory.
    out.mkdir(parents=True,mode=0o700);private.mkdir(parents=True,mode=0o700)
    private.chmod(0o700)
    (out/'command_processor.cpp').write_text(cpp)
    (out/'fifo_firstfault_worker_capture.hpp').write_text(header)
    patch=''.join(difflib.unified_diff(raw.decode().splitlines(True),cpp.splitlines(True),fromfile='command_processor.cpp',tofile='command_processor.cpp'))
    (out/'worker.patch').write_text(patch)
    report={'status':'SOURCE_GENERATED_REVIEW_REQUIRED_NOT_COMPILED','source_sha256':SOURCE_SHA,'source_bridge_sha256':BRIDGE_SHA,'source_bridge':bridge,'original_operations_strip_equal':True,'env':'PETARI_FIFO_FIRSTFAULT=1','marker':'P_FIFO_WORKER_FIRSTFAULT_v1','runtime_output':str(private/'firstfault.log'),'capture_ready':False,'production_acceptance':False,'limits':['original DL UNKNOWN','caller UNKNOWN','absolute FIFO ownership UNKNOWN','true alignment UNKNOWN','ring overwrite and raw cap explicit','I/O timing perturbation; incomplete writes LIMIT'],'files':{n:sha((out/n).read_bytes()) for n in ['command_processor.cpp','fifo_firstfault_worker_capture.hpp','worker.patch']}}
    (out/'manifest.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({'status':report['status'],'manifest':str(out/'manifest.json'),'files':report['files']}))


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ['source','bridge','output','capture-dir']:p.add_argument('--'+n,required=True)
    args=p.parse_args()
    try:generate(args)
    except (OSError,ValueError,KeyError,UnicodeError) as e:print('REJECTED: '+str(e),file=sys.stderr);return 1
    return 0

if __name__=='__main__':sys.exit(main())
