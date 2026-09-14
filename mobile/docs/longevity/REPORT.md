# Long-duration follow-up

**The additional runs were worthwhile. They exposed a long-term diversity problem that the shorter catalog trials did not settle.**

The playable catalog and Simulator build are unchanged. This is a diagnosis, not a new balancing patch. The earlier catalog playtest remains useful, but the results do not support a claim of a persistently diverse, unattended ecosystem.

## What ran

Twelve uninterrupted two-hour worlds: **24 additional simulated hours**.

- Four normal-mutation worlds, seeds 6100–6103.
- Four mutation-disabled controls with the same seeds.
- Two capacity controls: 960 cells instead of 240, seeds 6100–6101.
- Two movement controls: resource-patch drift 6 instead of 1.5, seeds 6100–6101.

The first eight use the exact default playable configuration, initial food wake, opening organisms, positions and energies. Unlike the earlier holdout harness, they do not perturb world parameters or relocate founders. The controls change only the named parameter or mutation rates. No organisms are rescued or introduced after reset. No environmental interventions occur in the default runs.

The research observer records ancestry, physical cells, mature adults, intactness, architecture, organ counts, tiny bodies and births each simulated minute. Actual physical poses are recorded every five minutes. It archives reproducing late descendants with new encoded architectures, bounded to 64 per world. Measurements do not enter selection.

A 7,200-step parity check against the existing `EvolutionResearch` observer produced the identical terminal state hash **5712747812388379802**, including RNG and physical cell state. The older executable needs duration `60.001` rather than `60` to execute exactly 7,200 steps because its duration division truncates a floating-point timestep. The new observer uses an integer step count. This was a harness detail, not a change to the engine.

## Default worlds: the main finding

| Seed | Surviving starting ancestry at two hours | Mature adults | Adults whose developed genomes have ≤2 cells | Physical cells, including developing offspring | Maximum mature generation | Mature births in final 15 minutes |
|---|---|---:|---:|---:|---:|---:|
| 6100 | Skiff | 224 | 223 | 240 | 99 | 99 |
| 6101 | Skiff | 39 | 0 | 192 | 141 | 493 |
| 6102 | Skiff | 35 | 0 | 180 | 121 | 489 |
| 6103 | Whorl | 233 | 233 | 240 | 36 | 52 |

**All four worlds lost three of their four starting ancestries.** Only one ancestry remained by the first recorded samples at 8, 15, 8 and 85 minutes, respectively. Sampling is once per minute, so these are observed times, not exact extinction timestamps.

**Two of the four also converged overwhelmingly to one- and two-cell bodies.** Their final mature organisms had no motors, food/creature sensors or depots. The tiny counts come from inherited developed genomes, and the adults were intact: these are not damaged organisms mistakenly counted as tiny species.

The other two worlds maintained active sensory swimmers and hundreds of mature births in the final quarter-hour. Long runs therefore do not inevitably become inert dots. But these runs also lost the long collector, rotating-arm form and reservoir strategies.

Ancestry is not the same as ecological diversity. Descendants within one ancestry can become different organisms. Here, the physical poses and organ inventories support narrowing toward passive tiny bodies or compact sensory swimmers, rather than merely a change in lineage labels.

![Default lineage trajectories](lineage-trajectories.png)

The filled areas are physical cells by starting ancestry. The dashed line is the **number of tiny adults**, a separate count shown on the same numerical scale, not a percentage or biomass measurement.

![Actual physical poses](physical-poses.png)

These are engine positions, not illustrations. Food is omitted for readability. Colors identify ancestry; they do not assert that descendants still have the ancestor's architecture.

## Mutation is only part of the problem

All four mutation-disabled worlds also ended with only Skiff ancestry. They reached a single surviving ancestry by observed minutes 6, 18, 9 and 11. Their intact four-cell swimmers continued reproducing; final-quarter-hour mature births were 476, 465, 479 and 476.

This separates two effects:

1. **Loss of the starting portfolio can occur without mutation.** The default ecology already favors one of the original strategies over prolonged unattended competition.
2. **Mutation can further simplify the winning population.** In one normal run Whorl ancestry won by losing its arms, sensors and propulsion, not by sustaining the original spinning collector.

Turning mutation down would not solve the first problem. Preserving catalog architectures specially would hide the second.

## The global cell limit affects turnover, but is not the sole driver

Raising the limit to 960 cells did not preserve multiple starting ancestries in either paired world.

