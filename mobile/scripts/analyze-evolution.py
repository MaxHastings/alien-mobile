#!/usr/bin/env python3
"""Read-only M14 observer summary; no output feeds back into the simulation."""
import csv, json, pathlib, statistics, sys
from collections import Counter, defaultdict

def read(path):
    with path.open() as f:
        return list(csv.DictReader(f))

def analyze(stem):
    pop=read(pathlib.Path(str(stem)+'-population.csv'))
    phen=read(pathlib.Path(str(stem)+'-phenotypes.csv'))
    histories=read(pathlib.Path(str(stem)+'-lineages.csv'))
    final=pop[-1]; seconds=float(final['seconds']); mins=seconds/60
    log=pathlib.Path(str(stem)+'.log').read_text().strip().splitlines()[-1]
    summary=dict(item.split('=',1) for item in log.split() if '=' in item)
    wall=float(summary['wall_seconds'])
    bytime=defaultdict(list)
    for p in phen: bytime[float(p['seconds'])].append(p)
    last=bytime.get(seconds,[])
    # Sampling can report the last timestamp twice; preserve unique individuals.
    last=list({p['id']:p for p in last}.values())
    counters={k:dict(sorted(Counter(int(p[k]) for p in last).items()))
              for k in ('cells','genome_nodes','modules','motors','sensors','attackers','branches')}
    mature_gen=max((int(p['generation']) for p in phen),default=0)
    observed=set(); first_times=[]; dominant=[]; previous_ids=set(); replacements=[]
    for t,rows in sorted(bytime.items()):
        rows=list({p['id']:p for p in rows}.values())
        signatures={(int(p['cells']),int(p['modules']),int(p['branches']),
                     int(p['motors']),int(p['sensors']),int(p['attackers'])) for p in rows}
        if t>=30:
            first_times.extend([t]*len(signatures-observed))
        observed.update(signatures)
        counts=Counter(p['lineage'] for p in rows)
        if counts: dominant.append((t,counts.most_common(1)[0][0]))
        ids={p['id'] for p in rows}
        if previous_ids: replacements.append(len(previous_ids-ids)/len(previous_ids))
        previous_ids=ids
    switches=[b[0] for a,b in zip(dominant,dominant[1:]) if a[1]!=b[1]]
    rate_fields=[k for k in phen[0] if k.endswith('_rate') or k.endswith('_magnitude')] if phen else []
    rates={k:[min(float(p[k]) for p in phen),max(float(p[k]) for p in phen)] for k in rate_fields}
    r={'seed':int(summary['seed']),'simulated_seconds':seconds,'wall_seconds':wall,
       'alive_at_end':int(final['cells'])>0,'births':int(final['births']),
       'births_per_1x_minute':int(final['births'])/mins,
       'births_per_headless_wall_minute':int(final['births'])*60/wall,
       'headless_times_real_time':seconds/wall,'maximum_attempted_generation':int(final['generation']),
       'maximum_completed_generation':int(final.get('mature_generation') or mature_gen),
       'generation_measurement':'completed counter' if final.get('mature_generation') else 'sampled mature lower bound',
       'mature_population':int(final['mature']),'final_cells':int(final['cells']),
       'peak_sampled_cells':max(int(p['cells']) for p in pop),
       'observed_lineages':len(histories),'persistent_120s_lineages':sum(float(h['last_mature_observation'])-float(h['first_mature_observation'])>=120 for h in histories),
       'observed_lineages_absent_at_end':sum(float(h['last_mature_observation'])<seconds for h in histories),
       'births_per_mean_mature_population_per_1x_minute':int(final['births'])/mins/max(1,statistics.mean(int(p['mature']) for p in pop)),
       'final_distributions':counters,'mutation_regime_ranges':rates,
       'observed_morphotype_count':len(observed),'first_morphotype_times':first_times,
       'dominant_lineage_switches':len(switches),'dominant_lineage_switch_times':switches,
       'mean_sample_interval_population_replacement_fraction':statistics.mean(replacements) if replacements else 0,
       'attacked_energy':float(final['attacked']),'digested_energy':float(final['digested']),
       'maximum_energy_error':float(summary['maximum_energy_error']),
       'completed_mutation_births':int(final['mutated_births']) if final.get('mutated_births') else None,
       'completed_structural_births':int(final['structural_births']) if final.get('structural_births') else None,
       'completed_meta_births':int(final['meta_births']) if final.get('meta_births') else None}
    for label,key in [('candidate_mutation','completed_mutation_births'),('structural_operator','completed_structural_births')]:
        count=r[key]
        r[label+'_births_per_1x_minute']=count/mins if count is not None else None
        r['mean_seconds_per_'+label+'_birth']=seconds/count if count else None
    return r

directory=pathlib.Path(sys.argv[1])
results=[analyze(pathlib.Path(str(p).removesuffix('-population.csv'))) for p in sorted(directory.glob('*-population.csv'))]
(directory/'analysis.json').write_text(json.dumps(results,indent=2)+'\n')
lines=['# Measured evolution runs','',
       'Births/min below means simulated minutes at an ideal sustained 1×. Headless throughput is separately measured wall time. Generation is completed, or a sampled mature lower bound for older runs. Morphotype signatures are physical cell count, module count, branch count and motor/sensor/attacker counts; damage and sampling can affect them. They are **not** a count of meaningful innovations or a fitness signal.','',
       '| Seed | End (s) | Alive | Births/min 1× | Mature gen | Mature population | Cells | Seen lineages | Persistent ≥120s | Headless × |',
       '|---|---:|:---:|---:|---:|---:|---:|---:|---:|---:|']
for r in results:
    lines.append(f"| {r['seed']} | {r['simulated_seconds']:.0f} | {'yes' if r['alive_at_end'] else 'no'} | {r['births_per_1x_minute']:.1f} | {r['maximum_completed_generation']} | {r['mature_population']} | {r['final_cells']} | {r['observed_lineages']} | {r['persistent_120s_lineages']} | {r['headless_times_real_time']:.1f} |")
lines+=['',f"Survival: {sum(r['alive_at_end'] for r in results)}/{len(results)}. Full distributions, inherited mutation ranges, event counters and observed lineage replacement times are in analysis.json.",'']
(directory/'ANALYSIS.md').write_text('\n'.join(lines))
print('\n'.join(lines))
