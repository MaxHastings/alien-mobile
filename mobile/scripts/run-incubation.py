#!/usr/bin/env python3
"""Two-phase offline breeding pipeline for the built-in creature archetypes.

This intentionally does not reuse the old 600-second/first-40 search. Phase 1
adapts control and organ properties while preserving anatomy. Phase 2 runs
conservative structural evolution for hours, archives only late descendants,
and evaluates each candidate as a lineage across independent colonies.
"""
from __future__ import annotations
import argparse, csv, json, subprocess, hashlib, os
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DISCOVERY = ROOT / "docs" / "discovery"
OUT = ROOT / "docs" / "incubation"
EXE = Path(os.environ.get("ALIEN_DISCOVERY_EXE", str(ROOT / "build-incubation" / "AlienMobileCatalogDiscovery")))

ARCHETYPES = ["Dart", "Ribbon", "Crown", "Vault", "Lancer", "Primitive", "Contractile"]
PHASE1_SEEDS = [7301, 7307, 7313]
TRAINING = [(0, 7401), (1, 7407), (2, 7413)]
HOLDOUT = [(3, 8401), (4, 8407), (7, 8413), (9, 8419)]

def run_one(args):
    mode, seed, profile, seconds, manifest, folder = args
    folder.mkdir(parents=True, exist_ok=True)
    cmd = [str(EXE), mode, str(seed), str(profile), str(seconds), str(manifest), str(folder), "none"]
    (folder / "command.json").write_text(json.dumps(cmd, indent=2) + "\n")
    p = subprocess.run(cmd, capture_output=True, text=True)
    (folder / "summary.csv").write_text(p.stdout)
    (folder / "stderr.txt").write_text(p.stderr)
    if p.returncode:
        raise RuntimeError(f"{folder}: exit {p.returncode}: {p.stderr}")
    return folder

def parallel(jobs, workers):
    with ThreadPoolExecutor(max_workers=workers) as pool:
        return list(pool.map(run_one, jobs))

def baseline_paths():
    paths = {}
    for name in ARCHETYPES:
        p = DISCOVERY / "baseline" / f"{name}.dna"
        if p.exists(): paths[name] = p
    if len(paths) != len(ARCHETYPES):
        raise SystemExit("run the existing discovery seed export first: run-discovery.py seeds ...")
    return paths

def late_candidates(name, phase1_dirs, limit):
    rows = []
    for folder in phase1_dirs:
        archive = folder / "archive.csv"
        if not archive.exists(): continue
        for row in csv.reader(archive.open()):
            ident, family, generation, children, second, cells = map(int, row)
            dna = folder / f"g{ident}.dna"
            if dna.exists(): rows.append((generation, second, children, cells, dna))
    # All rows are already from the final quarter. Keep structurally distinct,
    # late, reproducing samples; no early individual can enter this set.
    rows.sort(key=lambda x: (x[0], x[1], x[2]), reverse=True)
    chosen, signatures = [], set()
    for row in rows:
        data = row[4].read_bytes()
        sig = hashlib.sha256(data).hexdigest()
        if sig in signatures: continue
        signatures.add(sig); chosen.append(row)
        if len(chosen) >= limit: break
    return chosen

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("stage", choices=["phase1", "phase2", "holdout", "all"])
    ap.add_argument("--phase1-seconds", type=int, default=1800)
    ap.add_argument("--phase2-seconds", type=int, default=7200)
    ap.add_argument("--candidates-per-archetype", type=int, default=4)
    ap.add_argument("--workers", type=int, default=4)
    a = ap.parse_args()
    OUT.mkdir(parents=True, exist_ok=True)
    bases = baseline_paths()
    baseline_manifests = {}
    for name, dna in bases.items():
        manifest = OUT / "manifests" / f"baseline-{name}.list"
        manifest.parent.mkdir(parents=True, exist_ok=True)
        manifest.write_text(str(dna) + "\n")
        baseline_manifests[name] = manifest

    if a.stage in ("phase1", "all"):
        jobs = []
        for name, dna in bases.items():
            for seed in PHASE1_SEEDS:
                profile = PHASE1_SEEDS.index(seed)
                jobs.append(("incubate-phase1", seed, profile, a.phase1_seconds, baseline_manifests[name],
                             OUT / "phase1" / f"{name}-{seed}"))
        parallel(jobs, a.workers)

    phase1 = {name: sorted((OUT / "phase1").glob(f"{name}-*")) for name in bases}
    candidates = {}
    for name in bases:
        selected = late_candidates(name, phase1[name], a.candidates_per_archetype)
        candidates[name] = []
        for rank, (generation, second, children, cells, dna) in enumerate(selected):
            target = OUT / "candidates" / f"{name}-{rank}.dna"
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(dna.read_bytes())
            candidates[name].append(dict(name=name, rank=rank, generation=generation,
                                         second=second, children=children, cells=cells,
                                         dna=str(target)))
    (OUT / "candidates.json").write_text(json.dumps(candidates, indent=2) + "\n")

    if a.stage in ("phase2", "all"):
        jobs = []
        for name, entries in candidates.items():
            for c in entries:
                manifest = OUT / "manifests" / f"{name}-{c['rank']}.list"
                manifest.parent.mkdir(parents=True, exist_ok=True)
                manifest.write_text(c["dna"] + "\n")
                for profile, seed in TRAINING:
                    jobs.append(("incubate-phase2", seed, profile, a.phase2_seconds, manifest,
                                 OUT / "phase2" / f"{name}-{c['rank']}-{profile}-{seed}"))
        parallel(jobs, a.workers)

    if a.stage in ("holdout", "all"):
        jobs = []
        for name, entries in candidates.items():
            for c in entries:
                manifest = OUT / "manifests" / f"{name}-{c['rank']}.list"
                for profile, seed in HOLDOUT:
                    jobs.append(("incubate-phase2", seed, profile, a.phase2_seconds, manifest,
                                 OUT / "holdout" / f"{name}-{c['rank']}-{profile}-{seed}"))
        parallel(jobs, a.workers)

    if a.stage in ("phase2", "holdout", "all"):
        report = []
        for name, entries in candidates.items():
            for c in entries:
                for stage in ("phase2", "holdout"):
                    root = OUT / stage
                    for folder in sorted(root.glob(f"{name}-{c['rank']}-*")):
                        lineage = folder / "lineage.csv"
                        if not lineage.exists(): continue
                        for row in csv.DictReader(lineage.open()):
                            row.update(candidate=f"{name}-{c['rank']}", stage=stage,
                                       source_gen=c["generation"], source_second=c["second"])
                            report.append(row)
        with (OUT / "lineage-evidence.csv").open("w", newline="") as f:
            if report:
                writer = csv.DictWriter(f, fieldnames=list(report[0]))
                writer.writeheader(); writer.writerows(report)

if __name__ == "__main__": main()
