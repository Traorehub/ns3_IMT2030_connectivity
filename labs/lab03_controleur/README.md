# Lab 3 - Contrôleur d'admission

Ce laboratoire reprend le plan factoriel du Lab 2 et remplace le contrôleur qui n'agissait pas sur les paquets. Le Lab 2 appelait `Stop` et `Start` sur `UdpClient`. Ces appels n'annulaient pas l'émission déjà planifiée, de sorte que les octets reçus restaient identiques entre `baseline` et `adaptive`. Le source actuel, dans `labs/lab01_wifi_dense/scenario/imt2030-density-gestion.cc`, émet par un socket UDP planifié par le simulateur. Une station réduite au silence ne consomme pas son budget de 40 paquets.

## Loi

La capacité de référence vaut 3 Mbit/s. Elle ne change pas le débit physique `HtMcs7`. Elle sert à calculer le plafond `K`, égal à la partie entière de `3e6 * intervalle_s * 0,82 / (512 * 8)`, puis borné entre 1 et le nombre de terminaux. À 40 ms, 80 ms et 160 ms, ce plafond vaut 24, 48 et 96. Les terminaux d'indice strictement inférieur à `K` émettent pendant toute la simulation. Les autres restent silencieux. Le compteur `throttleEvents` vaut alors le nombre de terminaux exclus.

Le modèle radio reste le proxy Wi-Fi 802.11n du Lab 1, avec Yans et le SSID `imt2030-dense-zone`.

## Campagne

Le script `scripts/lancer_campagne.py` enchaîne les 24 runs, avec `gridSide` dans {4, 6, 8, 10}, `intervalMs` dans {40, 80, 160} et les deux politiques. La durée vaut 10 s. Le script `scripts/analyser_campagne.py` produit les SVG, ainsi que `livraison_campagne.png`, `octets_campagne.png` et `throttle_40ms.png`.

Le tableau officiel est `sorties/resultats.csv`. Lorsque `K` couvre toute la grille, les deux politiques coïncident. Lorsque `K` est plus petit, le taux de livraison des octets émis augmente et le volume reçu par le point d'accès diminue.

Sur 100 terminaux et 40 ms, la baseline reçoit 361472 octets, pour un taux de 0,177. L'admission, limitée à 24 terminaux, reçoit 287744 octets, pour un taux de 0,585, avec 76 événements et 24 flux. Le gain porte sur la fraction des octets émis qui arrivent. Le service agrégé de la cellule baisse.

L'indice de Jain du CSV est calculé sur les flux qui ont reçu au moins un octet. Les terminaux silencieux n'y figurent pas.

## Lecture d'un chiffre

Le volume reçu divisé par 512 donne le nombre de paquets arrivés. Le taux de livraison divise ce volume par les octets réellement émis. À 100 terminaux et 40 ms, la baseline émet 4000 paquets et en livre 706, d'où le taux 0,177. L'admission émet 960 paquets et en livre 562, d'où le taux 0,585. Le taux monte, et le nombre de paquets arrivés baisse. L'indice de Jain ne compte pas les stations qui n'ont rien reçu. Le compteur d'événements vaut ici le nombre de stations coupées une fois.

Ces grilles ont un espacement de 5 m. Leur densité spatiale reste de l'ordre de 50 000 terminaux par kilomètre carré. L'ordre de grandeur IMT-2030 retenu dans le dépôt va d'un million à cent millions. Le cas de 100 terminaux à 40 ms est sévère pour un point d'accès Wi-Fi. Il ne reproduit pas cette densité.

## Rapport

Le rapport détaillé est [docs/latex/rapport_lab03.pdf](../../docs/latex/rapport_lab03.pdf). Il précise aussi l'écart entre cette baseline et le CSV du Lab 2, produit avec `UdpClient`. Les deux fichiers ne doivent pas être fusionnés. Relancer le script du Lab 2 avec le source actuel écraserait cette archive.
