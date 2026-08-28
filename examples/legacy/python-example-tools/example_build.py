#!/usr/bin/env python3
from __future__ import annotations
import argparse, json, os, shutil, subprocess, sys, tempfile
from pathlib import Path

VALID_EXTS = {'.CMOT','.CMTV','.HMOT','.HMTV','.cmot','.cmtv','.hmot','.hmtv'}

def load_rows(root: Path):
    manifest = root/'manifests'/'examples.jsonl'
    return [json.loads(x) for x in manifest.read_text(encoding='utf-8').splitlines() if x.strip()]

def compiler_cmd(compiler: str, args: list[str]) -> list[str]:
    return ([sys.executable, compiler] if compiler.endswith('.py') else [compiler]) + args

def run(cmd, timeout=None):
    return subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, timeout=timeout)

def default_compiler(root: Path) -> str:
    candidate = root.parent/'tools'/'cmotive.py'
    return str(candidate) if candidate.exists() else os.environ.get('CMOTIVE', 'cmotive')

def default_preprocessor(root: Path) -> str:
    candidate = root.parent/'tools'/'cmotivepp.py'
    return str(candidate) if candidate.exists() else os.environ.get('CMOTIVEPP', 'cmotivepp')

def include_args(root: Path):
    args = ['-I', str(root/'headers'), '-I', str(root), '-I', str(root.parent/'lib')]
    return args

def compile_one(root: Path, compiler: str, row: dict, out: Path, obj=False, keep_c=False):
    src = root/row['file']
    out.parent.mkdir(parents=True, exist_ok=True)
    args = include_args(root)
    if obj:
        args += ['-c']
    if keep_c:
        args += ['--keep-c']
    args += [str(src), '-o', str(out)]
    return run(compiler_cmd(compiler, args))

def preprocess_one(root: Path, pp: str, row: dict, out: Path):
    src = root/row['file']
    out.parent.mkdir(parents=True, exist_ok=True)
    args = ['-I', str(root/'headers'), '-I', str(root), '-I', str(root.parent/'lib'), str(src), '-o', str(out)]
    return run(compiler_cmd(pp, args))

def verify_run(root: Path, compiler: str, rows: list[dict], timeout: float, keep_build=False):
    build = Path(tempfile.mkdtemp(prefix='cmotive-examples-'))
    failures = []
    try:
        for row in rows:
            exe = build/(Path(row['file']).stem + ('.exe' if os.name == 'nt' else ''))
            cp = compile_one(root, compiler, row, exe, keep_c=True)
            if cp.returncode != 0 or cp.stderr:
                failures.append({'file': row['file'], 'phase': 'compile', 'code': cp.returncode, 'stdout': cp.stdout, 'stderr': cp.stderr})
                continue
            try:
                rp = run([str(exe)], timeout=timeout)
            except subprocess.TimeoutExpired:
                failures.append({'file': row['file'], 'phase': 'run', 'code': 'timeout', 'stdout': '', 'stderr': ''})
                continue
            expected = int(row.get('expected_exit', 0))
            contains = row.get('expected_stdout_contains', '')
            if rp.returncode != expected or rp.stderr or (contains and contains not in rp.stdout):
                failures.append({'file': row['file'], 'phase': 'run', 'code': rp.returncode, 'stdout': rp.stdout, 'stderr': rp.stderr, 'expected_stdout_contains': contains})
        report = root/'VERIFY_REPORT.md'
        if failures:
            report.write_text('# CMotive language examples verification report\n\nFAILED\n\n```json\n'+json.dumps(failures, indent=2)+'\n```\n', encoding='utf-8')
            print(f'FAILED: {len(failures)} of {len(rows)} example(s). See {report}', file=sys.stderr)
            return 1
        report.write_text(f'# CMotive language examples verification report\n\nPASS: {len(rows)} examples compiled, linked, executed, exited with expected status, produced no stderr, and printed their expected verification marker.\n', encoding='utf-8')
        print(f'PASS: {len(rows)} examples')
        return 0
    finally:
        if keep_build:
            print('build dir:', build)
        else:
            shutil.rmtree(build, ignore_errors=True)

