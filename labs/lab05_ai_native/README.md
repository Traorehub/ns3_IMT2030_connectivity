# Lab 5 - Boucle fermée sur la livraison

Ce laboratoire ajoute la politique `feedback` au scénario Wi-Fi des labs précédents. Le plafond part de la valeur du Lab 3, calculée avec une capacité de référence de 3 Mbit/s. Toutes les 0,5 s, il baisse si la livraison de la fenêtre est inférieure à 0,35, et il monte si elle est supérieure à 0,50. La radio reste le proxy 802.11n, avec Yans et `HtMcs7`. La boucle n'est pas un modèle entraîné, et elle n'utilise pas `ns3-gym`.

## Campagne

Le script `scripts/lancer_campagne.py` exécute 12 runs, sur `gridSide` dans {4, 6, 8, 10} et `intervalMs` dans {40, 80, 160}. Le script `scripts/analyser_campagne.py` rapproche ces runs des politiques baseline et adaptive du CSV du Lab 3.

À 100 terminaux et 40 ms, la boucle reçoit 865280 octets, soit 1690 paquets, pour un taux de 0,819 et 56 flux. La baseline du Lab 3 en reçoit 706, et l'admission fixe en reçoit 562. Le volume monte parce que de nouvelles stations prennent le relais après le budget de 40 paquets. Le plafond final de 56 n'est pas un effectif simultané pendant les 10 s.

Sur 64 terminaux à 80 ms, le volume de la boucle dépasse l'admission fixe, et le taux lui est inférieur, 0,677 contre 0,708. Cette cellule est rapportée dans le PDF afin de ne pas généraliser un gain sur les deux métriques à la fois.

## Rapport

Le rapport détaillé est [docs/latex/rapport_lab05.pdf](../../docs/latex/rapport_lab05.pdf). La synthèse des cinq labs est [docs/latex/rapport_general.pdf](../../docs/latex/rapport_general.pdf).
