#!/usr/bin/env python3
"""Campagne Lab 4 executee : 16 terminaux, 40 ms, trois politiques.

Les grilles de 36 terminaux et plus s'arretent dans le RRC ideal de
5G-LENA v3.3.1 sur cet ns-3.43. Elles ne sont pas lancees ici.
"""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path

NS3_DIR = Path("/mnt/c/Users/MOH/Documents/Projet_Perso/ns-allinone-3.43/ns-3.43")
OUT_REL = "../projet-visuel-imt2030/labs/lab04_5g_lena/sorties"
OUT_ABS = Path(
    "/mnt/c/Users/MOH/Documents/Projet_Perso/ns-allinone-3.43/projet-visuel-imt2030/labs/lab04_5g_lena/sorties"
)

POLICIES = ("baseline", "adaptive", "feedback")
SIM_TIME = 10
CAPACITY_MBPS = 3
GRID_SIDE = 4
INTERVAL_MS = 40


def main() -> int:
    OUT_ABS.mkdir(parents=True, exist_ok=True)
    csv_path = OUT_ABS / "resultats.csv"
    if csv_path.exists():
        csv_path.unlink()

    log_path = OUT_ABS / "journal_campagne.txt"
    with log_path.open("w", encoding="utf-8") as log:
        log.write("Runs : 16 terminaux, 40 ms, trois politiques\n")
        for index, policy in enumerate(POLICIES, start=1):
            args = (
                f"scratch/projet-visuel-imt2030/imt2030-nr-gestion "
                f"--policy={policy} --gridSide={GRID_SIDE} --simTime={SIM_TIME} "
                f"--intervalMs={INTERVAL_MS} --capacityMbps={CAPACITY_MBPS} "
                f"--outputDir={OUT_REL}"
            )
            header = f"[{index}/3] policy={policy}"
            print(header, flush=True)
            log.write(header + "\n")
            log.flush()
            completed = subprocess.run(
                ["./ns3", "run", "--no-build", args],
                cwd=NS3_DIR,
                text=True,
                capture_output=True,
            )
            log.write(completed.stdout)
            if completed.stderr:
                log.write(completed.stderr)
            log.flush()
            if completed.returncode != 0:
                print(completed.stderr, file=sys.stderr)
                return completed.returncode
    print(f"CSV : {csv_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
