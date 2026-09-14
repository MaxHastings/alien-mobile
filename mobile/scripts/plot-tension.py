#!/usr/bin/env python3
"""Plot measured trajectories and physical cells; never select simulation outcomes."""
import csv, pathlib, sys, collections, colorsys
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
stem=pathlib.Path(sys.argv[1])
def read(suffix):
 with pathlib.Path(str(stem)+suffix).open() as f:return list(csv.DictReader(f))
pop=read('-population.csv');ph=read('-phenotypes.csv');regions=read('-regions.csv');bodies=read('-bodies.csv')
by=collections.defaultdict(dict)
for x in ph:by[float(x['seconds'])][x['id']]=x
times=sorted(by);x=[t/3600 for t in times]
plt.style.use('dark_background');fig,axes=plt.subplots(3,1,figsize=(11,9),sharex=True,constrained_layout=True)
for label,predicate in [('1 cell',lambda n:n==1),('2–3 cells',lambda n:2<=n<=3),('4–6 cells',lambda n:4<=n<=6),('7+ cells',lambda n:n>=7)]:
 axes[0].plot(x,[sum(predicate(int(v['cells'])) for v in by[t].values()) for t in times],label=label,lw=1.5)
axes[0].set_ylabel('Mature organisms');axes[0].legend(ncol=4);axes[0].set_title(stem.name+' — actual sampled history')
for b in range(4):
 data={float(v['seconds']):v for v in regions if v['region']==str(b)};ts=sorted(data)
 axes[1].plot([t/3600 for t in ts],[float(data[t]['resource_energy']) for t in ts],label=f'Bed {b+1}')
axes[1].set_ylabel('Stored resource energy');axes[1].legend(ncol=4)
axes[2].plot(x,[sum(float(v['paid_thrust'])>1e-6 for v in by[t].values()) for t in times],color='#5ed5db')
axes[2].set_ylabel('Bodies exerting paid thrust');axes[2].set_xlabel('Simulated hours')
for ax in axes:ax.grid(alpha=.15)
fig.savefig(str(stem)+'-history.png',dpi=150);plt.close(fig)
frames=collections.defaultdict(list)
for b in bodies:frames[float(b['seconds'])].append(b)
end=max(frames);targets=[300,1800,end];chosen=[min(frames,key=lambda t:abs(t-target)) for target in targets]
fig,axes=plt.subplots(1,3,figsize=(15,5),constrained_layout=True)
for ax,t in zip(axes,chosen):
 grouped=collections.defaultdict(dict)
 for b in frames[t]:grouped[b['id']][b['cell']]=b
 for organism in grouped.values():
  for b in organism.values():
   px,py=float(b['x']),float(b['y']);color=colorsys.hsv_to_rgb(float(b['hue']),.6,.9)
   parent=organism.get(b['parent'])
   if parent:
    qx,qy=float(parent['x']),float(parent['y']);dx=qx-px;dy=qy-py;dx-=32*round(dx/32);dy-=32*round(dy/32)
    ax.plot([px,px+dx],[py,py+dy],color=color,lw=.65,alpha=.65)
   ax.add_patch(plt.Circle((px,py),.34,facecolor=color,alpha=.8,edgecolor='none'))
 ax.set(xlim=(-16,16),ylim=(-16,16),aspect='equal',title=f'{t/60:.1f} simulated minutes');ax.set_xticks([]);ax.set_yticks([])
fig.suptitle('Recorded physical cells — shared spatial scale; no reconstructed or idealized bodies')
fig.savefig(str(stem)+'-physical.png',dpi=150);plt.close(fig)
