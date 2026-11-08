# Lab 2 — Campagne paramétrique

Ce laboratoire exécute un plan factoriel sur le scénario Wi-Fi du Lab 1. Les facteurs sont `gridSide` (4, 6, 8, 10), `intervalMs` (40, 80, 160) et `policy` (baseline, adaptive). La durée est fixée à 10 s. Chaque terminal envoie au plus 40 paquets de 512 octets. L'animation NetAnim est désactivée.

## Scripts

Le script `scripts/lancer_campagne.py` enchaîne les 24 runs et écrit `sorties/resultats.csv`. Le script `scripts/analyser_campagne.py` produit les SVG par intervalle, ainsi que `livraison_campagne.png` et `throttle_40ms.png`.

## Lecture des résultats

La livraison diminue lorsque le nombre de terminaux augmente et lorsque les paquets sont rapprochés dans le temps. Sur 16 terminaux, la livraison vaut 1 pour les trois intervalles. Sur 64 terminaux, elle passe de 0,437 à 40 ms jusqu'à 0,625 à 160 ms. Sur 100 terminaux, elle vaut 0,186 à 40 ms, 0,289 à 80 ms et 0,354 à 160 ms.

Le contrôleur adaptive n'enregistre des événements que pour 100 terminaux et un intervalle de 40 ms. Le compteur vaut alors 50, et les octets reçus restent identiques à ceux de la baseline. Le fichier `sorties/topologie.csv` ne décrit que le dernier run du script. Il ne résume pas le plan. Le tableau complet est `sorties/resultats.csv`.

Le rapport détaillé est [docs/latex/rapport_lab02.pdf](../../docs/latex/rapport_lab02.pdf).
