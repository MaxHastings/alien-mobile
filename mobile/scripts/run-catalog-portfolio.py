import subprocess,concurrent.futures,pathlib,sys
root=pathlib.Path(__file__).resolve().parents[1]
phase=sys.argv[1] if len(sys.argv)>1 else 'first'
jobs=[(p,s,i,0,0,180) for p in range(5) for s in [17,43] for i in range(5)]
if phase=='ablation':jobs=[(p,s,i,a,0,180) for p in range(5) for s in [17,43] for i in range(5) for a in [1]]
if phase=='mixed':jobs=[(p,s,-1,0,m,300) for p in range(5) for s in [17,43,71] for m in [0,1]]
if phase=='mutation':jobs=[(p,s,i,0,1,360) for p in [0,2,3,4] for s in [17,43] for i in range(5)]
def run(job):
 r=subprocess.run([str(root/'build-catalog/AlienMobileCatalogPortfolio'),*map(str,job)],capture_output=True,text=True)
 if r.returncode:raise RuntimeError((job,r.stderr,r.stdout))
 return r.stdout
out=root/'docs/catalog-ownership'/f'{phase}.csv'
with out.open('w') as f:
 f.write('profile,seed,name,ablation,mutation,seconds,mature,cells,births,organism_seconds,generation,variants,absorbed,attacked,digested,recognizable,min_body,max_body\n')
 with concurrent.futures.ThreadPoolExecutor(max_workers=6) as ex:
  for n,result in enumerate(ex.map(run,jobs)):
   f.write(result);f.flush();print(f'{n+1}/{len(jobs)}',flush=True)
