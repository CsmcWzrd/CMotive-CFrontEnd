#!/usr/bin/env python3
from __future__ import annotations
import argparse, os, shutil, subprocess, sys
from pathlib import Path
SOURCE_EXTS = {'.CMOT', '.CMTV', '.cmot', '.cmtv'}

def norm(p: Path) -> Path: return p.expanduser().resolve()
def find_python() -> str: return sys.executable or 'python'
def find_tool(names):
    for n in names:
        f = shutil.which(n)
        if f: return f
    return None

def package_sources(examples_root: Path):
    pkg = examples_root/'packages'
    return sorted(p for p in pkg.rglob('*') if p.is_file() and p.suffix in SOURCE_EXTS) if pkg.exists() else []
def manifests(examples_root: Path):
    pkg = examples_root/'packages'
    return sorted(p for p in pkg.rglob('*.cmotpkg') if p.is_file()) if pkg.exists() else []
def compiler_path(cmotive_root: Path):
    for c in [cmotive_root/'tools'/'cmotive.py', cmotive_root/'build'/'bin'/'cmotive', cmotive_root/'tools'/'cmotive']:
        if c.exists(): return c
    raise SystemExit('CMotive compiler not found. Expected tools/cmotive.py under: '+str(cmotive_root))
def out_root(examples_root: Path, configuration: str, platform: str): return examples_root/'build'/'vs2022'/'packages'/f'{configuration}-{platform}'
def obj_path(build_dir: Path, examples_root: Path, src: Path): return build_dir/'obj'/src.relative_to(examples_root).with_suffix('.obj' if os.name == 'nt' else '.o')
def run(cmd, allow_fail=False):
    print('+ '+' '.join(str(x) for x in cmd))
    proc=subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    if proc.stdout: print(proc.stdout, end='' if proc.stdout.endswith('\n') else '\n')
    if proc.returncode != 0:
        if allow_fail: return False
        raise SystemExit(proc.returncode)
    return True

def validate(examples_root: Path, cmotive_root: Path):
    comp=compiler_path(cmotive_root); srcs=package_sources(examples_root)
    if not srcs: raise SystemExit('No package sources found under '+str(examples_root/'packages'))
    print('CMotive compiler: '+str(comp)); print('Package sources: '+str(len(srcs))); print('Package manifests: '+str(len(manifests(examples_root))))
    return srcs

def make_host(build_dir: Path, src_count: int, manifest_count: int, produced):
    host_c=build_dir/'cmotive_langexamples_package_host.c'
    host_c.write_text('#include <stdio.h>\nint main(void){printf("CMotive Lang-Examples packages: %d source(s), %d manifest(s)\\n", '+str(src_count)+', '+str(manifest_count)+');return 0;}\n', encoding='utf-8')
    produced.append(host_c)
    cl=find_tool(['cl.exe','cl']) or find_tool(['clang-cl.exe','clang-cl'])
    if cl:
        exe=build_dir/'cmotive-langexamples-package-host.exe'
        if run([cl, '/nologo', str(host_c), '/Fe:'+str(exe)], allow_fail=True) and exe.exists():
            produced.append(exe)
        else:
            note=build_dir/'native-toolchain-host-build-failed.txt'
            note.write_text('A C/C++ compiler was found, but the package-host executable could not be built in this environment. In VS2022, build from a Developer Command Prompt with Windows SDK headers installed. Package object generation still completed.\n', encoding='utf-8')
            produced.append(note)
    else:
        note=build_dir/'native-toolchain-not-found.txt'
        note.write_text('cl.exe/clang-cl.exe was not found; open from a VS2022 Developer Command Prompt to build the package-host executable.\n', encoding='utf-8')
        produced.append(note)

def build(examples_root: Path, cmotive_root: Path, configuration: str, platform: str):
    srcs=validate(examples_root, cmotive_root); comp=compiler_path(cmotive_root); build_dir=out_root(examples_root, configuration, platform); build_dir.mkdir(parents=True, exist_ok=True)
    produced=[]
    for src in srcs:
        out=obj_path(build_dir, examples_root, src); out.parent.mkdir(parents=True, exist_ok=True)
        run([find_python(), str(comp), '-c', str(src), '-o', str(out)])
        if out.exists(): produced.append(out)
    for m in manifests(examples_root):
        dst=build_dir/'manifests'/m.relative_to(examples_root/'packages'); dst.parent.mkdir(parents=True, exist_ok=True); shutil.copy2(m,dst); produced.append(dst)
    make_host(build_dir, len(srcs), len(manifests(examples_root)), produced)
    (build_dir/'BUILD_OUTPUTS.txt').write_text('CMotive Lang-Examples VS2022 package build outputs\n\nOutput root: '+str(build_dir)+'\n\n'+'\n'.join(str(p) for p in produced)+'\n', encoding='utf-8')
    (build_dir/'packages.stamp').write_text('ok\n', encoding='utf-8')
    print('Lang-Examples VS2022 outputs written under: '+str(build_dir))

def clean(examples_root: Path, configuration: str, platform: str):
    path=out_root(examples_root, configuration, platform)
    if path.exists(): shutil.rmtree(path); print('Removed '+str(path))
    else: print('Nothing to clean: '+str(path))

def main(argv):
    ap=argparse.ArgumentParser(); ap.add_argument('--mode', choices=['build','rebuild','clean','validate'], required=True); ap.add_argument('--examples-root', required=True); ap.add_argument('--cmotive-root', required=True); ap.add_argument('--configuration', default='Debug'); ap.add_argument('--platform', default='x64')
    ns=ap.parse_args(argv); examples_root=norm(Path(ns.examples_root)); cmotive_root=norm(Path(ns.cmotive_root))
    if ns.mode=='clean': clean(examples_root, ns.configuration, ns.platform)
    elif ns.mode=='validate': validate(examples_root, cmotive_root)
    elif ns.mode=='rebuild': clean(examples_root, ns.configuration, ns.platform); build(examples_root, cmotive_root, ns.configuration, ns.platform)
    else: build(examples_root, cmotive_root, ns.configuration, ns.platform)
    return 0
if __name__ == '__main__': raise SystemExit(main(sys.argv[1:]))
