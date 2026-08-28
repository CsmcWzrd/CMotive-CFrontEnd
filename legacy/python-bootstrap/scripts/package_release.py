#!/usr/bin/env python3
import argparse, hashlib, json, tarfile, time, zipfile
from pathlib import Path
EXCLUDES={'build','dist','.git'}
def files(root):
    for p in sorted(root.rglob('*')):
        if p.is_file() and not any(part in EXCLUDES for part in p.relative_to(root).parts): yield p
def sha256(p):
    h=hashlib.sha256(); h.update(p.read_bytes()); return h.hexdigest()
def main():
    ap=argparse.ArgumentParser(); ap.add_argument('--root',default='.'); ap.add_argument('--out',default='dist'); ns=ap.parse_args(); root=Path(ns.root).resolve(); out=Path(ns.out); out.mkdir(parents=True,exist_ok=True)
    (root/'release'/'provenance').mkdir(parents=True,exist_ok=True)
    manifest={'name':'CMotive','version':(root/'VERSION').read_text().strip(),'created_utc':int(time.time()),'files':[{'path':str(p.relative_to(root)),'sha256':sha256(p)} for p in files(root)]}
    (root/'release'/'provenance'/'manifest.json').write_text(json.dumps(manifest,indent=2),encoding='utf-8')
    tar_path=out/'cmotive-source.tar.gz'; zip_path=out/'cmotive-source.zip'
    with tarfile.open(tar_path,'w:gz') as tf:
        for p in files(root): tf.add(p,arcname='cmotive-source/'+str(p.relative_to(root)))
    with zipfile.ZipFile(zip_path,'w',zipfile.ZIP_DEFLATED) as zf:
        for p in files(root): zf.write(p,'cmotive-source/'+str(p.relative_to(root)))
    print(tar_path); print(zip_path)
if __name__=='__main__': main()
