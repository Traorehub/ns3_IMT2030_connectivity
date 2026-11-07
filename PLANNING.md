# Planning des cinq labs

Ce document fixe la progression du depot `ns3_IMT2030_connectivity`. Chaque lab possede son dossier sous `labs/` et son propre README lorsque le travail est engage.

Le Lab 1 est le seul lab termine a ce jour. Les Labs 2 a 5 sont planifies et comportent pour l'instant un README de cadrage.

## Lab 1 (en cours de publication)

Dossier `labs/lab01_wifi_dense/`. Le scenario ns-3 utilise le module Wi-Fi 802.11n (Yans, HtMcs7). Il joue le role de proxy pour une zone dense type connectivite massive, sans implementer la radio 6G.

## Lab 2 (a venir)

Dossier `labs/lab02_campagne/`. Campagne systematique sur `gridSide` et `intervalMs`, graphiques a partir du CSV, protocole experimental ecrit.

## Lab 3 (a venir)

Dossier `labs/lab03_controleur/`. Controleur `adaptive` revisionne pour modifier une metrique de livraison, pas seulement compter les evenements de veille.

## Lab 4 (a venir)

Dossier `labs/lab04_5g_lena/`. Introduction du module 5G-LENA (NR 3GPP) en remplacement progressif du proxy Wi-Fi.

## Lab 5 (a venir)

Dossier `labs/lab05_ai_native/`. Boucle de controle documentee, eventuellement apprentissage via ns3-gym, dans la lignee des architectures RIC.

## Ordre recommande

Vous devez valider le Lab 1 et le Lab 2 avant d'engager le Lab 4, car la montée en complexite radio et la courbe d'apprentissage ns-3 se cumulent sinon.
