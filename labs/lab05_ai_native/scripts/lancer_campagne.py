#!/usr/bin/env python3
"""Campagne Lab 5 : meme plan, politique feedback, capacite de reference 3 Mbit/s."""

from __future__ import annotations

import itertools
import subprocess
import sys
from pathlib import Path

NS3_DIR = Path("/mnt/c/Users/MOH/Documents/Projet_Perso/ns-allinone-3.43/ns-3.43")
OUT_REL = "../projet-visuel-imt2030/labs/lab05_ai_native/sorties"
OUT_ABS = Path(
    "/mnt/c/Users/MOH/Documents/Projet_Perso/ns-allinone-3.43/projet-visuel-imt2030/labs/lab05_ai_native/sorties"
)

GRID_SIDES = (4, 6, 8, 10)
INTERVALS_MS = (40, 80, 160)
SIM_TIME = 10
CAPACITY_MBPS = 3


def main() -> int:
    OUT_ABS.mkdir(parents=True, exist_ok=True)
    csv_path = OUT_ABS / "resultats.csv"
    if csv_path.exists():
        csv_path.unlink()

    combos = list(itertools.product(GRID_SIDES, INTERVALS_MS))
    log_path = OUT_ABS / "journal_campagne.txt"
    with log_path.open("w", encoding="utf-8") as log:
        log.write(f"Nombre de runs : {len(combos)}\n")
        for index, (grid_side, interval_ms) in enumerate(combos, start=1):
            args = (
                f"scratch/projet-visuel-imt2030/imt2030-density-gestion "
                f"--policy=feedback --gridSide={grid_side} --simTime={SIM_TIME} "
                f"--intervalMs={interval_ms} --capacityMbps={CAPACITY_MBPS} "
                f"--enableAnim=0 --outputDir={OUT_REL}"
            )
            header = (
                f"[{index}/{len(combos)}] gridSide={grid_side} "
                f"intervalMs={interval_ms} policy=feedback"
            )
            print(header, flush=True)
            log.write(header + "\n")
            log.flush()
            completed = subprocess.run(
                ["./ns3", "run", args],
                cwd=NS3_DIR,
                text=True,
                capture_output=True,
            )
            log.write(completed.stdout)
            if completed.returncode != 0:
                log.write(completed.stderr)
                print(completed.stderr, file=sys.stderr)
                return completed.returncode
    print(f"CSV : {csv_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
