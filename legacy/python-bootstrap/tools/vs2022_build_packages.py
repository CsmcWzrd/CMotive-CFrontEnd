#!/usr/bin/env python3
"""Visual Studio 2022 build helper for CMotive package/tool outputs.

This script is invoked by vs2022/CMotive.Packages.vcxproj. It intentionally
places build products under:

  CMotive/build/vs2022/packages/<Configuration>-<Platform>/

Outputs include copied compiler tool entrypoints, package-manager object/library
when a native Windows toolchain is available, and an output manifest so Visual
Studio users can see exactly what was produced.
"""
from __future__ import annotations
import argparse, os, shutil, subprocess, sys
from pathlib import Path

CPP_SOURCES = [Path('src/packages/package_manager.cpp')]
TOOL_SOURCES = [Path('tools/cmotive.py'), Path('tools/cmotivepp.py')]

def norm(p: Path) -> Path:
    return p.expanduser().resolve()

def run(cmd: list[str], allow_fail: bool = False) -> bool:
    print('+ ' + ' '.join(str(x) for x in cmd))
    proc = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    if proc.stdout:
        print(proc.stdout, end='' if proc.stdout.endswith('\n') else '\n')
    if proc.returncode != 0:
        if allow_fail:
            return False
        raise SystemExit(proc.returncode)
    return True

def find_tool(names: list[str]) -> str | None:
    for n in names:
        found = shutil.which(n)
        if found:
            return found
    return None

def out_root(root: Path, configuration: str, platform: str) -> Path:
    return root / 'build' / 'vs2022' / 'packages' / f'{configuration}-{platform}'

def copy_tools(root: Path, out: Path, produced: list[Path]) -> None:
    bindir = out / 'bin'
    bindir.mkdir(parents=True, exist_ok=True)
    for rel in TOOL_SOURCES:
        src = root / rel
        if src.exists():
            dst = bindir / src.name
            shutil.copy2(src, dst)
            produced.append(dst)
    cmotive = bindir / 'cmotive.py'
    alias = bindir / 'cmotive++.py'
    if cmotive.exists():
        shutil.copy2(cmotive, alias)
        produced.append(alias)

def compile_package_manager(root: Path, out: Path, produced: list[Path]) -> None:
    objdir = out / 'obj'
    objdir.mkdir(parents=True, exist_ok=True)
    cpp_files = [root / p for p in CPP_SOURCES if (root / p).exists()]
    if not cpp_files:
        return
    cl = find_tool(['cl.exe', 'cl'])
    clangcl = find_tool(['clang-cl.exe', 'clang-cl'])
    libtool = find_tool(['lib.exe', 'lib'])
    objs: list[Path] = []
    if cl or clangcl:
        compiler = cl or clangcl
        for cpp in cpp_files:
            obj = objdir / (cpp.stem + '.obj')
            ok = run([compiler, '/nologo', '/EHsc', '/std:c++17', '/c', str(cpp), '/Fo:' + str(obj)], allow_fail=True)
            if ok and obj.exists():
                objs.append(obj); produced.append(obj)
        if objs and libtool:
            libpath = out / 'cmotive-packages.lib'
            ok = run([libtool, '/NOLOGO', '/OUT:' + str(libpath)] + [str(o) for o in objs], allow_fail=True)
            if ok and libpath.exists(): produced.append(libpath)
    else:
        # Non-Windows verification fallback: write an explicit note instead of silently succeeding.
        note = out / 'native-toolchain-not-found.txt'
        note.write_text('cl.exe/clang-cl.exe was not found; Visual Studio Developer Command Prompt is required for cmotive-packages.lib.\n', encoding='utf-8')
        produced.append(note)

def write_manifest(out: Path, produced: list[Path], root: Path) -> None:
    manifest = out / 'BUILD_OUTPUTS.txt'
    lines = ['CMotive VS2022 package/tool build outputs', '', 'Output root: ' + str(out), '']
    for p in produced:
        lines.append(str(p))
    if not produced:
        lines.append('(no outputs produced)')
    manifest.write_text('\n'.join(lines) + '\n', encoding='utf-8')
    stamp = out / 'cmotive-packages.stamp'
    stamp.write_text('ok\n', encoding='utf-8')

def build(root: Path, configuration: str, platform: str) -> None:
    out = out_root(root, configuration, platform)
    out.mkdir(parents=True, exist_ok=True)
    produced: list[Path] = []
    copy_tools(root, out, produced)
    compile_package_manager(root, out, produced)
    write_manifest(out, produced, root)
    print('CMotive VS2022 outputs written under: ' + str(out))

def clean(root: Path, configuration: str, platform: str) -> None:
    out = out_root(root, configuration, platform)
    if out.exists():
        shutil.rmtree(out)
        print('Removed ' + str(out))
    else:
        print('Nothing to clean: ' + str(out))

def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument('--mode', choices=['build','rebuild','clean'], required=True)
    ap.add_argument('--cmotive-root', required=True)
    ap.add_argument('--configuration', default='Debug')
    ap.add_argument('--platform', default='x64')
    ns = ap.parse_args(argv)
    root = norm(Path(ns.cmotive_root))
    if ns.mode == 'clean': clean(root, ns.configuration, ns.platform)
    elif ns.mode == 'rebuild': clean(root, ns.configuration, ns.platform); build(root, ns.configuration, ns.platform)
    else: build(root, ns.configuration, ns.platform)
    return 0
if __name__ == '__main__':
    raise SystemExit(main(sys.argv[1:]))
