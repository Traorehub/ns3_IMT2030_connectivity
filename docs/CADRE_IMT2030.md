# Cadre IMT-2030 et honnêteté du modèle

Le dépôt porte le nom `ns3_IMT2030_connectivity` parce qu'il organise une recherche sur la connectivité massive et la gestion de charge en zone dense. Ces thèmes figurent dans le cadre IMT-2030 et dans la recommandation ITU-R M.2160.

Le code livré dans les Labs 1 et 2 s'appuie sur Wi-Fi 802.11n dans ns-3. Les helpers sont ceux de Yans. Le standard est `WIFI_STANDARD_80211n`. Le débit de données est `HtMcs7` et le SSID est `imt2030-dense-zone`. Le Lab 2 réutilise ce même binaire et fait varier la taille de la grille ainsi que l'intervalle entre paquets.

La traduction entre la vision IMT et la simulation passe par une zone représentative. Une grille de `gridSide` par `gridSide` terminaux entoure un point d'accès. Cette grille ne reproduit pas une surface d'un kilomètre carré, ni un dénombrement de plusieurs millions de nœuds discrets.

Dans un mémoire ou un entretien, la formulation suivante reste exacte. Les Labs 1 et 2 étudient les métriques et les politiques de charge sur un proxy Wi-Fi dense. Les labs suivants rapprocheront le modèle du NR, puis des boucles de contrôle de type RAN intelligent.
