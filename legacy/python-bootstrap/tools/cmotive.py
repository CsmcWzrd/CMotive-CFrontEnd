#!/usr/bin/env python3
import argparse, json, os, platform, shutil, subprocess, sys, tempfile
from pathlib import Path

def find_root(start):
    for base in [start] + list(start.parents):
        if (base / 'src' / 'cmotive').exists():
            return base
        if base.name == 'bin' and (base.parent.parent / 'src' / 'cmotive').exists():
            return base.parent.parent
    return start.parents[1]
ROOT = find_root(Path(__file__).resolve())
sys.path.insert(0, str(ROOT / 'src'))
from cmotive.compiler import CompilerPipeline

VALID_SRC = {'.CMOT','.CMTV','.HMOT','.HMTV','.cmot','.cmtv','.hmot','.hmtv'}
OBJ_EXTS = {'.o','.obj'}
ARCH_ALIASES = {
    'x86':'x86','i386':'x86','i686':'x86',
    'x64':'x86_64','x86_64':'x86_64','amd64':'x86_64',
    'arm':'arm','armv7':'arm','arm64':'arm64','aarch64':'arm64'
}

def choose_cc():
    if os.environ.get('CMOTIVE_CC'):
        return os.environ['CMOTIVE_CC']
    if platform.system() == 'Windows':
        return shutil.which('clang') or shutil.which('gcc') or shutil.which('cl') or 'cc'
    return shutil.which('cc') or shutil.which('clang') or shutil.which('gcc') or 'cc'

def choose_ld(cc):
    return os.environ.get('CMOTIVE_LD') or cc

def compiler_is_msvc(cc):
    return Path(str(cc).split()[0]).name.lower() in {'cl','cl.exe'}

def optimization_flag(value):
    value = (value or '').strip()
    if value in {'O1', 'O2', 'O3', 'Os'}:
        return value
    return ''

def write_debug_metadata(path, symbols, inputs, target_arch, debug_level, optimize):
    data = {
        'format': 'CMotive debug metadata v1',
        'debug_level': int(debug_level or 0),
        'optimization': optimization_flag(optimize),
        'target_arch': target_arch,
        'inputs': [str(x) for x in inputs],
        'symbols': symbols or [],
    }
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(data, indent=2, sort_keys=True) + '\n', encoding='utf-8')
    return path

def debug_metadata_path(out):
    out = Path(out)
    return out.with_suffix(out.suffix + '.cmotive.debug.json')

def symbol_output_path(out):
    out = Path(out)
    return out.with_name(out.name + '_cmot_debugsymbols.syms')

def run_symbol_tool(binary_path, metadata_path):
    script = ROOT / 'tools' / 'CMotiveSymsToDebugFile.py'
    if not script.exists():
        return 0
    cmd = [sys.executable, str(script), str(binary_path), '--metadata', str(metadata_path), '-o', str(symbol_output_path(binary_path))]
    return run_cmd(cmd)

def canonical_arch(value):
    v = (value or '').lower()
    return ARCH_ALIASES.get(v, v or 'native')

def arch_flags(target_arch, for_link=False):
    arch = canonical_arch(target_arch)
    sysname = platform.system()
    if compiler_is_msvc(choose_cc()):
        return []
    if sysname == 'Darwin' and arch in {'x86_64','arm64'}:
        return ['-arch', arch]
    if arch == 'x86_64' and sysname != 'Windows':
        return ['-m64']
    if arch == 'x86' and sysname != 'Windows':
        return ['-m32']
    return []

def run_cmd(cmd):
    p = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if p.returncode:
        sys.stderr.write(p.stdout + p.stderr)
    return p.returncode

def compile_c(cc, c_path, obj_path, target_arch=None, debug_level=0, optimize=""):
    obj_path.parent.mkdir(parents=True, exist_ok=True)
    opt = optimization_flag(optimize)
    if compiler_is_msvc(cc):
        cmd = [cc, '/nologo', '/c', str(c_path), '/Fo' + str(obj_path)]
        if debug_level:
            cmd.append('/Zi' if debug_level >= 2 else '/Z7')
        if opt in {'O1','Os'}:
            cmd.append('/O1')
        elif opt in {'O2','O3'}:
            cmd.append('/O2')
        return cmd
    cmd = [cc]
    cmd += arch_flags(target_arch)
    if debug_level:
        cmd.append('-g' if debug_level == 1 else f'-g{debug_level}')
    if opt:
        cmd.append('-' + opt)
    # Strip-compatible native object output. Debug sections are normal DWARF/CodeView
    # data when requested and can be separated or removed by platform strip tools.
    cmd += ['-c', str(c_path), '-o', str(obj_path)]
    return cmd

def link_objects(ld, objs, out, libdirs, libs, target_arch=None, debug_level=0, optimize=""):
    out.parent.mkdir(parents=True, exist_ok=True)
    opt = optimization_flag(optimize)
    if compiler_is_msvc(ld):
        cmd = [ld, '/nologo'] + [str(o) for o in objs] + ['/Fe:' + str(out)]
        if debug_level:
            cmd.append('/DEBUG')
        return cmd
    cmd = [ld]
    cmd += arch_flags(target_arch)
    if debug_level:
        cmd.append('-g' if debug_level == 1 else f'-g{debug_level}')
    if opt:
        cmd.append('-' + opt)
    cmd += [str(p) for p in objs]
    cmd += ['-L' + d for d in libdirs]
    # The generated standard-library runtime includes math and concurrency helpers.
    # Add the portable linker switches by default while still respecting explicit user libs.
    if platform.system() != 'Windows':
        cmd += ['-pthread']
        if 'm' not in libs:
            cmd += ['-lm']
    else:
        if not compiler_is_msvc(ld) and 'ws2_32' not in libs:
            cmd += ['-lws2_32']
    cmd += ['-l' + l for l in libs] + ['-o', str(out)]
    return cmd

