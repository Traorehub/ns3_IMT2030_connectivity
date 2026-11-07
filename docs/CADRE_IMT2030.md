# Cadre IMT-2030 et honnetete du modele

Le depot porte le nom `ns3_IMT2030_connectivity` parce qu'il organise une recherche sur la **connectivite massive** et la **gestion de charge en zone dense**, themes presents dans le cadre IMT-2030 et dans la recommandation ITU-R M.2160.

Le code livre dans le Lab 1 n'implemente pas la pile radio 6G. Il s'appuie sur Wi-Fi 802.11n dans ns-3 : helpers Yans, standard `WIFI_STANDARD_80211n`, debit `HtMcs7`, SSID `imt2030-dense-zone`. Ce choix permet de travailler la methodologie avant d'installer 5G-LENA.

La traduction entre vision IMT et simulation se fait par une **zone representative** : une grille de `gridSide` par `gridSide` terminaux autour d'un point d'acces. Cette grille ne reproduit pas une surface d'un kilometre carre ni des millions de noeuds discrets.

Dans un memoire ou un entretien, vous pouvez formuler ainsi : le Lab 1 etudie les metriques et les politiques de charge sur un proxy Wi-Fi dense, tandis que les labs suivants rapprocheront le modele du NR puis des boucles de controle de type RAN intelligent.
