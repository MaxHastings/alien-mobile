#!/usr/bin/env python3
"""Bounded independent engine runs; preserves commands, summaries and raw histories."""
import concurrent.futures, subprocess, pathlib, json, sys
root=pathlib.Path(__file__).resolve().parents[1]
out=root/'docs/discovery'
exe=root/'build-release/AlienMobileCatalogDiscovery'
stage=sys.argv[1] if len(sys.argv)>1 else 'evolve'
def run(job):
    name,mode,seed,profile,seconds,manifest,ablate=job
    folder=out/name
    folder.mkdir(parents=True,exist_ok=True)
    cmd=list(map(str,[exe,mode,seed,profile,seconds,manifest,folder,ablate]))
    (folder/'command.json').write_text(json.dumps(cmd))
    p=subprocess.run(cmd,capture_output=True,text=True)
    (folder/'summary.csv').write_text(p.stdout)
    (folder/'stderr.txt').write_text(p.stderr)
    if p.returncode:raise RuntimeError((name,p.returncode,p.stderr))
    print(name,p.stdout.strip(),flush=True)
    return p.stdout
jobs=[]
if stage=='evolve':
    for dna in sorted((out/'baseline').glob('*.dna')):
        manifest=out/(dna.stem+'.list');manifest.write_text(str(dna)+'\n')
        for profile in range(3):
            seed=201+profile*37
            jobs.append((f'evolve-{dna.stem}-{seed}','evolve',seed,profile,600,manifest,'none'))
elif stage=='rotation':
    for profile in [0,2,6,7]:
        for seed in range(5100,5106):
            jobs.append((f'rotation-{profile}-{seed}','assay',seed,profile,600,out/'finalists/portfolio.list','none'))
elif stage=='final-mechanism':
    for ablate in ['none','motor']:
        for profile in [0,1,2]:
            for seed in [1223,1877]:
                jobs.append((f'final-mechanism-Whorl-{ablate}-{profile}-{seed}','fixed',seed,profile,300,out/'finalists/Whorl.list',ablate))
elif stage=='holdout':
    for manifest in sorted((out/'finalists').glob('*.list')):
        for profile in [0,1,2,4,6,7]:
            for seed in [4001,7003,9007]:
                jobs.append((f'holdout-{manifest.stem}-{profile}-{seed}','assay',seed,profile,600,manifest,'none'))
elif stage=='crown':
    for manifest in sorted((out/'crown-refine').glob('*.list')):
        for profile in [0,1,2,4]:
            jobs.append((f'crown-{manifest.stem}-{profile}','fixed',809,profile,300,manifest,'none'))
elif stage=='reference':
    for name in ['Dart-ancestor','Ribbon-ancestor','Crown-ancestor','Vault-ancestor']:
        for profile in [0,1,2,4]:
            jobs.append((f'reference-{name}-{profile}','fixed',809,profile,300,out/'shortlist'/(name+'.list'),'none'))
elif stage=='storage':
    for name in ['Vault-d1','Crown-d2']:
        for ablate in ['none','depot']:
            for seed in [809,1223,1877]:
                jobs.append((f'storage-{name}-{ablate}-{seed}','fixed',seed,6,600,out/'shortlist'/(name+'.list'),ablate))
elif stage=='ablation':
    for name,ablates in {'Contractile-d0':['none','tail'], 'Crown-d2':['none','motor','depot'], 'Crown-d1':['none','motor','arms'], 'Dart-d0':['none','sensor','motor'], 'Dart-d1':['none','motor'], 'Ribbon-d0':['none','length','motor'], 'Ribbon-d1':['none','length','motor'], 'Vault-d1':['none','depot']}.items():
        manifest=out/'shortlist'/(name+'.list')
        for ablate in ablates:
            for profile in [0,1,2,3]:
                seed=809
                jobs.append((f'ablation-{name}-{ablate}-{profile}','fixed',seed,profile,300,manifest,ablate))
elif stage=='assay':
    manifests=sorted((out/'shortlist').glob('*.list'))
    for manifest in manifests:
        for profile in range(5):
            for seed in [809,1223]:
                jobs.append((f'assay-{manifest.stem}-{profile}-{seed}','assay',seed,profile,360,manifest,'none'))
with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:
    results=list(pool.map(run,jobs))
(out/(stage+'-summary.csv')).write_text('seed,profile,candidate,final_mature,births,max_generation,changed_births,late_births,final_cells\n'+''.join(results))