def main(argv=None):
    ap = argparse.ArgumentParser(prog='cmotive')
    ap.add_argument('--version', action='store_true')
    ap.add_argument('-c', dest='compile_only', action='store_true')
    ap.add_argument('-o', dest='output')
    ap.add_argument('-I', dest='includes', action='append', default=[])
    ap.add_argument('-L', dest='libdirs', action='append', default=[])
    ap.add_argument('-l', dest='libs', action='append', default=[])
    ap.add_argument('--target-arch', choices=sorted(ARCH_ALIASES.keys()))
    ap.add_argument('-g', dest='debug_level', action='store_const', const=1, default=0, help='emit level-1 debug information and CMotive .syms file')
    ap.add_argument('-g2', dest='debug_level', action='store_const', const=2, help='emit level-2 debug information and CMotive .syms file')
    ap.add_argument('-g3', dest='debug_level', action='store_const', const=3, help='emit level-3 debug information and CMotive .syms file')
    ap.add_argument('-O1', dest='optimize', action='store_const', const='O1', default='', help='optimize generated native code')
    ap.add_argument('-O2', dest='optimize', action='store_const', const='O2', help='optimize generated native code more aggressively')
    ap.add_argument('-O3', dest='optimize', action='store_const', const='O3', help='optimize generated native code for speed')
    ap.add_argument('-Os', dest='optimize', action='store_const', const='Os', help='optimize generated native code for size')
    ap.add_argument('--emit-c', action='store_true')
    ap.add_argument('--keep-c', action='store_true')
    ap.add_argument('--print-linker', action='store_true')
    ap.add_argument('--print-toolchain', action='store_true')
    ap.add_argument('--print-target-arch', action='store_true')
    ap.add_argument('inputs', nargs='*')
    ns = ap.parse_args(argv)
    if ns.version:
        print('CMotive compiler 0.2.2-rc1')
        return 0
    cc = choose_cc(); ld = choose_ld(cc)
    if ns.print_linker:
        print(ld)
        return 0
    if ns.print_toolchain:
        print('cc=' + str(cc))
        print('ld=' + str(ld))
        return 0
    target_arch = canonical_arch(ns.target_arch or platform.machine())
    if ns.print_target_arch:
        print(target_arch)
        return 0
    if not ns.inputs:
        ap.error('no input files')
    srcs = [Path(p) for p in ns.inputs if Path(p).suffix in VALID_SRC]
    objs = [Path(p) for p in ns.inputs if Path(p).suffix.lower() in OBJ_EXTS]
    if not srcs and not objs:
        ap.error('no CMotive source/header or object input files')
    out = Path(ns.output) if ns.output else Path('a.out' if not ns.compile_only else (srcs[0].stem + ('.obj' if platform.system() == 'Windows' else '.o')))
    out.parent.mkdir(parents=True, exist_ok=True)
    pipe = CompilerPipeline(ns.includes, target_arch)
    temp_ctx = tempfile.TemporaryDirectory(prefix='cmotive-')
    with temp_ctx as td_s:
        td = Path(td_s)
        made = []
        debug_records = []
        compiled_inputs = []
        for s in srcs:
            unit = pipe.compile_to_c(s)
            compiled_inputs.append(str(s))
            for rec in getattr(unit, 'debug_symbols', []) or []:
                rec = dict(rec)
                rec['source'] = str(s)
                debug_records.append(rec)
            c = td / (s.stem + '.c')
            c.write_text(unit.c_source, encoding='utf-8')
            if ns.keep_c:
                keep = out.parent / (s.stem + '.c') if not ns.emit_c else out
                keep.write_text(unit.c_source, encoding='utf-8')
            if ns.emit_c:
                target = out if len(srcs) == 1 else out.parent / (s.stem + '.c')
                target.write_text(unit.c_source, encoding='utf-8')
                continue
            obj = out if ns.compile_only and len(srcs) == 1 else td / (s.stem + ('.obj' if platform.system() == 'Windows' else '.o'))
            rc = run_cmd(compile_c(cc, c, obj, target_arch, ns.debug_level, ns.optimize))
            if rc:
                return rc
            made.append(obj)
        if ns.emit_c or ns.compile_only:
            if ns.debug_level and not ns.emit_c:
                meta = write_debug_metadata(debug_metadata_path(out), debug_records, compiled_inputs, target_arch, ns.debug_level, ns.optimize)
                run_symbol_tool(out, meta)
            return 0
        rc = run_cmd(link_objects(ld, made + objs, out, ns.libdirs, ns.libs, target_arch, ns.debug_level, ns.optimize))
        if rc:
            return rc
        if ns.debug_level:
            meta = write_debug_metadata(debug_metadata_path(out), debug_records, compiled_inputs, target_arch, ns.debug_level, ns.optimize)
            rc = run_symbol_tool(out, meta)
            if rc:
                return rc
    return 0

if __name__ == '__main__':
    raise SystemExit(main())
