# Built-in creature incubation

This is the offline breeding pipeline for turning rough built-in archetypes
into ordinary playable starting organisms. It intentionally supersedes the
old 600-second/first-40 discovery search; old archives remain under
`docs/discovery/` for comparison.

## Stages

1. **Phase 1 — competence.** Each archetype runs in three independent
   colonies for 1,800 simulated seconds. Controller and organ-property edits
   are heritable; anatomy, roles, and development are frozen. No candidate is
   archived before the final quarter of the colony, and an archived adult must
   have produced at least three mature children.
2. **Phase 2 — lineage breeding.** Late phase-1 descendants run in separate
   colonies for 7,200 simulated seconds with conservative geometry, role,
   construction, and module mutation rates. Evidence is recorded per lineage,
   not as a single best individual.
3. **Holdout.** Finalists are replayed with ordinary mutation in resource
   layouts and seeds not used during incubation. The holdout is a diagnostic;
   it never changes runtime behavior or grants a catalog advantage.

The observer records late adult continuity, mature births, maximum generation,
changed descendants, and retention of the founder's defining organ mechanism.
Birth count alone is not a selection criterion. The pipeline preserves failed
colonies and empty candidate slots as evidence. In particular, repeated loss
of a defining mechanism is evidence against the archetype, not a reason to
protect it.

Run it with:

```sh
cmake -S mobile -B mobile/build-incubation -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_SYSROOT="$(xcrun --show-sdk-path)"
cmake --build mobile/build-incubation --target AlienMobileCatalogDiscovery -j4
python3 mobile/scripts/run-incubation.py all --workers 4
```

Set `ALIEN_DISCOVERY_EXE` when using another host build. The generated
`lineage-evidence.csv`, per-colony `lineage.csv`, `integrity.txt`, DNA archives,
and command files are the audit trail. A catalog update must copy only an
exact archived genome into the generated catalog source and record its hash;
the game still loads it through the ordinary founder path.
