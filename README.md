# ns3_IMT2030_connectivity

Ce dépôt organise une progression par laboratoires sur la connectivité massive et la gestion de charge, dans une perspective IMT-2030. Le simulateur est ns-3.43, exécuté sous WSL. Les Labs 1 et 2 sont livrés. Les Labs 3 à 5 restent au stade du cadrage.

Le code radio livré est du Wi-Fi 802.11n, avec la couche Yans et le débit HtMcs7. Il sert de proxy pour une zone dense. Le fichier [docs/CADRE_IMT2030.md](docs/CADRE_IMT2030.md) précise le lien avec la recommandation ITU-R M.2160. Le planning figure dans [PLANNING.md](PLANNING.md).

## Organisation des labs

| Dossier | Statut | Contenu |
|---------|--------|---------|
| [labs/lab01_wifi_dense](labs/lab01_wifi_dense/) | Livré | Scénario C++, comparaison baseline / adaptive, captures |
| [labs/lab02_campagne](labs/lab02_campagne/) | Livré | Plan factoriel de 24 runs, figures, rapport |
| [labs/lab03_controleur](labs/lab03_controleur/) | Planifié | README de cadrage |
| [labs/lab04_5g_lena](labs/lab04_5g_lena/) | Planifié | README de cadrage |
| [labs/lab05_ai_native](labs/lab05_ai_native/) | Planifié | README de cadrage |

## Rapports PDF

Le Lab 1 est décrit dans [docs/latex/rapport_lab01.pdf](docs/latex/rapport_lab01.pdf). Le Lab 2 est décrit dans [docs/latex/rapport_lab02.pdf](docs/latex/rapport_lab02.pdf). Les sources LaTeX sont dans [docs/latex/](docs/latex/).

## Reproduire le Lab 1

Copiez `labs/lab01_wifi_dense/scenario/` vers `ns-3.43/scratch/projet-visuel-imt2030/`, puis suivez le [README du Lab 1](labs/lab01_wifi_dense/README.md).

## Reproduire le Lab 2

Depuis la racine `ns-allinone-3.43`, après compilation du scratch :

```bash
python3 projet-visuel-imt2030/labs/lab02_campagne/scripts/lancer_campagne.py
python3 projet-visuel-imt2030/labs/lab02_campagne/scripts/analyser_campagne.py
```

Le plan croise `gridSide` égal à 4, 6, 8 et 10 avec `intervalMs` égal à 40, 80 et 160, pour les politiques baseline et adaptive. La durée vaut 10 s. Les 24 lignes sont dans `labs/lab02_campagne/sorties/resultats.csv`.

Sur 16 terminaux, la livraison vaut 1. Sur 100 terminaux et un intervalle de 40 ms, elle vaut 0,186. Le contrôleur adaptive n'enregistre des événements que dans cette dernière cellule, au nombre de 50, sans modifier les octets reçus.

## Licence

MIT. Voir [LICENSE](LICENSE).
