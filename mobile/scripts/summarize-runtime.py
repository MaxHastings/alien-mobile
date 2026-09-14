#!/usr/bin/env python3
"""Summarize reported native timing samples; cumulative CPU averages stay labeled."""
import json,pathlib,re,statistics,sys
path=pathlib.Path(sys.argv[1]);text=path.read_text();metrics={}
for key in ['fps','thermal','tps','biologyMs','syncMs','metalMs','renderCpuMs',
            'renderGpuMs','memoryMB','livingCells','simulatedSeconds','births']:
    values=[float(v) for v in re.findall(r'\b'+key+r'=([0-9.]+)',text)]
    if values:
        ordered=sorted(values)
        metrics[key]={'samples':len(values),'min':min(values),'median':statistics.median(values),
                      'p05':ordered[int((len(ordered)-1)*.05)],'max':max(values),'last':values[-1]}
metrics['note']='CPU stage values are reported cumulative averages, not per-frame percentiles. Simulator thermal state is not a physical-device thermal measurement.'
path.with_suffix('.metrics.json').write_text(json.dumps(metrics,indent=2)+'\n')
print(json.dumps(metrics,indent=2))
