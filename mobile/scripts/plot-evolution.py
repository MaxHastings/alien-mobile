"""Render measured physical states, never reconstructed DNA or selected fitness."""
import csv,sys,colorsys
from pathlib import Path
from collections import defaultdict
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
stem=Path(sys.argv[1]);rows=list(csv.DictReader(Path(str(stem)+'-bodies.csv').open()))
times=sorted({float(r['seconds']) for r in rows})
fig,axes=plt.subplots(1,5,figsize=(20,5),facecolor='#070c15')
for ax,target in zip(axes,(0,300,600,1200,1800)):
 t=min(times,key=lambda x:abs(x-target));frame=[r for r in rows if float(r['seconds'])==t]
 points={(r['id'],r['cell']):r for r in frame}
 ax.set_facecolor('#070c15')
 for r in frame:
  p=points.get((r['id'],r['parent']));x,y=float(r['x']),float(r['y']);color=colorsys.hsv_to_rgb(float(r['hue']),.6,1)
  if p and abs(float(p['x'])-x)<12 and abs(float(p['y'])-y)<12:ax.plot([x,float(p['x'])],[y,float(p['y'])],color=color,alpha=.55,lw=.7)
  ax.scatter([x],[y],s=6,c=[color],alpha=.85 if r.get('mature','1')=='1' else .4)
 ax.set_aspect('equal');ax.set_xlim(-13,13);ax.set_ylim(-13,13);ax.set_title(f'{t/60:.1f} simulated min',color='white');ax.axis('off')
fig.suptitle('Actual physical snapshots · '+stem.name+' · no genome inspection',color='white');fig.tight_layout()
fig.savefig(str(stem)+'-worlds.png',dpi=140,facecolor=fig.get_facecolor());plt.close(fig)
last=[r for r in rows if float(r['seconds'])==times[-1] and r.get('mature','1')=='1'];groups=defaultdict(list)
for r in last:groups[r['id']].append(r)
selected=[];sizes=set()
for id,group in groups.items():
 if len(group)>=4 and int(group[0]['generation'])>=2 and len(group) not in sizes:
  selected.append((id,group));sizes.add(len(group))
  if len(selected)==4:break
if selected:
 fig,axes=plt.subplots(1,len(selected),figsize=(5*len(selected),5),squeeze=False,facecolor='#070c15')
 for ax,(id,group) in zip(axes.flat,selected):
  x0,y0=float(group[0]['x']),float(group[0]['y']);points={r['cell']:(((float(r['x'])-x0+36)%72)-36,((float(r['y'])-y0+36)%72)-36) for r in group};color=colorsys.hsv_to_rgb(float(group[0]['hue']),.6,1)
  ax.set_facecolor('#070c15')
  for r in group:
   x,y=points[r['cell']];p=points.get(r['parent'])
   if p:ax.plot([x,p[0]],[y,p[1]],color=color,lw=2,alpha=.65)
   ax.add_patch(plt.Circle((x,y),.34,color=color,alpha=.85))
   ax.arrow(x,y,float(r['vx']),float(r['vy']),width=.015,color='white',alpha=.5)
  ax.set_aspect('equal');ax.autoscale_view();ax.margins(.25);ax.axis('off');ax.set_title(f'ID {id} · generation {group[0]["generation"]}\n{len(group)} living cells',color='white')
 fig.suptitle('Blind physical specimens · arrows are instantaneous velocity',color='white');fig.tight_layout()
 fig.savefig(str(stem)+'-blind-specimens.png',dpi=150,facecolor=fig.get_facecolor())
 print('Selected solely by physical count:',[(i,len(g)) for i,g in selected])
