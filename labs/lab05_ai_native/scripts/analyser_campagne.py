#!/usr/bin/env python3
"""Figures Lab 5 : feedback juxtapose aux politiques du Lab 3."""

from __future__ import annotations

import csv
from collections import defaultdict
from pathlib import Path


def load_rows(path: Path) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as handle:
        return list(csv.DictReader(handle))


def main() -> int:
    base = Path(__file__).resolve().parent.parent
    sorties = base / "sorties"
    lab3 = base.parent / "lab03_controleur" / "sorties" / "resultats.csv"
    rows = load_rows(lab3) + load_rows(sorties / "resultats.csv")
    delivery: dict[int, dict[str, list[tuple[int, float]]]] = defaultdict(
        lambda: defaultdict(list)
    )
    volume: dict[int, dict[str, list[tuple[int, float]]]] = defaultdict(
        lambda: defaultdict(list)
    )
    for result in rows:
        interval = int(float(result["intervalMs"]))
        policy = result["policy"]
        nsta = int(result["nSta"])
        delivery[interval][policy].append((nsta, float(result["deliveryRatio"])))
        volume[interval][policy].append((nsta, float(result["bytesRx"]) / 1000.0))

    try:
        import matplotlib

        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
    except ImportError:
        print("matplotlib absent")
        return 1

    colors = {"baseline": "#4a6fa5", "adaptive": "#c45c3e", "feedback": "#2f7d4a"}
    order = ("baseline", "adaptive", "feedback")

    fig, axes = plt.subplots(1, 3, figsize=(11.2, 3.6), sharey=True)
    for ax, interval in zip(axes, sorted(delivery)):
        for policy in order:
            points = sorted(delivery[interval][policy])
            ax.plot(
                [p[0] for p in points],
                [p[1] for p in points],
                marker="o",
                color=colors[policy],
                label=policy,
            )
        ax.set_title(f"Intervalle {interval} ms")
        ax.set_xlabel("Nombre de terminaux")
        ax.set_ylim(0, 1.05)
        ax.grid(True, alpha=0.3)
    axes[0].set_ylabel("Taux de livraison")
    axes[2].legend(loc="upper right")
    fig.suptitle("Lab 5, boucle fermee, comparee au Lab 3")
    fig.tight_layout()
    fig.savefig(sorties / "livraison_campagne.png", dpi=140)
    plt.close(fig)

    fig, axes = plt.subplots(1, 3, figsize=(11.2, 3.6), sharey=True)
    for ax, interval in zip(axes, sorted(volume)):
        for policy in order:
            points = sorted(volume[interval][policy])
            ax.plot(
                [p[0] for p in points],
                [p[1] for p in points],
                marker="o",
                color=colors[policy],
                label=policy,
            )
        ax.set_title(f"Intervalle {interval} ms")
        ax.set_xlabel("Nombre de terminaux")
        ax.grid(True, alpha=0.3)
    axes[0].set_ylabel("Octets recus (milliers)")
    axes[2].legend(loc="upper right")
    fig.suptitle("Volume recu, boucle fermee et admission fixe")
    fig.tight_layout()
    fig.savefig(sorties / "octets_campagne.png", dpi=140)
    plt.close(fig)
    print("Figures ecrites dans", sorties)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
