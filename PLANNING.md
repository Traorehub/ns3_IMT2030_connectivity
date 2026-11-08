# Planning des cinq labs

Ce document fixe la progression du dépôt `ns3_IMT2030_connectivity`. Chaque lab possède son dossier sous `labs/` et son propre README.

Les Labs 1 et 2 sont livrés. Les Labs 3 à 5 comportent un README de cadrage.

## Lab 1 (livré)

Dossier `labs/lab01_wifi_dense/`. Le scénario ns-3 utilise le module Wi-Fi 802.11n, avec Yans et HtMcs7. Il joue le rôle de proxy pour une zone dense de type connectivité massive. Le rapport est `docs/latex/rapport_lab01.pdf`.

## Lab 2 (livré)

Dossier `labs/lab02_campagne/`. Le plan factoriel compte 24 runs, sur `gridSide`, `intervalMs` et la politique. Le rapport est `docs/latex/rapport_lab02.pdf`.

## Lab 3 (à venir)

Dossier `labs/lab03_controleur/`. Le contrôleur `adaptive` devra modifier une métrique de livraison, puisque le Lab 2 a montré qu'il reste inactif sur 23 cellules du plan et qu'il ne change pas les octets reçus sur la cellule où il s'active.

## Lab 4 (à venir)

Dossier `labs/lab04_5g_lena/`. Introduction du module 5G-LENA, afin de remplacer progressivement le proxy Wi-Fi.

## Lab 5 (à venir)

Dossier `labs/lab05_ai_native/`. Boucle de contrôle documentée, avec un éventuel apprentissage via ns3-gym, dans la lignée des architectures RIC.

## Ordre recommandé

Le Lab 4 suppose que les Labs 1 et 2 sont assimilés, car la montée en complexité radio s'ajoute à la courbe d'apprentissage de ns-3.
