#!/usr/bin/env python3
"""Bounded checks of frozen catalog archives; no live fitness selection."""
import concurrent.futures, hashlib, json, pathlib, subprocess
root=pathlib.Path(__file__).resolve().parents[1]
out=root/'docs/playtest-pass/sessions';out.mkdir(parents=True,exist_ok=True)
exe=root/'build-release/AlienMobileCatalogDiscovery'
archive=root/'docs/discovery/finalists'
(out/'core-sha256.json').write_text(json.dumps({str(p.relative_to(root)):hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted((root/'core').rglob('*')) if p.is_file()},indent=2))
jobs=[]
for name,profile,seconds,ablation in [('Skiff',1,300,'sensor'),('Thread',0,300,'length'),('Whorl',0,300,'motor'),('Husk',6,600,'depot')]:
    for variant in ['none',ablation]:
        jobs.append((f'{name}-{variant}','fixed',1877,profile,seconds,name,variant))
for seed in [913,1913,2913]:jobs.append((f'default-{seed}','long-game',seed,0,600,'portfolio','none'))
jobs.append(('soak-3913','long-game',3913,0,1800,'portfolio','none'))
def run(job):
    name,mode,seed,profile,seconds,manifest,variant=job
    dest=out/name;dest.mkdir(exist_ok=True)
    cmd=list(map(str,[exe,mode,seed,profile,seconds,archive/(manifest+'.list'),dest,variant]))
    (dest/'command.json').write_text(json.dumps(cmd))
    p=subprocess.run(cmd,cwd=root,capture_output=True,text=True)
    (dest/'stdout.txt').write_text(p.stdout);(dest/'stderr.txt').write_text(p.stderr)
    (dest/'exit.json').write_text(json.dumps({'returncode':p.returncode}))
    print(name,p.returncode,p.stdout.strip(),flush=True)
    return {'name':name,'returncode':p.returncode,'summary':p.stdout}
with concurrent.futures.ThreadPoolExecutor(max_workers=2) as pool:results=list(pool.map(run,jobs))
(out/'results.json').write_text(json.dumps(results,indent=2))
if any(r['returncode'] for r in results):raise SystemExit(1)
