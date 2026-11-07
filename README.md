# ns3_IMT2030_connectivity

Ce depot GitHub rassemble une progression par laboratoires autour de la connectivite massive et de la gestion de charge, dans une perspective IMT-2030. Le simulateur est ns-3.43. Seul le **Lab 1** contient aujourd'hui un scenario executable et des resultats publies.

Le code radio du Lab 1 reste du **Wi-Fi 802.11n** (couche Yans, MCS HtMcs7). Il sert de proxy pedagogique pour une zone dense. Il ne faut pas le presenter comme une simulation de la pile 6G. Le fichier [docs/CADRE_IMT2030.md](docs/CADRE_IMT2030.md) developpe cette distinction. Le planning complet figure dans [PLANNING.md](PLANNING.md).

## Organisation des labs

| Dossier | Statut | Contenu actuel |
|---------|--------|----------------|
| [labs/lab01_wifi_dense](labs/lab01_wifi_dense/) | Livre | Scenario, sorties CSV, figures, captures |
| [labs/lab02_campagne](labs/lab02_campagne/) | Planifie | README de cadrage |
| [labs/lab03_controleur](labs/lab03_controleur/) | Planifie | README de cadrage |
| [labs/lab04_5g_lena](labs/lab04_5g_lena/) | Planifie | README de cadrage |
| [labs/lab05_ai_native](labs/lab05_ai_native/) | Planifie | README de cadrage |

## Entree rapide Lab 1

Copiez `labs/lab01_wifi_dense/scenario/` vers `ns-3.43/scratch/projet-visuel-imt2030/`, puis compilez et executez en suivant le [README du Lab 1](labs/lab01_wifi_dense/README.md).

## Licence

MIT. Voir [LICENSE](LICENSE).
