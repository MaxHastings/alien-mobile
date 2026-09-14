"""Read the explicit research DNA format; expose inherited morphology for audit."""
from pathlib import Path
fields='role waveform motorMode period phase amplitude bendingAngle contraction motorStrength axisAngle signalWeight motorChannel sensorRange sensitivity extractionRate digestionRate storageCapacity defenseStrength memoryMode memoryTime neural selfWeight'.split()
rates='neural geometry property role insert erase duplicateGene deleteGene copySection moveSection constructor meta neuralMagnitude geometryMagnitude propertyMagnitude'.split()
def read(path):
    it=iter(Path(path).read_text().split());n=lambda:float(next(it));entry=int(n());count=int(n());r={k:n() for k in rates};genes=[]
    for _ in range(count):
        g=dict(orientation=n(),phase=n());nodes=[]
        for _ in range(int(n())):
            node={k:n() for k in ['parent','x','y','constructor','stiffness','target','branches','repetitions','angle','branchAngle','repetitionAngle','intervalScale','separate']}
            node.update({k:n() for k in fields});node['weights']=[n() for _ in range(64)];node['biases']=[n() for _ in range(8)];nodes.append(node)
        g['nodes']=nodes;genes.append(g)
    return dict(entry=entry,rates=r,genes=genes)
def signature(g):return tuple(tuple((n['parent'],n['role'],n['motorMode'] if n['role']==3 else -1,n['target'],n['branches'],n['repetitions']) for n in gene['nodes']) for gene in g['genes'])
if __name__=='__main__':
    import csv,json
    root=Path(__file__).resolve().parents[1]/'docs/discovery';short=root/'shortlist';short.mkdir(exist_ok=True);records=[]
    for base in sorted((root/'baseline').glob('*.dna')):
        (short/(base.stem+'-ancestor.list')).write_text(str(base)+'\n')
        candidates=[]
        for folder in sorted(root.glob('evolve-'+base.stem+'-*')):
            if not (folder/'archive.csv').exists():continue
            for row in csv.reader((folder/'archive.csv').open()):
                id,f,gen,children,time,cells=map(int,row);p=folder/f'g{id}.dna'
                if p.exists():candidates.append((gen,time,cells,p,read(p)))
        # Representative late-generation survivor and a structurally distinct survivor.
        candidates.sort(key=lambda x:(x[0],x[1]),reverse=True);used=set();chosen=[]
        for item in candidates:
            sig=signature(item[-1])
            if sig in used:continue
            used.add(sig);chosen.append(item)
            if len(chosen)==3:break
        for rank,(gen,time,cells,p,g) in enumerate(chosen):
            name=f'{base.stem}-d{rank}';(short/(name+'.list')).write_text(str(p)+'\n');records.append(dict(name=name,path=str(p),generation=gen,seconds=time,cells=cells,signature=signature(g)))
    (root/'shortlist.json').write_text(json.dumps(records,indent=2))
    for r in records:print(r['name'],r['generation'],r['cells'],r['path'])
