from pathlib import Path
import csv,math,sys
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
root=Path(__file__).resolve().parents[1]/'docs/discovery'
names=['Skiff','Thread','Whorl','Husk'] if 'final' in sys.argv else ['Dart-d0','Ribbon-d0','Crown-d1','Crown-d2','Contractile-d0','Vault-d1'];times=[0,4,8,16]
fig,axs=plt.subplots(len(names),len(times),figsize=(14,19),facecolor='#071b20')
colors=['#9cb9ad','#fff3be','#ffc15a','#60dcf1','#f5d65d','#e889f2','#f2617a','#bc7bdf','#b091ff','#89c8ff','#efffff','#cacaca','#faa078','#8be5b5']
for row,name in enumerate(names):
    data=list(csv.DictReader((root/('observe-'+name)/'bodies.csv').open()));data=[r for r in data if r['id']=='0'];first=[r for r in data if r['second']=='0'];origin=(float(first[0]['x']),float(first[0]['y']))
    for col,t in enumerate(times):
        ax=axs[row,col];ax.set_facecolor('#071b20');pose={int(r['node']):r for r in data if int(r['second'])==t}
        for n,r in pose.items():
            x,y=float(r['x'])-origin[0],float(r['y'])-origin[1];p=int(r['parent'])
            if p in pose:ax.plot([x,float(pose[p]['x'])-origin[0]],[y,float(pose[p]['y'])-origin[1]],color='#57808a',lw=2)
            ax.add_patch(plt.Circle((x,y),.34,color=colors[int(r['role'])]))
        ax.plot([float(r['x'])-origin[0] for r in data if r['node']=='0' and int(r['second'])<=t],[float(r['y'])-origin[1] for r in data if r['node']=='0' and int(r['second'])<=t],color='white',alpha=.3,lw=1)
        ax.set_xlim(-8,8);ax.set_ylim(-8,8);ax.set_aspect('equal');ax.axis('off');ax.set_title(f'{name} · {t}s',color='white',fontsize=11)
fig.suptitle('Actual engine poses • identical spatial scale • faint root trajectory',color='white',fontsize=17)
fig.tight_layout();fig.savefig(root/('final-physical-motion.png' if 'final' in sys.argv else 'physical-motion.png'),dpi=130)
