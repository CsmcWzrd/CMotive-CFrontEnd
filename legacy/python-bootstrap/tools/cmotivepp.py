#!/usr/bin/env python3
import argparse, sys
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
from cmotive.preprocessor import Preprocessor

def main(argv=None):
    ap = argparse.ArgumentParser(prog='cmotivepp')
    ap.add_argument('-I', dest='includes', action='append', default=[])
    ap.add_argument('-D', dest='defines', action='append', default=[])
    ap.add_argument('-o')
    ap.add_argument('input')
    ns = ap.parse_args(argv)
    defines = {}
    for d in ns.defines:
        if '=' in d:
            k, v = d.split('=', 1)
        else:
            k, v = d, '1'
        defines[k] = v
    pp = Preprocessor(ns.includes, defines)
    text = pp.process(ns.input)
    if ns.o:
        Path(ns.o).parent.mkdir(parents=True, exist_ok=True)
        Path(ns.o).write_text(text, encoding='utf-8')
    else:
        sys.stdout.write(text)
    return 0

if __name__ == '__main__':
    raise SystemExit(main())
