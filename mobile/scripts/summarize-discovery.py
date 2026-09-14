#!/usr/bin/env python3
"""Passive reporting only: these diagnostics never enter the engine."""
from pathlib import Path
import csv,json,collections
root=Path(__file__).resolve().parents[1]/'docs/discovery'
report={};names=['Skiff','Thread','Whorl','Husk']
for name in names:
    rows=[];pedigree=[]
    for folder in sorted(root.glob('holdout-'+name+'-*')):
        rows+=list(csv.reader((folder/'summary.csv').open()))
        pedigree+=list(csv.DictReader((folder/'pedigree.csv').open()))
    early=[p for p in pedigree if p['mature']=='1' and 1<=int(p['generation'])<=3]
    mature=[p for p in pedigree if p['mature']=='1' and int(p['generation'])>0]
    surviving=[p for p in mature if int(p['last_seen'])>=599]
    report[name]={'trials':len(rows),'sustained':sum(int(r[3])>0 and int(r[7])>0 for r in rows),'births':sum(int(r[4]) for r in rows),'max_generation':max(int(r[5]) for r in rows),'early_recognizable':[sum(int(p['recognizable']) for p in early),len(early)],'all_recognizable':[sum(int(p['recognizable']) for p in mature),len(mature)],'final_recognizable':[sum(int(p['recognizable']) for p in surviving),len(surviving)],'changed_reproducing_adults':sum(int(p['changed'])>0 and int(p['mature_children'])>0 for p in mature),'descendant_sizes':sorted(set(int(p['cells']) for p in mature)),'profiles':{str(profile):{'sustained':sum(int(r[3])>0 and int(r[7])>0 for r in rows if int(r[1])==profile),'births':sum(int(r[4]) for r in rows if int(r[1])==profile)} for profile in [0,1,2,4,6,7]}}
# Adult counts are descriptive, not a fitness score. Rotated repeats additionally
# record actual physical-cell occupancy to expose the small-body counting bias.
competitions=[]
for folder in list(root.glob('holdout-portfolio-*'))+list(root.glob('rotation-*')):
    if not (folder/'summary.csv').exists() or not (folder/'pedigree.csv').exists():continue
    rows=list(csv.reader((folder/'summary.csv').open()))
    if len(rows)!=4:continue
    # Genome cell count can exceed surviving body count; report adult counts
    # directly and do not mislabel expected genotype size as physical biomass.
    adults=[int(r[3]) for r in rows];leader=max(range(4),key=lambda i:adults[i]) if max(adults)>0 else -1
    competitions.append(dict(run=folder.name,adults=adults,births=[int(r[4]) for r in rows],leader=' + '.join(names[i] for i,a in enumerate(adults) if a==max(adults)) if leader>=0 else 'extinct'))
report['competition']=competitions
biomass=[]
for folder in root.glob('rotation-*'):
    if not (folder/'summary.csv').exists():continue
    rows=list(csv.reader((folder/'summary.csv').open()))
    if len(rows)!=4 or any(len(r)<9 for r in rows):continue
    cells=[int(r[8]) for r in rows];leader=max(range(4),key=lambda i:cells[i]) if max(cells)>0 else -1
    biomass.append(dict(run=folder.name,cells=cells,leader=names[leader] if leader>=0 else 'extinct',fraction=max(cells)/sum(cells) if sum(cells) else 0))
report['physical_competition']=biomass
report['physical_leaders']=dict(collections.Counter(b['leader'] for b in biomass))
report['competition_leaders']=dict(collections.Counter(c['leader'] for c in competitions))
report['competition_survival']={name:sum(c['adults'][i]>0 for c in competitions) for i,name in enumerate(names)}
errors=[float(p.read_text().split('=')[1]) for p in root.glob('*/integrity.txt')]
report['maximum_energy_error']=max(errors)
histories=list(root.glob('*/history.csv'));elapsed=0
for p in histories:
    rows=list(csv.DictReader(p.open()))
    if rows:elapsed+=max(int(r['second']) for r in rows)
report['recorded_runs']=len(histories);report['recorded_simulated_hours_lower_bound']=elapsed/3600
(root/'evidence.json').write_text(json.dumps(report,indent=2)+'\n')
for name in names:print(name,report[name])
print('competition leaders',report['competition_leaders'],'survival',report['competition_survival'])
print('runs',len(histories),'observed hours',elapsed/3600,'energy error',max(errors))
