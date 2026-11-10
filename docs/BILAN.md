# Bilan

Ce dépôt suit une progression en cinq laboratoires sur la gestion de charge en zone dense, dans une perspective IMT-2030. Le simulateur est ns-3.43, exécuté sous WSL. Les quatre premiers labs reposent sur un proxy Wi-Fi 802.11n. Le Lab 4 ajoute une cellule NR 5G-LENA, limitée à 16 terminaux sur cet arbre.

La question centrale est la suivante. Lorsqu'une règle limite ou réordonne les émetteurs, le volume de paquets qui arrive au point de collecte change-t-il, et à quel prix sur le taux de livraison des octets réellement émis ?

Sur 100 terminaux et 40 ms, le proxy Wi-Fi reçoit 706 paquets sans contrôle, 562 avec une admission fixe à 24 terminaux, et 1690 avec une boucle qui ajuste ce plafond selon la livraison mesurée. La grille correspond à environ 50 000 terminaux par kilomètre carré, et non à l'ordre de grandeur d'un million à cent millions visé par IMT-2030.

Le Lab 2 archive une campagne avec `UdpClient`, où le contrôleur adaptive était sans effet sur les octets. Le Lab 3 corrige l'émetteur et montre que l'admission fixe élève le taux et réduit le volume. Le Lab 4 compile 5G-LENA v3.3.1 et livre les 640 paquets sur 16 terminaux. Les grilles de 36 et de 100 terminaux s'y arrêtent dans le RRC idéal. Le Lab 5 n'est pas un modèle entraîné. Il applique deux seuils à une fenêtre de 0,5 s.

La synthèse PDF est [latex/rapport_general.pdf](latex/rapport_general.pdf). Le cadre IMT-2030 est [CADRE_IMT2030.md](CADRE_IMT2030.md).
