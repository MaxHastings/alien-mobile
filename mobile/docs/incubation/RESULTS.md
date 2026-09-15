# Incubation results

The new pipeline was run against seven authored archetypes. Phase 1 used
three independent 1,800-second colonies per archetype, with anatomy, roles,
and development frozen. Phase 2 used the late phase-1 candidate for each
surviving archetype in three independent 7,200-second colonies with
conservative heritable structural mutation. Holdouts used four unseen seed /
resource-layout combinations for 1,800 simulated seconds.

The archive contains every run, including the aborted two-candidate phase-2
pass and empty candidate sets. The summary below is aggregate evidence, not a
fitness score:

| phase-2 candidate | training late adults | training late births | training max generation | holdout late adults | holdout late births |
|---|---:|---:|---:|---:|---:|
| Dart-0 | 1,765 | 1,521 | 140 | 988 | 853 |
| Ribbon-0 | 379 | 330 | 74 | 262 | 193 |
| Contractile-0 | 504 | 442 | 119 | 532 | 469 |
| Vault-0 | 272 | 196 | 49 | 149 | 62 |
| Primitive-0 | 927 | 650 | 48 | 1,340 | 1,124 |
| Crown-0 | 316 | 68 | 48 | 272 | 87 |

Lancer produced no late reproducing phase-1 candidate. Crown's defining
mechanism was rarely retained in phase 2, so it was not promoted. Primitive
was viable but overlapped the compact-feeder role; it was not used for a
catalog slot. The four promoted late genomes are Dart, Ribbon, Contractile,
and Vault, compiled under the existing player-facing names Skiff, Thread,
Whorl, and Husk respectively. Their exact DNA hashes are in
`docs/discovery/finalists/checksums.json`.

This is evidence of multi-generation continuity and holdout viability, not a
claim that the four are globally optimal. Births and adult counts were
reported alongside late persistence and mechanism retention; no runtime buff,
species flag, equal-share target, or artificial lifespan objective was added.
The previous catalog DNA is retained under `docs/incubation/previous-finalists/`
for direct comparison.
