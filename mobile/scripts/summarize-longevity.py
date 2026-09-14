from pathlib import Path
import csv,json,collections
root=Path(__file__).resolve().parents[1]/'docs/longevity';names=['Skiff','Thread','Whorl','Husk'];results=[]
for folder in sorted(root.iterdir()):
 if not (folder/'completion.json').exists():continue
 completion=json.loads((folder/'completion.json').read_text())
 if completion['exit_code']:raise RuntimeError(folder)
 rows=list(csv.DictReader((folder/'longevity.csv').open()));by=collections.defaultdict(list)
 for r in rows:by[int(r['second'])].append(r)
 final=by[max(by)];first_single=next((t for t,rr in sorted(by.items()) if sum(int(r['physical_cells'])>0 for r in rr)<=1),None)
 prev=by[max(t for t in by if t<=6300)]
 births_last15=sum(int(r['births']) for r in final)-sum(int(r['births']) for r in prev)
 endpoint={names[int(r['family'])]:{k:int(v) for k,v in r.items() if k not in ['second','family']} for r in final}
 if (folder/'pedigree.csv').exists():
  lives=list(csv.DictReader((folder/'pedigree.csv').open()));last=[r for r in lives if int(r['last_seen'])>=7199 and r['mature']=='1'];sizes=dict(collections.Counter(r['cells'] for r in last))
 else:sizes={}
 result=dict(run=folder.name,first_single_ancestry_second=first_single,final=endpoint,births_last15_minutes=births_last15,last_second_adult_sizes=sizes,energy_error=float((folder/'integrity.txt').read_text().split('=')[1]),wall_seconds=completion['wall_seconds'])
 results.append(result)
 print(folder.name,'single at',first_single,'late births',births_last15,'sizes',sizes)
(root/'evidence.json').write_text(json.dumps(results,indent=2)+'\n')
