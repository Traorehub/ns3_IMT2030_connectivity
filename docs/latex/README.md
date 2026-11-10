# Rapports PDF

Six sources sont maintenues dans ce dossier.

| Rapport | Source | PDF |
|---------|--------|-----|
| Lab 1 | `rapport_lab01.tex` | `rapport_lab01.pdf` |
| Lab 2 | `rapport_lab02.tex` | `rapport_lab02.pdf` |
| Lab 3 | `rapport_lab03.tex` | `rapport_lab03.pdf` |
| Lab 4 | `rapport_lab04.tex` | `rapport_lab04.pdf` |
| Lab 5 | `rapport_lab05.tex` | `rapport_lab05.pdf` |
| Synthèse | `rapport_general.tex` | `rapport_general.pdf` |

Compilation sous WSL, avec `texlive-latex-extra` et `texlive-lang-french` :

```bash
cd /mnt/c/Users/MOH/Documents/Projet_Perso/ns-allinone-3.43/projet-visuel-imt2030/docs/latex
pdflatex rapport_lab01.tex
pdflatex rapport_lab01.tex
pdflatex rapport_lab02.tex
pdflatex rapport_lab02.tex
pdflatex rapport_lab03.tex
pdflatex rapport_lab03.tex
pdflatex rapport_lab04.tex
pdflatex rapport_lab04.tex
pdflatex rapport_lab05.tex
pdflatex rapport_lab05.tex
pdflatex rapport_general.tex
pdflatex rapport_general.tex
```

Le Lab 2 inclut les PNG produits par `labs/lab02_campagne/scripts/analyser_campagne.py`. Le Lab 3 inclut les PNG produits par `labs/lab03_controleur/scripts/analyser_campagne.py`. Le Lab 5 inclut les PNG produits par `labs/lab05_ai_native/scripts/analyser_campagne.py`. Il faut donc lancer le script correspondant avant une recompilation, si les figures ont été effacées.