In seed 6100, the endpoint still consisted almost entirely of tiny Skiff descendants: **332 of 333 mature adults** had at most two developed cells. The population occupied **371 physical cells**, far below the new limit, and produced **9,513 mature births in the last 15 minutes**.

The 240-cell counterpart had 223 tiny adults out of 224, occupied all 240 cells, and produced 99 mature births in that interval.

Thus the small technical limit can strongly reduce turnover once cheap bodies fill the tank. But simply increasing it does not remove selection for those bodies. The larger population can maintain rapid birth/death turnover while remaining mechanically simple. In the other capacity-control seed, sensory swimmers persisted, as in its default counterpart.

These are paired stochastic trajectories, not a universal numerical estimate of the cap's effect. No phone capacity setting was changed.

## More patch movement changes the winner, but did not restore the portfolio

With patch drift increased from 1.5 to 6:

- Seed 6100 ended with Thread ancestry and 31 mature adults. These were compact descendants; the late records show five-cell bodies, not preservation of the original nine-cell collector. Motors and sensing remained.
- Seed 6101 ended with Skiff ancestry and predominantly tiny descendants.

Both lost the other starting ancestries. This is evidence of context dependence, but not evidence that increasing a single movement parameter solves sustained diversity. It also demonstrates why an ancestry win cannot be treated as survival of its defining mechanism.

## What the engine is selecting for

The code and these experiments suggest two productive hypotheses:

- All default patches supply the same universally interceptable food under the same global radius, drift, cycle and energetic rules. Different locations do not automatically provide different ecological niches. Compact feeders can exploit broadly similar opportunities across the tank.
- Once food can reach a cell passively, losing sensing, propulsion and extra body cells can remove costs. A small stationary constructor can remain viable. The global capacity limit then makes low-turnover saturation possible even when incoming resources could support more biomass.

These are interpretations, not proofs that any specific ecological redesign will work. Existing construction failure, mutation, local energy budgets and resource contact were left intact.

An encoded-architecture count can also overstate meaningful diversity: different inactive modules or motor-mode fields can yield nearly identical tiny physical organisms. Late DNA files were preserved, but were not promoted into the catalog merely for being new genotypes.

## What I would do next

**Stop broad genome search for now.** More optimization against the same ecology risks making these dominant strategies still more efficient.

The next useful engineering experiment is to test whether genuinely different, simultaneous resource opportunities support different physical strategies over long runs—then measure whether the strategies and their useful mechanisms persist, rather than rewarding equal population shares or ancestry colors. A separate capacity/turnover audit should determine when a technical limit suppresses continuing evolution.

That work should keep ordinary inheritance and selection, and should not add catalog-specific survival, protected reproduction, artificial lifespan targets or forced population quotas. The four current specimens are useful probes for it. No such redesign has been applied here.

Your first playtest is still valuable. These unattended results cannot tell us whether intervention, new placements, food and flow make the actual experience engaging. They do tell us that the initial visual variety should not be mistaken for demonstrated long-term ecological variety.

## Integrity, artifacts and reproduction

All twelve runs completed their requested 864,000 simulation steps. Finite-state and connection checks passed. Maximum observed energy-accounting discrepancy across the long runs was **0.000096465 energy units**. No catalog genomes, playable ecology parameters, app code or Simulator installation were changed during this follow-up.

Artifacts:

- [Structured results](evidence.json)
- [Concise run summaries](summary.txt)
- Per-run `command.json`, `completion.json`, `longevity.csv`, `poses.csv`, `pedigree.csv`, `summary.csv`, `integrity.txt`, and archived late DNA.
- [Observer parity result](parity/existing.txt) and [matching hash](parity/new/state-hash.txt).

`last_second_adult_sizes` in the structured results is derived from the pedigree's second-resolution timestamps and can include an organism that died within the final second. Exact endpoint adult and tiny counts come from the final `longevity.csv` sample and are the counts used above.

From the repository root:

```sh
cmake --build mobile/build-release --target AlienMobileCatalogDiscovery -j4
python3 mobile/scripts/run-longevity.py paired
python3 mobile/scripts/run-longevity.py capacity
python3 mobile/scripts/run-longevity.py drift
python3 mobile/scripts/summarize-longevity.py
```

`plot-longevity.py` regenerates the figures with Matplotlib installed.

**Assessment: the catalog is available for human playtesting; sustained unattended ecological diversity remains an objective weakness.**
