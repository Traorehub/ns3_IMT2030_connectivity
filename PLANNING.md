# Planning des cinq labs

Ce document fixe la progression du dépôt `ns3_IMT2030_connectivity`. Chaque lab possède son dossier sous `labs/` et son propre README.

Les Labs 1 à 3 et le Lab 5 sont des campagnes Wi-Fi. Le Lab 4 est une campagne NR partielle, limitée à 16 terminaux.

## Lab 1 (livré)

Dossier `labs/lab01_wifi_dense/`. Le scénario ns-3 utilise le module Wi-Fi 802.11n, avec Yans et HtMcs7. Il joue le rôle de proxy pour une zone dense de type connectivité massive. Le rapport est `docs/latex/rapport_lab01.pdf`.

## Lab 2 (livré)

Dossier `labs/lab02_campagne/`. Le plan factoriel compte 24 runs, sur `gridSide`, `intervalMs` et la politique. Le rapport est `docs/latex/rapport_lab02.pdf`.

## Lab 3 (livré)

Dossier `labs/lab03_controleur/`. L'émetteur `UdpClient` est remplacé par des sockets planifiés, et l'admission permanente ne laisse émettre que `K` terminaux. Sur le plan de 24 runs, avec une capacité de référence de 3 Mbit/s, le taux de livraison des octets émis augmente lorsque `K` est inférieur à la taille de la grille, tandis que le volume reçu par le point d'accès diminue. Le rapport est `docs/latex/rapport_lab03.pdf`.

## Lab 4 (livré, partiel)

Dossier `labs/lab04_5g_lena/`. Le module 5G-LENA v3.3.1 est compilé dans `ns-3.43/contrib/nr`. Sur 16 terminaux et 40 ms, les trois politiques reçoivent 327680 octets. Sur 36 et 100 terminaux, le RRC idéal s'arrête à 30 ms. Le rapport est `docs/latex/rapport_lab04.pdf`.

## Lab 5 (livré)

Dossier `labs/lab05_ai_native/`. La politique `feedback` ajuste le plafond selon la livraison mesurée sur 0,5 s. Le rapport est `docs/latex/rapport_lab05.pdf`. La synthèse des cinq labs est `docs/latex/rapport_general.pdf`. La boucle n'est pas un modèle entraîné.

## Ordre recommandé

La lecture des Labs 1 à 3 et 5 se fait sur le proxy Wi-Fi. Le Lab 4 ajoute la cellule NR de 16 terminaux, sans reprendre les grilles de 36, 64 et 100 terminaux.
