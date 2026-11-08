# Rapports PDF

Deux sources sont maintenues dans ce dossier.

| Rapport | Source | PDF |
|---------|--------|-----|
| Lab 1 | `rapport_lab01.tex` | `rapport_lab01.pdf` |
| Lab 2 | `rapport_lab02.tex` | `rapport_lab02.pdf` |

Compilation sous WSL, avec `texlive-latex-extra` et `texlive-lang-french` :

```bash
cd /mnt/c/Users/MOH/Documents/Projet_Perso/ns-allinone-3.43/projet-visuel-imt2030/docs/latex
pdflatex rapport_lab01.tex
pdflatex rapport_lab01.tex
pdflatex rapport_lab02.tex
pdflatex rapport_lab02.tex
```

Le Lab 2 inclut les PNG produits par `labs/lab02_campagne/scripts/analyser_campagne.py`. Il faut donc lancer ce script avant une recompilation, si les figures ont été effacées.
