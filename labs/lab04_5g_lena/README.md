# Lab 4 - Cellule NR, grille de 16 terminaux

Le module 5G-LENA v3.3.1, commit `f13da239`, est compilé dans `ns-3.43/contrib/nr`. Cette étiquette est prévue pour ns-3.42. Le scénario `scenario/imt2030-nr-gestion.cc` reprend la grille, le budget de 40 paquets de 512 octets et les politiques `baseline`, `adaptive` et `feedback`.

La radio est fixée avant les runs. Elle vaut 3,5 GHz, 20 MHz, numérologie 0, intérieur en visibilité, planificateur OFDMA round-robin, motif TDD à majorité montante. La capacité de 3 Mbit/s calcule le plafond. Elle ne règle pas la bande NR.

## Campagne exécutée

Le script `scripts/lancer_campagne.py` refait trois runs, sur 16 terminaux, à 40 ms et pendant 10 s. Les trois politiques reçoivent 327680 octets et livrent tous les paquets. La boucle compte 4 événements et ne change pas le volume. Le même volume est celui du Wi-Fi sur cette cellule non saturée.

À 36 terminaux et à 100 terminaux, la simulation s'arrête à 30 ms dans le RRC idéal, avec l'assertion RNTI 0. Ces tailles ne sont pas dans le CSV. Le plan de 36 runs n'a pas été exécuté.

## Rapport

Le rapport est [docs/latex/rapport_lab04.pdf](../../docs/latex/rapport_lab04.pdf).
