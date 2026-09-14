#!/usr/bin/env python3
"""Passive, ancestry-aware report; incomplete runs are excluded explicitly."""
import csv,json,pathlib,statistics,sys,collections

def read(p):
 with p.open() as f:return list(csv.DictReader(f))
def mean(values):return statistics.mean(values) if values else None

def analyze(stem):
 log=pathlib.Path(str(stem)+'.log').read_text()
 if 'outcome_hash=' not in log:return None
 meta=dict(x.split('=',1) for x in log.split() if '=' in x)
 pop=read(pathlib.Path(str(stem)+'-population.csv'));lives=read(pathlib.Path(str(stem)+'-lives.csv'))
 end=float(meta['simulated_seconds']);result=dict(seed=int(meta['seed']),regime=meta['regime'],seconds=end,wall_seconds=float(meta['wall_seconds']),energy_error=float(meta['maximum_energy_error']),hash=meta['outcome_hash'],ancestries={})
 for a in ['0','1']:
  snapshots={float(x['seconds']):x for x in pop if x['ancestry']==a};final=snapshots[max(snapshots)]
  descendants=[x for x in lives if x['ancestry']==a and x['parent']!='4294967295']
  early=[x for x in descendants if float(x['created'])<=600]
  loss=[x for x in descendants if int(x['genetic_motors'])==0 or int(x['genetic_sensors'])==0]
  bymutation=collections.defaultdict(list)
  for x in early:bymutation[x['mutation']].append(x)
  last_present=max((t for t,x in snapshots.items() if int(x['mature']) or int(x['juvenile'])),default=0)
  avg=lambda xs,k:mean([float(x[k]) for x in xs])
  result['ancestries'][a]=dict(final_mature=int(final['mature']),final_cells=int(final['cells']),final_inherited_cells=int(final['inherited_cells']),final_intact=int(final['intact_sensor_motor_bodies']),final_moving=int(final['moving_bodies']),last_observed_presence=last_present,created=len(descendants),born=sum(float(x['mature_at'])>=0 for x in descendants),juvenile_losses=sum(float(x['mature_at'])<0 and float(x['ended'])>=0 for x in descendants),mean_early_mature_children=avg(early,'mature_children'),early_cohort_n=len(early),inherited_sensor_or_motor_absence=len(loss),uptake_per_cell_second=sum(float(x['observed_uptake']) for x in descendants)/max(1e-9,sum(float(x['cell_seconds']) for x in descendants)),starving_cell_fraction=sum(float(x['starving_cell_seconds']) for x in descendants)/max(1e-9,sum(float(x['cell_seconds']) for x in descendants)),cap_encountered=final['cap']=='1',by_mutation={m:dict(n=len(xs),mature_fraction=mean([float(x['mature_at'])>=0 for x in xs]),mean_mature_children=avg(xs,'mature_children')) for m,xs in bymutation.items()})
 return result

if __name__=='__main__':
 directory=pathlib.Path(sys.argv[1]);results=[]
 for p in sorted(directory.glob('*-population.csv')):
  r=analyze(pathlib.Path(str(p).removesuffix('-population.csv')))
  if r:results.append(r)
 (directory/'summary.json').write_text(json.dumps(results,indent=2)+'\n')
 for regime in ['fixed','small','full']:
  group=[x for x in results if x['regime']==regime]
  if not group:continue
  print(regime,'completed',len(group),'forager ancestry surviving',sum(x['ancestries']['1']['final_mature']>0 for x in group),'moving',sum(x['ancestries']['1']['final_moving']>0 for x in group),'median forager mature',statistics.median(x['ancestries']['1']['final_mature'] for x in group),'median early offspring',mean([x['ancestries']['1']['mean_early_mature_children'] for x in group if x['ancestries']['1']['mean_early_mature_children'] is not None]))
