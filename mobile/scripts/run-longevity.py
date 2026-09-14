#!/usr/bin/env python3
"""Long, unaltered playable worlds and paired mutation-disabled controls."""
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import subprocess,json,time,sys
root=Path(__file__).resolve().parents[1];out=root/'docs/longevity';out.mkdir(exist_ok=True)
def run(job):
    seed,mode,profile,label=job;folder=out/f'{label}-{seed}';folder.mkdir(exist_ok=True)
    cmd=[str(root/'build-release/AlienMobileCatalogDiscovery'),mode,str(seed),str(profile),'7200',str(root/'docs/discovery/finalists/portfolio.list'),str(folder),'none']
    (folder/'command.json').write_text(json.dumps(cmd,indent=2))
    start=time.monotonic();p=subprocess.run(cmd,capture_output=True,text=True)
    (folder/'summary.csv').write_text(p.stdout);(folder/'stderr.txt').write_text(p.stderr)
    (folder/'completion.json').write_text(json.dumps(dict(exit_code=p.returncode,wall_seconds=time.monotonic()-start)))
    if p.returncode:raise RuntimeError((folder,p.returncode,p.stderr))
    print(folder.name,p.stdout.strip(),flush=True)
stage=sys.argv[1] if len(sys.argv)>1 else 'paired'
if stage=='paired':jobs=[(seed,mode,0,mode) for seed in [6100,6101,6102,6103] for mode in ['long-game','long-fixed']]
elif stage=='capacity':jobs=[(seed,'long-game',8,'cap960') for seed in [6100,6101]]
elif stage=='drift':jobs=[(seed,'long-game',9,'drift6') for seed in [6100,6101]]
else:raise SystemExit('stage must be paired, capacity or drift')
with ThreadPoolExecutor(max_workers=4 if stage=='paired' else 2) as pool:list(pool.map(run,jobs))
