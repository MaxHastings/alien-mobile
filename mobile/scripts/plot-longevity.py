from pathlib import Path
import csv
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
root=Path(__file__).resolve().parents[1]/'docs/longevity';colors=['#f1af55','#69d75b','#58bdf0','#ab7af0'];names=['Skiff','Thread','Whorl','Husk']
fig,axes=plt.subplots(4,2,figsize=(13,11),sharex=True,sharey=True)
for row,seed in enumerate([6100,6101,6102,6103]):
 for col,mode in enumerate(['long-game','long-fixed']):
  ax=axes[row,col];p=root/f'{mode}-{seed}'/'longevity.csv'
  if not p.exists():continue
  rows=list(csv.DictReader(p.open()));times=sorted(set(int(r['second']) for r in rows));by={(int(r['second']),int(r['family'])):r for r in rows}
  times=[t for t in times if all((t,f) in by for f in range(4))]
  ax.stackplot([t/60 for t in times],*[ [int(by[t,f]['physical_cells']) for t in times] for f in range(4)],colors=colors,labels=names)
  ax.plot([t/60 for t in times],[sum(int(by[t,f]['tiny']) for f in range(4)) for t in times],color='#152a35',lw=1.5,ls='--',label='≤2-cell adults')
  ax.set_title(f'Seed {seed} · '+('normal mutation' if col==0 else 'mutation disabled'));ax.set_xlim(0,120);ax.set_ylim(0,250);ax.grid(alpha=.15)
  if col==0:ax.set_ylabel('Physical cells / tiny adults')
  if row==3:ax.set_xlabel('Simulated minutes')
fig.legend(*axes[0,0].get_legend_handles_labels(),loc='lower center',ncol=5)
fig.suptitle('Two-hour default worlds · actual starting cast · no interventions',fontsize=16);fig.tight_layout(rect=(0,.035,1,.97));fig.savefig(root/'lineage-trajectories.png',dpi=140)
fig,axes=plt.subplots(2,4,figsize=(14,7))
for row,seed in enumerate([6100,6101]):
 p=root/f'long-game-{seed}'/'poses.csv'
 if not p.exists():continue
 data=list(csv.DictReader(p.open()))
 for col,t in enumerate([0,600,1800,3600]):
  ax=axes[row,col];pose=[r for r in data if int(r['second'])==t];nodes={(r['id'],r['node']):r for r in pose}
  for r in pose:
   x,y=float(r['x']),float(r['y']);p=nodes.get((r['id'],r['parent']));f=int(r['family'])
   if p:ax.plot([x,float(p['x'])],[y,float(p['y'])],color=colors[f],lw=.7)
   ax.add_patch(plt.Circle((x,y),.34,color=colors[f]))
  ax.set_xlim(-24,24);ax.set_ylim(-24,24);ax.set_aspect('equal');ax.set_title(f'Seed {seed} · {t//60} min');ax.set_xticks([]);ax.set_yticks([]);ax.set_facecolor('#09202a')
fig.suptitle('Actual physical poses · ancestry colors · food omitted',fontsize=16);fig.tight_layout();fig.savefig(root/'physical-poses.png',dpi=150)
