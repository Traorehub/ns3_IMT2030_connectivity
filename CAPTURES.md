# Obtenir des images pour le CV / rapport

## 1. Relancer une simu (met a jour topologie + NetAnim)

```bash
cd /mnt/c/Users/MOH/Documents/Projet_Perso/ns-allinone-3.43/ns-3.43

./ns3 build scratch/projet-visuel-imt2030/imt2030-density-gestion

./ns3 run "scratch/projet-visuel-imt2030/imt2030-density-gestion --policy=adaptive --gridSide=10 --simTime=12 --intervalMs=40"
./ns3 run "scratch/projet-visuel-imt2030/imt2030-density-gestion --policy=baseline --gridSide=10 --simTime=12 --intervalMs=40"
```

Fichiers generes dans `projet-visuel-imt2030/sorties/` :

- `topologie.csv`
- `imt2030-animation.xml` (NetAnim)
- `resultats.csv`

Desactiver NetAnim (fichier plus leger) : `--enableAnim=0`

## 2. Generer les figures SVG + page web

```bash
python3 /mnt/c/Users/MOH/Documents/Projet_Perso/ns-allinone-3.43/projet-visuel-imt2030/scripts/generer_toutes_les_images.py
```

Sorties :

| Fichier | Contenu |
|---------|---------|
| `image_topologie.svg` | Grille STA + AP |
| `image_comparatif.svg` | Barres baseline vs adaptive |
| `portfolio_visuel.html` | Page unique pour captures |

## 3. Ouvrir sous Windows (navigateur)

Coller dans Edge / Chrome :

```
file:///C:/Users/MOH/Documents/Projet_Perso/ns-allinone-3.43/projet-visuel-imt2030/sorties/portfolio_visuel.html
```

**Win + Shift + S** : selection de zone pour enregistrer des **PNG** dans le presse-papiers ou le dossier Captures.

## 4. NetAnim (si l'application ne plante pas)

```bash
cd /mnt/c/Users/MOH/Documents/Projet_Perso/ns-allinone-3.43/netanim-3.109
./NetAnim
```

Fichier a ouvrir : `projet-visuel-imt2030/sorties/imt2030-animation.xml`

Capture d'ecran pendant la lecture (bouton Play).

Si **segmentation fault** : utiliser uniquement les SVG et la capture terminal (sections 2-3).

## 5. Liste minimale pour un README GitHub

1. `image_topologie.svg` (converti en PNG si besoin)
2. `image_comparatif.svg`
3. Capture terminal avec les metriques
4. (Optionnel) capture NetAnim

Conversion SVG vers PNG en ligne de commande (si `inkscape` installe) :

```bash
inkscape sorties/image_topologie.svg --export-filename=sorties/image_topologie.png
```
