#!/usr/bin/env python3
"""Predeclared matched arms, immutable binary, preserved failures."""
import argparse,concurrent.futures,hashlib,json,pathlib,shutil,subprocess,tarfile
p=argparse.ArgumentParser();p.add_argument('directory');p.add_argument('--seconds',type=int,default=3600);p.add_argument('--seeds',type=int,default=20);p.add_argument('--first',type=int,default=1);p.add_argument('--workers',type=int,default=4);p.add_argument('--regimes',nargs='+',default=['fixed','small','full'])
a=p.parse_args();out=pathlib.Path(a.directory)
if out.exists() and any(out.iterdir()):raise SystemExit('Choose an empty output directory; results are never overwritten.')
out.mkdir(parents=True,exist_ok=True);exe=out/'competition-runner';shutil.copy2('mobile/build-release/AlienMobileCompetitionResearch',exe)
(out/'manifest.json').write_text(json.dumps(vars(a)|{'sha256':hashlib.sha256(exe.read_bytes()).hexdigest()},indent=2))
with tarfile.open(out/'source.tar.gz','w:gz') as archive:
 for src in ['mobile/core','mobile/tests/CompetitionResearch.h','mobile/tests/CompetitionResearch.cpp','mobile/tests/CompetitionObserverTests.cpp','mobile/CMakeLists.txt','mobile/scripts/run-competition-research.py']:
  archive.add(src,arcname=src)
def run(item):
 seed,regime=item;stem=out/f'{regime}-{seed}'
 with open(str(stem)+'.log','w') as log:
  result=subprocess.run([str(exe),str(seed),str(a.seconds),str(stem),regime,'60'],stdout=log,stderr=subprocess.STDOUT)
 return f'{regime}-{seed}: exit {result.returncode}'
with concurrent.futures.ThreadPoolExecutor(max_workers=a.workers) as pool:
 for r in pool.map(run,[(s,r) for s in range(a.first,a.first+a.seeds) for r in a.regimes]):print(r,flush=True)
