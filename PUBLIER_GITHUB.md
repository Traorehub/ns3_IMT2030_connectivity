# Publier sur GitHub (Traorehub)

Depot local : **commit initial deja fait** sur la branche `main`.

## 1. Creer le depot vide sur GitHub

Compte : **Traorehub**

Nom suggere (meme esprit que `Federated_Learning_station_meteo`) :

`ns3_IMT2030_zone_dense` ou `IMT2030_ns3_dense_wifi`

Ne pas cocher README / .gitignore (deja presents localement).

## 2. Push (WSL)

```bash
cd /mnt/c/Users/MOH/Documents/Projet_Perso/ns-allinone-3.43/projet-visuel-imt2030

git remote add origin https://github.com/Traorehub/ns3_IMT2030_zone_dense.git

git push -u origin main
```

(Adapter l'URL si vous choisissez un autre nom.)

## 3. A lire avant push

- **README.md** = rapport + presentation (style proche de [Federated_Learning_station_meteo](https://github.com/Traorehub/Federated_Learning_station_meteo)) : badges, resultats numerotes, visuels, prérequis, commandes.
- **docs/BILAN.md** = synthese courte (equivalent du BILAN du depot FL).
- **RAPPORT.md** redirige vers README.

## 4. Fichiers visuels inclus

| Fichier | Role |
|---------|------|
| `docs/img/*.png` | Captures terminal GitHub |
| `sorties/image_*.svg` | Figures dans README |
| `sorties/portfolio_visuel.html` | Apercu complet (navigateur) |
| `sorties/resultats.csv` | Donnees des runs |

## 5. Apres le push

Verifier sur GitHub que les images du README s'affichent (chemins relatifs `docs/img/` et `sorties/`).
