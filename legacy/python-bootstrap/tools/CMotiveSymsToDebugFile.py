#!/usr/bin/env python3
"""CMotiveSymsToDebugFile

Create a human-readable CMotive debug-symbol file from a compiled executable,
shared library, static library, or object file.  The tool combines native symbol
addresses from nm/llvm-nm/objdump with compiler-generated CMotive prototype
metadata when it is available.
"""
from __future__ import annotations
import argparse
import json
import os
import re
import shutil
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path
from typing import Dict, Iterable, List, Tuple

FUNCTION_TYPES = set('TtWw')


def default_output(input_path: Path) -> Path:
    return input_path.with_name(input_path.name + '_cmot_debugsymbols.syms')


def default_metadata_candidates(input_path: Path) -> List[Path]:
    return [
        input_path.with_suffix(input_path.suffix + '.cmotive.debug.json'),
        input_path.with_name(input_path.name + '.cmotive.debug.json'),
        input_path.with_name(input_path.stem + '.cmotive.debug.json'),
    ]


def run_tool(cmd: List[str]) -> Tuple[int, str, str]:
    try:
        p = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, timeout=30)
        return p.returncode, p.stdout, p.stderr
    except Exception as exc:  # pragma: no cover - defensive in build tools
        return 127, '', str(exc)


def parse_nm_text(text: str) -> Dict[str, int]:
    symbols: Dict[str, int] = {}
    # Accept GNU/LLVM nm forms:
    # 0000000000001139 T main
    # 00000000 T _main
    #                  U printf
    pat = re.compile(r'^\s*([0-9A-Fa-f]{1,16})\s+([A-Za-z])\s+(.+?)\s*$')
    for line in text.splitlines():
        m = pat.match(line)
        if not m:
            continue
        typ = m.group(2)
        if typ not in FUNCTION_TYPES:
            continue
        name = m.group(3).strip()
        # Strip common decoration used by some targets while preserving exact name too.
        addr = int(m.group(1), 16)
        symbols.setdefault(name, addr)
        if name.startswith('_'):
            symbols.setdefault(name[1:], addr)
    return symbols


def native_symbols(input_path: Path) -> Dict[str, int]:
    for tool in ['llvm-nm', 'nm']:
        exe = shutil.which(tool)
        if not exe:
            continue
        rc, out, _ = run_tool([exe, '-n', str(input_path)])
        if rc == 0:
            syms = parse_nm_text(out)
            if syms:
                return syms
    objdump = shutil.which('objdump') or shutil.which('llvm-objdump')
    if objdump:
        rc, out, _ = run_tool([objdump, '-t', str(input_path)])
        if rc == 0:
            return parse_nm_text(out)
    return {}


def read_metadata(paths: Iterable[Path]) -> Tuple[dict, List[dict]]:
    merged_meta = {'inputs': [], 'debug_level': 0, 'optimization': '', 'target_arch': ''}
    records: List[dict] = []
    seen = set()
    for path in paths:
        if not path or not Path(path).exists():
            continue
        try:
            data = json.loads(Path(path).read_text(encoding='utf-8'))
        except Exception:
            continue
        for k in ['debug_level', 'optimization', 'target_arch']:
            if data.get(k):
                merged_meta[k] = data.get(k)
        for item in data.get('inputs', []) or []:
            if item not in merged_meta['inputs']:
                merged_meta['inputs'].append(item)
        for rec in data.get('symbols', []) or []:
            sym = rec.get('symbol')
            if not sym or sym in seen:
                continue
            seen.add(sym)
            records.append(rec)
    return merged_meta, records


def format_offset(value: int | None) -> str:
    if value is None:
        return '0x????????????????'
    return '0x%016X' % (value & ((1 << 64) - 1))


def write_syms(input_path: Path, output_path: Path, metadata: dict, records: List[dict], native: Dict[str, int]) -> None:
    output_path.parent.mkdir(parents=True, exist_ok=True)
    now = datetime.now(timezone.utc).isoformat()
    lines = [
        '# CMotive debug symbols',
        f'# input: {input_path}',
        f'# generated_utc: {now}',
        f'# debug_level: {metadata.get("debug_level", 0)}',
        f'# optimization: {metadata.get("optimization", "")}',
        f'# target_arch: {metadata.get("target_arch", "")}',
        '# format: offset64 | symbol | kind | package | class | source | prototype | c_prototype',
    ]
    if metadata.get('inputs'):
        lines.append('# sources: ' + ', '.join(metadata.get('inputs', [])))
    lines.append('')

    emitted = set()
    for rec in sorted(records, key=lambda r: (native.get(r.get('symbol',''), 1 << 63), r.get('symbol',''))):
        sym = rec.get('symbol', '')
        emitted.add(sym)
        line = ' | '.join([
            format_offset(native.get(sym)),
            sym,
            rec.get('kind', ''),
            rec.get('package', ''),
            rec.get('class', ''),
            rec.get('source', ''),
            rec.get('prototype', ''),
            rec.get('c_prototype', ''),
        ])
        lines.append(line.rstrip())

    # If no compiler metadata exists, still expose native function symbols.
    for sym, addr in sorted(native.items(), key=lambda x: (x[1], x[0])):
        if sym in emitted:
            continue
        lines.append(' | '.join([format_offset(addr), sym, 'native', '', '', '', '<prototype unavailable>', '']))

    output_path.write_text('\n'.join(lines) + '\n', encoding='utf-8')


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(prog='CMotiveSymsToDebugFile')
    ap.add_argument('input', help='executable, library, or object file')
    ap.add_argument('-m', '--metadata', action='append', default=[], help='compiler metadata JSON file; may be repeated')
    ap.add_argument('-o', '--output', help='output .syms file; default is Filename_cmot_debugsymbols.syms')
    ap.add_argument('--strict', action='store_true', help='fail if no CMotive metadata is available')
    ns = ap.parse_args(argv)

    input_path = Path(ns.input)
    if not input_path.exists():
        ap.error(f'input file not found: {input_path}')

    meta_paths = [Path(p) for p in ns.metadata] + default_metadata_candidates(input_path)
    metadata, records = read_metadata(meta_paths)
    if ns.strict and not records:
        print('CMotiveSymsToDebugFile: no CMotive metadata found', file=sys.stderr)
        return 2
    syms = native_symbols(input_path)
    output = Path(ns.output) if ns.output else default_output(input_path)
    write_syms(input_path, output, metadata, records, syms)
    print(output)
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