def build_mode(root: Path, compiler: str, rows, mode: str):
    build = root/'build'
    failures=[]
    for row in rows:
        stem = Path(row['file']).stem
        if mode == 'compile':
            out = build/'bin'/(stem + ('.exe' if os.name == 'nt' else ''))
            p = compile_one(root, compiler, row, out)
        elif mode == 'objects':
            out = build/'obj'/(stem + ('.obj' if os.name == 'nt' else '.o'))
            p = compile_one(root, compiler, row, out, obj=True)
        else:
            raise ValueError(mode)
        if p.returncode != 0 or p.stderr:
            failures.append({'file': row['file'], 'mode': mode, 'code': p.returncode, 'stdout': p.stdout, 'stderr': p.stderr})
    if failures:
        print(json.dumps(failures, indent=2), file=sys.stderr)
        return 1
    print(f'{mode}: {len(rows)} examples')
    return 0

def preprocess_mode(root: Path, pp: str, rows):
    failures=[]
    for row in rows:
        rel = Path(row['file'])
        out = root/'build'/'pp'/rel
        p = preprocess_one(root, pp, row, out)
        if p.returncode != 0 or p.stderr:
            failures.append({'file': row['file'], 'code': p.returncode, 'stdout': p.stdout, 'stderr': p.stderr})
    if failures:
        print(json.dumps(failures, indent=2), file=sys.stderr)
        return 1
    print(f'preprocess: {len(rows)} examples')
    return 0

def debug_symbols_mode(root: Path, compiler: str, rows):
    row = next((r for r in rows if r.get('id') == 150), None)
    if row is None:
        print('example 150 not present in manifest', file=sys.stderr)
        return 1
    build = root/'build'/'debug-symbols'
    exe = build/'example150_debug_symbols'
    build.mkdir(parents=True, exist_ok=True)
    args = include_args(root) + ['-g3', '-O2', str(root/row['file']), '-o', str(exe)]
    p = run(compiler_cmd(compiler, args))
    if p.returncode != 0 or p.stderr:
        print(p.stdout, end='')
        print(p.stderr, file=sys.stderr, end='')
        return 1
    syms = Path(str(exe) + '_cmot_debugsymbols.syms')
    meta = Path(str(exe) + '.cmotive.debug.json')
    if not syms.exists() or not meta.exists():
        print('debug symbol output missing', file=sys.stderr)
        return 1
    text = syms.read_text(encoding='utf-8')
    for token in ['debug_level: 3', 'optimization: O2', 'StartPackage__ExampleDebugSymbol__Add', 'I32 StartPackage::ExampleDebugSymbol::Add(rhs: I32)']:
        if token not in text:
            print('debug symbol output missing token: ' + token, file=sys.stderr)
            return 1
    print('debug-symbols: PASS')
    return 0

def manifest_check(root: Path, rows):
    missing = [r['file'] for r in rows if not (root/r['file']).is_file()]
    if missing:
        print('manifest references missing files: ' + ', '.join(missing), file=sys.stderr)
        return 1
    print(f'manifest ok: {len(rows)} examples')
    return 0

def main(argv=None):
    ap = argparse.ArgumentParser()
    ap.add_argument('mode', choices=['manifest','compile','objects','preprocess','run','check','clean','list','debug-symbols'])
    ap.add_argument('--compiler')
    ap.add_argument('--preprocessor')
    ap.add_argument('--timeout', type=float, default=5.0)
    ap.add_argument('--keep-build', action='store_true')
    ns = ap.parse_args(argv)
    root = Path(__file__).resolve().parents[1]
    if ns.mode == 'clean':
        shutil.rmtree(root/'build', ignore_errors=True); print('cleaned build outputs'); return 0
    rows = load_rows(root)
    compiler = ns.compiler or default_compiler(root)
    pp = ns.preprocessor or default_preprocessor(root)
    if ns.mode == 'list':
        print('\n'.join(r['file'] for r in rows)); return 0
    if ns.mode == 'debug-symbols': return debug_symbols_mode(root, compiler, rows)
    if ns.mode == 'manifest': return manifest_check(root, rows)
    if ns.mode == 'compile': return build_mode(root, compiler, rows, 'compile')
    if ns.mode == 'objects': return build_mode(root, compiler, rows, 'objects')
    if ns.mode == 'preprocess': return preprocess_mode(root, pp, rows)
    if ns.mode == 'run': return verify_run(root, compiler, rows, ns.timeout, ns.keep_build)
    if ns.mode == 'check':
        rc = manifest_check(root, rows) or build_mode(root, compiler, rows, 'objects') or verify_run(root, compiler, rows, ns.timeout, ns.keep_build)
        return rc
if __name__ == '__main__':
    raise SystemExit(main())
