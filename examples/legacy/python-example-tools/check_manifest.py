#!/usr/bin/env python3
from pathlib import Path
import json
root=Path(__file__).resolve().parents[1]
rows=[json.loads(x) for x in (root/'manifests/examples.jsonl').read_text().splitlines() if x.strip()]
missing=[r['file'] for r in rows if not (root/r['file']).exists()]
if missing:
    raise SystemExit('missing files: '+', '.join(missing))
print(f'manifest ok: {len(rows)} examples')
