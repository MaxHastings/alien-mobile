#!/usr/bin/env python3
"""Read-only summaries of completed tension research runs, including failures."""
import csv, json, pathlib, collections, math, sys

def rows(p):
 with p.open() as f:return list(csv.DictReader(f))
def analyze(stem):
 pop=rows(pathlib.Path(str(stem)+'-population.csv'));ph=rows(pathlib.Path(str(stem)+'-phenotypes.csv'));body=rows(pathlib.Path(str(stem)+'-bodies.csv'));reg=rows(pathlib.Path(str(stem)+'-regions.csv'))
 log=pathlib.Path(str(stem)+'.log').read_text().strip()
 if 'outcome_hash=' not in log:return None
 meta=dict(x.split('=',1) for x in log.split() if '=' in x)
 end=float(pop[-1]['seconds']);by=collections.defaultdict(dict)
 for x in ph:by[float(x['seconds'])][x['id']]=x
 final=by.get(end,{})
 distribution=lambda group:dict(sorted(collections.Counter(int(x['cells']) for x in group.values()).items()))
 first_region={};last_region={};migrations=[]
 for x in body:
  if x['cell']!='0' or x['mature']!='1':continue
  pos=float(x['x']),float(x['y']);distances=[]
  for b in range(4):
   center=(-.5+(1 if b%2 else -1)*7,-1.5+(1 if b//2 else -1)*7)
   d=[pos[k]-center[k] for k in range(2)];d=[v-32*round(v/32) for v in d];distances.append(math.hypot(*d))
  b=min(range(4),key=distances.__getitem__)
  if distances[b]>2.8+.9*b:continue
  key=x['id'];previous=last_region.get(key)
  if previous is not None and previous!=b:migrations.append(dict(seconds=float(x['seconds']),id=key,lineage=x['lineage'],source=previous,destination=b))
  first_region.setdefault(key,b);last_region[key]=b
 persistent=[]
 lineage=pathlib.Path(str(stem)+'-lineages.csv')
 if lineage.exists():persistent=[x for x in rows(lineage) if float(x['last_mature_observation'])-float(x['first_mature_observation'])>=120]
 checkpoints={}
 for target in (300,1800,3600,18000):
  ts=[t for t in by if t<=target+.1]
  if ts and target<=end+.1:
   t=max(ts);group=by[t];checkpoints[str(target)]=dict(observed_seconds=t,bodies=distribution(group),moving_bodies=sum(float(x['paid_thrust'])>1e-6 for x in group.values()))
 return dict(stem=str(stem),seed=int(meta['seed']),seconds=end,survived=end>=1799 and int(pop[-1]['cells'])>0,mature=int(pop[-1]['mature']),births=int(pop[-1]['births']),generation=int(pop[-1]['mature_generation']),final_bodies=distribution(final),final_moving_bodies=sum(float(x['paid_thrust'])>1e-6 for x in final.values()),max_body=max((int(x['cells']) for x in ph),default=0),persistent_lineages=len(persistent),persistent_lineages_reaching_7_cells=sum(int(x['max_cells'])>=7 for x in persistent),energy_error=float(meta['maximum_energy_error']),cap_encountered=pop[-1]['cap']=='1',checkpoints=checkpoints,final_regions=[x for x in reg if float(x['seconds'])==end],observed_migrations=migrations)
if __name__=='__main__':
 directory=pathlib.Path(sys.argv[1]);result=[]
 for p in sorted(directory.glob('*-population.csv')):
  value=analyze(pathlib.Path(str(p).removesuffix('-population.csv')))
  if value:result.append(value)
 (directory/'tension-summary.json').write_text(json.dumps(result,indent=2)+'\n')
 for x in result:print(pathlib.Path(x['stem']).name,round(x['seconds']),x['final_bodies'],'moving',x['final_moving_bodies'],'migrations',len(x['observed_migrations']))
