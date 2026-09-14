from pathlib import Path
import importlib.util,math,csv,sys
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
spec=importlib.util.spec_from_file_location('dna',Path(__file__).with_name('discovery-genomes.py'));dna=importlib.util.module_from_spec(spec);spec.loader.exec_module(dna)
root=Path(__file__).resolve().parents[1]/'docs/discovery'
colors=['#9cb9ad','#fff3be','#ffc15a','#60dcf1','#f5d65d','#e889f2','#f2617a','#bc7bdf','#b091ff','#89c8ff','#efffff','#cacaca','#faa078','#8be5b5']
def develop(g):
    points=[]
    def visit(gi,anchor,angle,depth=0):
        if depth>8 or len(points)>100:return
        gene=g['genes'][gi];local=[]
        for n in gene['nodes']:
            p=anchor if n['parent']<0 else local[int(n['parent'])]
            a=angle+gene['orientation'];x=n['x']*math.cos(a)-n['y']*math.sin(a);y=n['x']*math.sin(a)+n['y']*math.cos(a)
            if p>=0:x+=points[p][0];y+=points[p][1]
            idx=len(points);points.append((x,y,p,int(n['role'])));local.append(idx)
            if n['target']>=0:
                for b in range(int(n['branches'])):
                    an=idx
                    for r in range(int(n['repetitions'])):
                        visit(int(n['target']),an,a+n['angle']+b*n['branchAngle']+r*n['repetitionAngle'],depth+1);an=len(points)-1
    visit(g['entry'],-1,0);return points
files=sorted((root/'shortlist').glob('*.list'))
fig,axs=plt.subplots(math.ceil(len(files)/4),4,figsize=(16,4*math.ceil(len(files)/4)),facecolor='#071b20')
for ax in axs.flat:ax.set_facecolor('#071b20');ax.axis('off')
for ax,path in zip(axs.flat,files):
    points=develop(dna.read(path.read_text().strip()))
    for x,y,p,r in points:
        if p>=0:ax.plot([x,points[p][0]],[y,points[p][1]],color='#57808a',lw=3)
        ax.add_patch(plt.Circle((x,y),.34,color=colors[r]))
    ax.set_aspect('equal');ax.autoscale_view();ax.margins(.2);ax.set_title(path.stem,color='white',fontsize=12)
fig.suptitle('Inherited morphology • screening candidates (not simulated poses)',color='white',fontsize=19)
fig.savefig(root/'candidate-morphology.png',dpi=140,bbox_inches='tight')
