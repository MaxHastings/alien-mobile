# Persistent evolutionary tension

Design direction agreed in the September 13, 2026 discussion. This is a plan,
not an implementation or a claim that five-hour depth has been achieved.

The direction is **curated ignition, followed by autonomous ecological and
evolutionary tension**. Organisms should remain recognizable across generations,
while inherited change and their effects on local ecology create new selection
pressures. Quiet periods and extinction are legitimate outcomes.

## Boundaries

- Initialization may choose viable ordinary genomes, slight inherited variation,
  two or three related founder branches, useful placement, spatial opportunities,
  and a readable camera. A crude predator founder is optional, not a requirement.
- After initialization, survival and change come from physical resources,
  geography, paid action and construction, local interactions, recycling,
  reproduction, and inherited mutation. Founder identities receive no protection.
- Preserve primitive initialization for Origin/research experiments. The current
  primitive default remains unchanged until a replacement is evaluated.
- No biological director, population rescue, novelty reward, diversity quota,
  scheduled species appearance, or mutation increase triggered by stagnation.
  Presentation may highlight observed events without causing them.
- Do not optimize species per hour or require a minimum number of lineages.
  Metrics are observations and must never feed back into simulation rules.

## What M14 establishes

The [M14 report](M14_REPORT.md) documents finite motes, local consumption,
recycling, spatial flow, modular development, expressed module duplication,
structural deletion, role mutation, inherited controllers, memory/storage
mechanisms, and bounded meta-mutation. These are available mechanisms, not proof
that evolution discovers or retains useful combinations.

Both five-hour runs ended mostly or entirely single-celled. Some expanding DNA
was unexpressed. The present moving resource patch and background supply permit
turnover, but sustained trophic networks and persistent regional divergence have
not been demonstrated. Increasing births or stored genome length would not
resolve this evidence gap.

## Implementation order

1. **Spatial ecology and feedback from exploitation.** Test multiple resource
   regions with limited exchange and local, exhaustible availability. Resource
   supply and renewal obey fixed physical rules and an explicit energy ledger.
   Consumption changes availability; abundance does not directly set a bonus or
   penalty. Movement between regions must incur its existing physical costs.
   Verify that geography actually limits mixing rather than merely looking varied.
2. **Curated ignition.** Evaluate a small related founder set through the same
   development, costs, mutation and death paths as descendants. Measure opening
   readability separately from long-term persistence. Do not require predation
   until an ordinary genome supports it without subsidies.
3. **Coherent innovation.** Use the existing duplication and controller inheritance
   before extending the genome. Add expressiveness only when a demonstrated
   ecological opportunity cannot be reached with the existing mechanisms.
   Duplicates pay full costs and may fail; viable intermediates are a possibility,
   not a guarantee or a mutation acceptance filter.

## First concrete experiment

Add a research-only spatial environment alongside the current baseline in
`EvolutionResearch.cpp`, with configuration in `SimulationConfig.h` and physical
resource behavior in `World.cpp`. Begin with separated, locally depletable patches
and finite renewal. Keep total external energy input comparable to the baseline;
account explicitly for any new environmental reservoir, transfer, and loss.
Do not simultaneously change mutation rates or add cell roles.

Compare a two-by-two design: current versus spatial ecology, and primitive versus
curated founders. Use a predeclared common seed set, include extinctions, and retain
the current 360-cell limit for the initial comparison. Record capacity blocking:
a global allocation limit may interfere with regional independence and must not
be mistaken for ecological competition.

Inspect the first five minutes, 30-minute trajectories, and five-hour runs.
Record resource availability and uptake by region, organism migration, local
ancestry composition, mature phenotype distributions, expressed versus stored
structure, births/deaths, paid motion, and actual energy transfers from predation.
Lineage labels alone are not evidence of different ecological strategies.

Test causal accounting and observer independence. Compare otherwise matched
environments with faster exchange or without local depletion to distinguish the
effect of geography from consumption-driven feedback. Keep failures visible;
do not tune parameters per seed or reintroduce selected descendants into a run.

Evidence for progress would be persistent regional differences and changes in
relative reproductive success connected to measured local resource or prey
changes. More colors, more mutations, or resource cycles driven only by the clock
would not establish frequency-dependent selection. Survival alone is insufficient.
These are evaluation criteria, not runtime targets.

## Intended experience

| Time | Experience to evaluate, not an enforced event schedule |
| --- | --- |
| 0–5 minutes | Readable bodies, real feeding, movement, growth and reproduction |
| 5–30 minutes | Recognizable inherited differences; some branches spread or vanish |
| 30–120 minutes | Local exploitation changes opportunities; regional histories diverge |
| 2–5 hours | Accumulated ecological and inherited change makes the world's history visible |

The aim is dynamic distributions, not maximum diversity or complexity. A simple
organism winning is valid. Repeated convergence on the same simple strategy across
environments is evidence to investigate the available tradeoffs, not permission
to penalize that organism by identity.
