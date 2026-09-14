"""Plot physical snapshots exported by ALIEN_DEVELOPMENT_DUMP, not genome layouts."""
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import csv
from pathlib import Path
folder=Path(__file__).resolve().parents[1]/'docs/depth'
fig,axes=plt.subplots(2,2,figsize=(12,9),facecolor='#070c15')
for i,(ax,title) in enumerate(zip(axes.flat,['Repeated segment · 5 nodes → 26 cells','Radial branches · 5 nodes → 38 cells','Nested branching · 8 nodes → 29 cells','Repeated appendages · 7 nodes → 50 cells'])):
 rows=list(csv.DictReader((folder/f'phenotype-{i}.csv').open()))
 xy=[(float(r['x']),float(r['y'])) for r in rows]
 color=['#70cfac','#69bde0','#b19ddd','#e5b776'][i]
 ax.set_facecolor('#070c15')
 for r,(x,y) in zip(rows,xy):
  p=int(r['parent'])
  if p>=0: ax.plot([x,xy[p][0]],[y,xy[p][1]],color=color,alpha=.4,lw=1.3)
 ax.scatter(*zip(*xy),s=38,c=color,zorder=3)
 ax.scatter(*xy[0],s=12,c='#ffffff',zorder=4)
 ax.set_aspect('equal');ax.set_title(title,color='white',fontsize=12,pad=12);ax.axis('off');ax.margins(.15)
fig.suptitle('M8 · actual CPU cell positions after incremental construction + 10 seconds physics',color='white',fontsize=14)
fig.text(.5,.015,'Authored developmental references. Test energy supplied during growth; no creature-level movement.',ha='center',color='#8b9aad',fontsize=10)
fig.tight_layout(rect=(0,.035,1,.95));fig.savefig(folder/'m8-physical-phenotypes.png',dpi=150,facecolor=fig.get_facecolor())
