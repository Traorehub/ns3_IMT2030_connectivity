# Publier ce dossier sur GitHub

Depot recommande : **uniquement** `projet-visuel-imt2030/` (pas tout `ns-allinone-3.43`).

## 1. Creer le depot sur github.com

Nom suggere : `imt2030-ns3-zone-dense` ou `ns3-density-imt2030`.

## 2. Init git (PowerShell ou WSL)

```bash
cd /mnt/c/Users/MOH/Documents/Projet_Perso/ns-allinone-3.43/projet-visuel-imt2030

git init
git add README.md RAPPORT.md CAPTURES.md PUBLIER_GITHUB.md .gitignore
git add scenario/ scripts/ docs/ sorties/resultats.csv sorties/*.svg sorties/portfolio_visuel.html
git add docs/img/*.png

git commit -m "Simulation ns-3 : gestion de charge zone dense (cadre IMT-2030)"
git branch -M main
git remote add origin https://github.com/VOTRE_COMPTE/imt2030-ns3-zone-dense.git
git push -u origin main
```

Remplacez l'URL par votre depot.

## 3. Ce qui est deja pret pour le README

- Captures terminal : `docs/img/`
- Figures SVG : `sorties/image_topologie.svg`, `sorties/image_comparatif.svg`
- Donnees : `sorties/resultats.csv`
- Code source : `scenario/imt2030-density-gestion.cc`

## 4. Installation ns-3 (recruteur / jury)

Copier `scenario/*` vers `ns-3.xx/scratch/projet-visuel-imt2030/` dans une installation ns-3.43+, puis :

```bash
./ns3 build scratch/projet-visuel-imt2030/imt2030-density-gestion
```

Voir le README principal.
