# Bilan (synthèse)

Le détail du projet est dans le [README](../README.md) à la racine.

## Livrable actuel

- Scénario `imt2030-density-gestion` : grille dense, trafic UDP, comparaison `baseline` / `adaptive`.
- Métriques appendées dans `sorties/resultats.csv`.
- Figures : `sorties/image_topologie.svg`, `sorties/image_comparatif.svg`, captures dans `docs/img/`.

## Résultat de référence (campagnes effectuées)

Grille **10×10**, `--intervalMs=40`, **12 s** : livraison ~**18,6 %** ; **adaptive** = **50** `throttleEvents`, **baseline** = **0**.

## Prochaine itération technique

Améliorer le contrôleur pour que `adaptive` augmente ou stabilise `deliveryRatio` sous saturation, pas seulement déclencher des veilles.

## Lien 6G

Étape simulable d'un axe IMT-2030 (densité + gestion) ; stack 6G et AI-native complets : à brancher sur 5G-LENA / O-RAN.
