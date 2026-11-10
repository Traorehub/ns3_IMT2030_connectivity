# Lab 1 - Zone dense Wi-Fi (proxy de connectivité massive)

Ce laboratoire héberge le scénario principal `imt2030-density-gestion.cc`. La couche d'accès est du Wi-Fi 802.11n, avec `YansWifiPhyHelper`, le débit `HtMcs7` et le SSID `imt2030-dense-zone`. Le cadre IMT-2030 qualifie la question de densité et de gestion de charge. Le standard radio simulé reste le Wi-Fi.

Le Lab 2 a utilisé ce scénario avec un émetteur `UdpClient`. Le fichier contient désormais l'émetteur et l'admission du Lab 3, décrits dans `docs/latex/rapport_lab03.pdf`. Le rapport du Lab 2 reste `docs/latex/rapport_lab02.pdf`.

## Pile radio à citer

Le standard fixé est `WIFI_STANDARD_80211n`. Le canal et la physique passent par Yans. Les terminaux envoient du UDP vers un point d'accès. Aucun module NR ni 5G-LENA n'est chargé dans ce lab.

## Installation

Copiez le dossier `scenario/` dans `ns-3.43/scratch/projet-visuel-imt2030/`, puis lancez la compilation depuis la racine ns-3.

```bash
cd /chemin/vers/ns-3.43
./ns3 build scratch/projet-visuel-imt2030/imt2030-density-gestion
```

Les sorties par défaut vont dans `labs/lab01_wifi_dense/sorties/` lorsque vous exécutez depuis l'arborescence ns-allinone habituelle. Le chemin se change avec `--outputDir`.

## Campagne historique

Les commandes ci-dessous décrivent la référence du Lab 1, produite avec `UdpClient`. Sur une grille de 100 terminaux, un intervalle de 40 ms et une durée de 12 s, la livraison vers le point d'accès valait environ 0,186 pour les deux politiques. La politique adaptive enregistrait 50 événements, sans modifier les octets reçus. Le source actuel ne reproduit plus ces deux chiffres, parce que l'émetteur et l'admission ont changé au Lab 3. Le tableau archivé du plan à 10 s est `labs/lab02_campagne/sorties/resultats.csv`.

```bash
./ns3 run "scratch/projet-visuel-imt2030/imt2030-density-gestion --policy=baseline --gridSide=10 --simTime=12 --intervalMs=40"
./ns3 run "scratch/projet-visuel-imt2030/imt2030-density-gestion --policy=adaptive --gridSide=10 --simTime=12 --intervalMs=40"
```

## Schéma du CSV

Les nouveaux runs écrivent la colonne `intervalMs`. Le fichier `sorties/resultats.csv` de ce lab a été constitué avant cet ajout, et avant le changement d'émetteur du Lab 3. Il ne faut donc pas le fusionner avec le CSV du Lab 2, ni avec celui du Lab 3. La campagne `UdpClient` est `labs/lab02_campagne/sorties/resultats.csv`. La campagne avec admission est `labs/lab03_controleur/sorties/resultats.csv`.

## Figures

Le script `scripts/generer_toutes_les_images.py` lit `sorties/resultats.csv` et produit les SVG ainsi que `sorties/portfolio_visuel.html`. Les captures terminal se trouvent dans `docs/img/`.

![Topologie](sorties/image_topologie.svg)

![Comparatif baseline et adaptive](sorties/image_comparatif.svg)

![Capture terminal baseline et adaptive](docs/img/terminal-baseline-et-adaptive-10x10.png)

## Politique adaptive (version 0)

Toutes les 0,5 s, le contrôleur estime le rapport entre l'offre des terminaux actifs et une capacité de référence. Il met une fraction des terminaux en veille lorsque le seuil haut est dépassé, puis il en réactive un lorsque le seuil bas est atteint.

## Limites du lab

La grille est fixe. La capacité de référence est un paramètre. L'offre estimée se fonde sur le nombre de paquets prévus. Ces limites doivent figurer dans toute présentation scientifique. Le rapport est [docs/latex/rapport_lab01.pdf](../../docs/latex/rapport_lab01.pdf).
