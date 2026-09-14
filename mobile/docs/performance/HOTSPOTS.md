# Crowding performance pass — 2026-09-13

Source inspection and headless validation; no game, simulator, or device session was launched.

## Changes

- CPU collisions use a flat incident-connection index and constant-time per-cell connection marks instead of scanning every connection for each nearby pair. Grid buckets use the actual repulsion distance. Neighbor order and force accumulation order are preserved.
- Spatial queries accept reusable caller storage and enumerate axis bins without temporary vectors. Food, sensors, receivers and attacks reuse that storage within each pass. Results remain sorted by original cell index.
- Creature lookups cache validated vector indices. Growth, compaction, copies, reset, reordering and same-size public-vector replacements remain supported. Missing IDs do not rebuild the map unnecessarily.
- Sensor owner lookup is outside the neighbor loop. Motor and defender passes traverse flat incident edge/joint lists instead of repeatedly scanning global topology.
- Attack selection sorts eligible targets by distance, breaking ties by original index. Occlusion checks stop at the nearest accessible target and reuse relative positions. Interaction rules are unchanged; worst-case fully blocked crowds can still require quadratic occlusion work per attacker.
- Reproductive capacity waits retain the development summary alongside the proposed genome. Failed capacity checks no longer copy and re-validate the same DNA each tick.
- Metal keeps connectivity and slot maps until topology changes. Muscle target updates upload only changed neighbor/angle snapshots. GPU spatial-grid storage is reused through GPU-only writes on the same command queue. CPU-written snapshots remain immutable for in-flight commands.
- GPU repulsion rejects out-of-range cells before searching their connectivity.
- Rendering computes body colors once per creature per frame.
- Frame catch-up work has a wall-clock budget of half the target frame interval, with at most eight ticks per draw. At least one pending tick can run; overload discards whole-tick backlog while retaining the fractional remainder. Fixed physics dt remains unchanged. Simulation time may run slower than real time during overload. One expensive tick cannot be preempted.

## Measurements

Single Release runs on the same host, using `AlienMobileHotspotBenchmark`; times cover the entire fixture, not one frame. These are CPU timings, not phone FPS claims.

| Fixture | Before | After | Approximate speedup |
| --- | ---: | ---: | ---: |
| Seed 42 default world, 600 ticks | 33.05 ms | 16.68 ms | 2.0× |
| Seed 42 initially compressed world, 600 ticks | 31.05 ms | 13.19 ms | 2.4× |
| Dense 240-cell connected physics, 60 ticks | 112.03 ms | 26.43 ms | 4.2× |
| Dense 1,200-cell connected physics, 60 ticks | 3,760.10 ms | 333.95 ms | 11.3× |

All four printed final checksums matched the pre-change executable exactly. The default-world fixtures ended with 41 and 39 cells respectively; the 1,200-cell case is a stress fixture above the current player-world cap.

## Verification

- All 26 Release CTest tests passed, including CPU/Metal parity, dense GPU grid, biological behavior, damage, articulated locomotion, and mutation continuity.
- Four targeted AddressSanitizer/UndefinedBehaviorSanitizer tests passed: hot paths, mutation continuity, damage, articulation.
- iPhone Release build completed with signing disabled. Nothing was installed.
- New permanent hot-path tests cover index invalidation, frame-budget overload, exhaustive dense collision equivalence, and 200 randomized bounded/wrapped attack-selection comparisons with an index-order reference, including distance ties.
- Spatial tests retain exhaustive neighbor-coverage and duplicate checks across bounded/wrapped worlds and large query radii.

To reproduce CPU timings: build the Release `AlienMobileHotspotBenchmark` target and run it. To run regression checks: `ctest --test-dir mobile/build-release --output-on-failure`.

Dense physical interactions still grow with the number of actual nearby pairs. No interaction caps, dropped neighbors, weaker collisions, or changed reproduction costs were introduced. Live device profiling would be needed to attribute remaining frame spikes between GPU collision work, biology, and drawing.
