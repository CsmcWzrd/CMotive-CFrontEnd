#!/usr/bin/env python3
import argparse, json, os, shutil, subprocess, sys, tempfile
from pathlib import Path

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('--compiler', default=os.environ.get('CMOTIVE','cmotive'))
    ap.add_argument('--timeout', type=float, default=5.0)
    ap.add_argument('--keep-build', action='store_true')
    ns=ap.parse_args()
    root=Path(__file__).resolve().parents[1]
    rows=[json.loads(x) for x in (root/'manifests/examples.jsonl').read_text().splitlines() if x.strip()]
    build=Path(tempfile.mkdtemp(prefix='cmotive-examples-'))
    failures=[]
    try:
        for r in rows:
            src=root/r['file']
            exe=build/(src.stem + ('.exe' if os.name=='nt' else ''))
            cmd=[sys.executable, ns.compiler, str(src), '-o', str(exe)] if ns.compiler.endswith('.py') else [ns.compiler, str(src), '-o', str(exe)]
            c=subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            if c.returncode != 0 or c.stderr:
                failures.append({'file':r['file'],'phase':'compile','code':c.returncode,'stdout':c.stdout,'stderr':c.stderr})
                continue
            try:
                e=subprocess.run([str(exe)], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, timeout=ns.timeout)
            except subprocess.TimeoutExpired:
                failures.append({'file':r['file'],'phase':'run','code':'timeout','stdout':'','stderr':''})
                continue
            if e.returncode != r.get('expected_exit',0) or e.stderr:
                failures.append({'file':r['file'],'phase':'run','code':e.returncode,'stdout':e.stdout,'stderr':e.stderr})
        report=root/'VERIFY_REPORT.md'
        if failures:
            report.write_text('# CMotive examples verification report\n\nFAILED\n\n```json\n'+json.dumps(failures, indent=2)+'\n```\n')
            print(f'failed: {len(failures)} of {len(rows)}; see {report}', file=sys.stderr)
            return 1
        report.write_text(f'# CMotive examples verification report\n\nPASS: {len(rows)} examples compiled, linked, executed, exited 0, and produced no stderr.\n')
        print(f'PASS: {len(rows)} examples')
        return 0
    finally:
        if ns.keep_build:
            print('build dir:', build)
        else:
            shutil.rmtree(build, ignore_errors=True)
if __name__=='__main__':
    raise SystemExit(main())
