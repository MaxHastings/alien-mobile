#!/usr/bin/env python3
"""Run declared deterministic scenarios; no selection or feedback into biology."""
import argparse, concurrent.futures, json, pathlib, subprocess, shutil, hashlib, tarfile
p=argparse.ArgumentParser();p.add_argument('directory');p.add_argument('--environment',default='beds');p.add_argument('--seconds',type=int,default=1800);p.add_argument('--seeds',type=int,default=20);p.add_argument('--first',type=int,default=1);p.add_argument('--founders',nargs='+',default=['primitive','garden']);p.add_argument('--workers',type=int,default=4);p.add_argument('--sample',type=int,default=30)
a=p.parse_args();out=pathlib.Path(a.directory);out.mkdir(parents=True,exist_ok=True)
executable=out/'research-runner'
shutil.copy2('mobile/build-release/AlienMobileEvolutionResearch',executable)
manifest=vars(a)|{'executable_sha256':hashlib.sha256(executable.read_bytes()).hexdigest()}
(out/'manifest.json').write_text(json.dumps(manifest,indent=2))
with tarfile.open(out/'source.tar.gz','w:gz') as archive:
 for source in ['mobile/core','mobile/tests/EvolutionResearch.cpp','mobile/CMakeLists.txt','mobile/scripts/run-tension-research.py']:
  archive.add(source,arcname=source)

def run(item):
 seed,founder=item;stem=out/f'{a.environment}-{founder}-{seed}'
 command=[str(executable),str(seed),str(a.seconds),str(stem),founder,a.environment,'360',str(a.sample)]
 with open(str(stem)+'.log','w') as log:r=subprocess.run(command,stdout=log,stderr=subprocess.STDOUT)
 return f'{stem}: exit {r.returncode}'
with concurrent.futures.ThreadPoolExecutor(max_workers=a.workers) as pool:
 for result in pool.map(run,[(s,f) for s in range(a.first,a.first+a.seeds) for f in a.founders]):print(result,flush=True)
