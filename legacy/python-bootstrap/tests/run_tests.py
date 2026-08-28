#!/usr/bin/env python3
import argparse, os, subprocess, sys
from pathlib import Path


def run(cmd, timeout=45):
    p = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, timeout=timeout)
    if p.returncode:
        print('FAILED:', ' '.join(map(str, cmd)))
        if p.stdout: print(p.stdout)
        if p.stderr: print(p.stderr)
    return p.returncode == 0


def run_expect_code(cmd, expected=0, timeout=15, allow_stdout=True):
    p = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, timeout=timeout)
    if p.returncode != expected:
        print('FAILED:', ' '.join(map(str, cmd)), 'expected', expected, 'got', p.returncode)
        if p.stdout: print(p.stdout)
        if p.stderr: print(p.stderr)
        return False
    if p.stderr:
        print('FAILED: stderr produced by', ' '.join(map(str, cmd)))
        print(p.stderr)
        return False
    return True


def run_expect_failure(cmd, timeout=45):
    p = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, timeout=timeout)
    if p.returncode == 0:
        print('FAILED:', ' '.join(map(str, cmd)), 'expected failure but got success')
        if p.stdout: print(p.stdout)
        if p.stderr: print(p.stderr)
        return False
    return True


def build_and_run(b, src, out, exe_suffix='', include=None):
    cmd = [str(b/'cmotive'), src, '-o', 'build/' + out + exe_suffix]
    if include:
        cmd += ['-I', include]
    return run(cmd) and run_expect_code(['build/' + out + exe_suffix], 0)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--bin', default='build/bin')
    ap.add_argument('--full', action='store_true', help='run the larger local suite; default is the release smoke suite')
    ns = ap.parse_args()
    b = Path(ns.bin)
    exe_suffix = '.exe' if os.name == 'nt' else ''
    ok = True

    # Default `make test` is intentionally a release smoke suite so it
    # completes quickly even when generated C embeds the broad Sys runtime.
    ok &= run([str(b/'cmotive'), '--version'])
    ok &= run([str(b/'cmotive'), '-c', 'tests/conformance/basic.CMOT', '-o', 'build/basic.o'])
    ok &= build_and_run(b, 'tests/conformance/basic.CMOT', 'basic', exe_suffix)

    dbg_out = 'build/debug_symbols' + exe_suffix
    ok &= run([str(b/'cmotive'), '-g3', '-O2', 'tests/conformance/cmotive_debug_symbols.CMOT', '-o', dbg_out])
    ok &= run_expect_code([dbg_out], 0)
    syms_path = Path(dbg_out + '_cmot_debugsymbols.syms')
    meta_path = Path(dbg_out + '.cmotive.debug.json')
    if ok and (not syms_path.exists() or not meta_path.exists()):
        print('FAILED: debug symbol output files missing')
        ok = False
    if ok:
        syms = syms_path.read_text(encoding='utf-8')
        for token in ['debug_level: 3', 'optimization: O2', 'StartPackage__DebugThing__Add', 'I32 StartPackage::DebugThing::Add(y: I32)', 'StartPackage__Helper', '0x']:
            if token not in syms:
                print('FAILED: debug symbol file missing', token)
                ok = False
                break

    # New standard-library/native-socket/Dynamic-Struct coverage.
    ok &= build_and_run(b, 'tests/conformance/cmotive_sys_io_rename.CMOT', 'sys_io_rename', exe_suffix)
    ok &= build_and_run(b, 'tests/conformance/cmotive_stl_containers.CMOT', 'stl_containers', exe_suffix)
    ok &= build_and_run(b, 'tests/conformance/cmotive_algorithms.CMOT', 'algorithms', exe_suffix)
    ok &= build_and_run(b, 'tests/conformance/cmotive_net_native_sockets.CMOT', 'net_native_sockets', exe_suffix)
    ok &= build_and_run(b, 'tests/conformance/cmotive_thread_native.CMOT', 'thread_native', exe_suffix)
    ok &= build_and_run(b, 'tests/conformance/cmotive_dynamic_struct.CMOT', 'dynamic_struct', exe_suffix)
    ok &= build_and_run(b, 'tests/conformance/cmotive_auto_getset.CMOT', 'auto_getset', exe_suffix)
    ok &= build_and_run(b, 'tests/conformance/cmotive_operation_overload.CMOT', 'operation_overload', exe_suffix)
    ok &= build_and_run(b, 'tests/conformance/cmotive_tstore_threadstore.CMOT', 'tstore_threadstore', exe_suffix)
    ok &= build_and_run(b, 'tests/conformance/cmotive_global_anywhere.CMOT', 'global_anywhere', exe_suffix)
    ok &= build_and_run(b, 'tests/conformance/cmotive_fptr_function_pointer.CMOT', 'fptr_function_pointer', exe_suffix)
    ok &= build_and_run(b, 'tests/conformance/cmotive_overridable_pure_virtual.CMOT', 'overridable_pure_virtual', exe_suffix)
    ok &= build_and_run(b, 'tests/conformance/cmotive_stl_object_methods.CMOT', 'stl_object_methods', exe_suffix)
    ok &= build_and_run(b, 'tests/conformance/cmotive_algorithms_object_methods.CMOT', 'algorithms_object_methods', exe_suffix)
    ok &= build_and_run(b, 'tests/conformance/cmotive_io_object_methods.CMOT', 'io_object_methods', exe_suffix)
    ok &= build_and_run(b, 'tests/conformance/cmotive_filesystem_object_methods.CMOT', 'filesystem_object_methods', exe_suffix)
    ok &= build_and_run(b, 'tests/conformance/cmotive_net_object_methods.CMOT', 'net_object_methods', exe_suffix)
    ok &= build_and_run(b, 'tests/conformance/cmotive_thread_object_methods.CMOT', 'thread_object_methods', exe_suffix)
    ok &= build_and_run(b, 'tests/conformance/cmotive_string_wide_object_methods.CMOT', 'string_wide_object_methods', exe_suffix)

    ok &= run([str(b/'cmotive'), '--emit-c', 'tests/conformance/cmotive_dynamic_struct.CMOT', '-o', 'build/dynamic_struct.c'])
    if ok:
        ctext = Path('build/dynamic_struct.c').read_text(encoding='utf-8')
        for token in ['typedef struct MyDynStruct', 'uint16_t d;', 'long double i;']:
            if token not in ctext:
                print('FAILED: Dynamic Struct generated C missing', token)
                ok = False
                break

    ok &= run_expect_failure([str(b/'cmotive'), 'tests/conformance/cmotive_invalid_base.CMOT', '-o', 'build/invalid_base' + exe_suffix])

    if ns.full:
        # Optional local expansion: compile/run representative remaining tests.
        more = [
            ('tests/conformance/cmotive_control_flow.CMTV','control',None),
            ('tests/conformance/cmotive_preprocessor.CMOT','preprocessor',None),
            ('tests/conformance/cmotive_sys_stl_template.CMOT','sys_stl_template',None),
            ('tests/conformance/cmotive_package_method_mangling.CMOT','package_method_mangling',None),
            ('tests/conformance/cmotive_keyword_synonyms.CMOT','keyword_synonyms',None),
            ('tests/conformance/cmotive_exception_destructor_unwind.CMOT','exception_destructor_unwind',None),
            ('tests/conformance/cmotive_constructor_type_overload.CMOT','constructor_type_overload',None),
            ('tests/conformance/cmotive_target_hit_object.CMOT','target_hit_object',None),
            ('tests/conformance/cmotive_target_hit_sender.CMOT','target_hit_sender',None),
        ]
        for src, out, inc in more:
            ok &= build_and_run(b, src, out, exe_suffix, inc)

    print('CMotive tests:', 'PASS' if ok else 'FAIL')
    return 0 if ok else 1

if __name__ == '__main__':
    raise SystemExit(main())
