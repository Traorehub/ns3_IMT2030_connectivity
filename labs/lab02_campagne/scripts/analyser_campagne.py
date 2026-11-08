#!/usr/bin/env python3
"""Figures SVG de la campagne Lab 2 a partir de sorties/resultats.csv."""

from __future__ import annotations

import csv
from collections import defaultdict
from pathlib import Path


def load_rows(path: Path) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as handle:
        return list(csv.DictReader(handle))


def svg_grouped_bars(
    series: dict[str, list[tuple[int, float]]],
    out: Path,
    title: str,
    y_label: str,
    y_max: float | None = None,
) -> None:
    width, height = 980.0, 520.0
    pad_l, pad_r, pad_t, pad_b = 70.0, 180.0, 50.0, 70.0
    policies = list(series.keys())
    xs = sorted({x for values in series.values() for x, _ in values})
    if not xs:
        return
    ymax = y_max
    if ymax is None:
        ymax = max((y for values in series.values() for _, y in values), default=1.0)
        ymax = max(ymax * 1.15, 0.05)
    chart_w = width - pad_l - pad_r
    chart_h = height - pad_t - pad_b
    group_w = chart_w / len(xs)
    bar_w = group_w / (len(policies) + 1)
    colors = {"baseline": "#4a6fa5", "adaptive": "#c45c3e"}

    def y_px(value: float) -> float:
        return pad_t + chart_h - (value / ymax) * chart_h

    lines = [
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{width:.0f}" height="{height:.0f}">',
        '<rect width="100%" height="100%" fill="#fafafa"/>',
        f'<text x="{pad_l}" y="28" font-family="sans-serif" font-size="16">{title}</text>',
        f'<text x="18" y="{pad_t + chart_h / 2:.0f}" font-size="12" transform="rotate(-90 18,{pad_t + chart_h / 2:.0f})">{y_label}</text>',
    ]
    for tick in range(5):
        value = ymax * tick / 4
        yy = y_px(value)
        lines.append(
            f'<line x1="{pad_l}" y1="{yy:.1f}" x2="{pad_l + chart_w}" y2="{yy:.1f}" stroke="#e0e0e0"/>'
        )
        lines.append(
            f'<text x="{pad_l - 8}" y="{yy + 4:.1f}" font-size="11" text-anchor="end">{value:.2f}</text>'
        )
    for i, nsta in enumerate(xs):
        for j, policy in enumerate(policies):
            value = dict(series[policy]).get(nsta, 0.0)
            x = pad_l + i * group_w + (j + 0.4) * bar_w
            y = y_px(value)
            h = pad_t + chart_h - y
            lines.append(
                f'<rect x="{x:.1f}" y="{y:.1f}" width="{bar_w * 0.85:.1f}" height="{h:.1f}" fill="{colors.get(policy, "#888")}"/>'
            )
        lines.append(
            f'<text x="{pad_l + i * group_w + group_w / 2:.1f}" y="{height - 28}" font-size="12" text-anchor="middle">{nsta} STA</text>'
        )
    legend_x = width - pad_r + 16
    for j, policy in enumerate(policies):
        yy = 70 + j * 22
        lines.append(
            f'<rect x="{legend_x}" y="{yy}" width="14" height="14" fill="{colors.get(policy, "#888")}"/>'
        )
        lines.append(
            f'<text x="{legend_x + 20}" y="{yy + 12}" font-size="12">{policy}</text>'
        )
    lines.append("</svg>")
    out.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    base = Path(__file__).resolve().parent.parent
    sorties = base / "sorties"
    rows = load_rows(sorties / "resultats.csv")
    delivery_by_interval: dict[int, dict[str, list[tuple[int, float]]]] = defaultdict(
        lambda: defaultdict(list)
    )
    throttle_by_interval: dict[int, dict[str, list[tuple[int, float]]]] = defaultdict(
        lambda: defaultdict(list)
    )
    for result in rows:
        interval = int(float(result["intervalMs"]))
        policy = result["policy"]
        nsta = int(result["nSta"])
        delivery_by_interval[interval][policy].append((nsta, float(result["deliveryRatio"])))
        throttle_by_interval[interval][policy].append((nsta, float(result["throttleEvents"])))

    for interval, series in sorted(delivery_by_interval.items()):
        svg_grouped_bars(
            series,
            sorties / f"livraison_intervalle_{interval}ms.svg",
            f"Taux de livraison selon le nombre de terminaux, intervalle {interval} ms",
            "Taux de livraison",
            y_max=1.0,
        )
    for interval, series in sorted(throttle_by_interval.items()):
        svg_grouped_bars(
            series,
            sorties / f"throttle_intervalle_{interval}ms.svg",
            f"Evenements de gestion selon le nombre de terminaux, intervalle {interval} ms",
            "Evenements throttle",
        )
    try:
        import matplotlib

        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
    except ImportError:
        print("matplotlib absent, SVG seulement")
        print("Figures ecrites dans", sorties)
        return 0

    fig, axes = plt.subplots(1, 3, figsize=(11.2, 3.6), sharey=True)
    for ax, interval in zip(axes, sorted(delivery_by_interval)):
        for policy, color in (("baseline", "#4a6fa5"), ("adaptive", "#c45c3e")):
            points = sorted(delivery_by_interval[interval][policy])
            ax.plot(
                [p[0] for p in points],
                [p[1] for p in points],
                marker="o",
                color=color,
                label=policy,
            )
        ax.set_title(f"Intervalle {interval} ms")
        ax.set_xlabel("Nombre de terminaux")
        ax.set_ylim(0, 1.05)
        ax.grid(True, alpha=0.3)
    axes[0].set_ylabel("Taux de livraison")
    axes[2].legend(loc="upper right")
    fig.suptitle("Campagne Lab 2, duree 10 s, 40 paquets par terminal")
    fig.tight_layout()
    fig.savefig(sorties / "livraison_campagne.png", dpi=140)
    plt.close(fig)

    fig, ax = plt.subplots(figsize=(7.2, 4.2))
    points = sorted(throttle_by_interval[40]["adaptive"])
    ax.bar([str(p[0]) for p in points], [p[1] for p in points], color="#c45c3e")
    ax.set_xlabel("Nombre de terminaux")
    ax.set_ylabel("Evenements throttle")
    ax.set_title("Politique adaptive, intervalle 40 ms")
    ax.grid(True, axis="y", alpha=0.3)
    fig.tight_layout()
    fig.savefig(sorties / "throttle_40ms.png", dpi=140)
    plt.close(fig)
    print("Figures ecrites dans", sorties)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
