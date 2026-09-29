# QRCodeGenerator

QRCodeGenerator est un projet C++17 qui transforme un texte ou une URL en une image PNG contenant une matrice de QR Code. La bibliothèque choisit une version, encode les données en mode octets, ajoute une correction Reed–Solomon, construit les motifs du QR Code, choisit un masque et écrit l’image.

> **État du projet : prototype.** Le dépôt contient la chaîne de génération décrite ci-dessous, mais le code présente une incohérence entre les valeurs de l’énumération `CorrectionLevel` et l’indexation des tables. Les niveaux demandés peuvent donc être mal interprétés. Le résultat n’a pas de test automatisé de décodage fourni avec le projet.

## Fonctionnalités présentes dans le code

- Encodage du texte en mode octets.
- Sélection automatique d’une version entre 1 et 40 selon la longueur du texte et le paramètre de correction.
- Calcul et entrelacement des octets Reed–Solomon en blocs.
- Dessin des motifs de position, de synchronisation et d’alignement, ainsi que des informations de format et de version.
- Placement des codewords dans les cellules libres de la matrice.
- Évaluation des huit masques QR et application de celui dont le score de pénalité est le plus bas.
- Création d’un PNG RGBA avec une échelle réglable et une marge de fond.
- Exemple en console qui demande le texte à encoder.

## Contenu du dépôt

```text
QRCodeGenerator/
├── CMakeLists.txt
├── src/
│   ├── CMakeLists.txt
│   └── QRCodeGenerator/
│       ├── QRCodeGenerator.h / .cpp
│       ├── QRCodeDataEncoding.h / .cpp
│       ├── QRCodeReedSolomonCorrector.h / .cpp
│       ├── QRCodeFunctionPatterns.h / .cpp
│       ├── QRCodeBitPlacement.h / .cpp
│       ├── QRCodeMasking.h / .cpp
│       ├── QRCodeFormatPattern.h / .cpp
│       ├── QRCodeBCHCode.h / .cpp
│       ├── QRCodePNGFile.h / .cpp
│       ├── QRCodeData.h
│       └── QRCodeColor.h
└── test/
    ├── CMakeLists.txt
    └── Main.cpp
```

`QRCodeFormatPattern` et `QRCodeBCHCode` sont présents dans le projet, mais le chemin principal de génération utilise `QRCodeFunctionPatterns` pour dessiner les informations de format et de version.

## Utilisation de la bibliothèque

La méthode publique principale prend le nom du fichier PNG, le texte, le niveau de correction et un facteur d’échelle facultatif :

```cpp
#include "QRCodeGenerator.h"

int main()
{
    QRCodeGenerator::GenerateQRCode(
        "QRCode.png",
        "https://www.lepetitportfolio.fr",
        CorrectionLevel::M);
}
```

Le facteur d’échelle par défaut est de 20 pixels par module. La version est choisie dans `QRCodeGenerator::GenerateQRCode` en appelant `QRCodeDataEncoding::SelectVersionForText`.

Une surcharge permet de choisir les couleurs RGBA des modules foncés, des modules clairs et du fond :

```cpp
QRCodeGenerator::GenerateQRCode(
    "QRCode.png",
    "https://www.lepetitportfolio.fr",
    CorrectionLevel::M,
    {20, 20, 20, 255},       // modules foncés
    {240, 240, 240, 255},    // modules clairs
    {240, 240, 240, 255},    // fond et marge
    20);                     // pixels par module
```

La première surcharge utilise ces mêmes couleurs par défaut. Le générateur ajoute une marge de fond de **2 modules** de chaque côté; cette valeur est le paramètre par défaut de la fonction interne `GenerateQRPixelData` et n’est pas exposée dans les surcharges publiques de `GenerateQRCode`.

Le masque est choisi automatiquement. La classe `QRCodeMasking` expose `SelectBestMask` pour comparer les huit candidats, mais `QRCodeGenerator::GenerateQRCode` ne propose pas de paramètre public pour imposer un numéro de masque.

Le programme d’exemple `test/Main.cpp` demande un texte ou une URL dans la console et enregistre `QRCode.png` dans le répertoire de travail courant.

## Étapes de génération

1. `QRCodeDataEncoding::SelectVersionForText` parcourt les versions de 1 à 40 et compare la taille nécessaire au nombre de codewords de données disponibles.
2. `QRCodeDataEncoding::EncodeTextToDataCodewords` ajoute l’indicateur de mode octets, la longueur, les octets du texte, le terminateur, l’alignement sur un octet et les codewords de remplissage `0xEC` et `0x11`.
3. `QRCodeReedSolomonCorrector::ErrorCorrectionAndInterleave` divise les données en blocs, calcule les octets ECC dans GF(256), puis entrelace les blocs.
4. `QRCodeMasking::SelectBestMask` crée une matrice pour chaque numéro de masque de 0 à 7, y place les données et calcule le score. En cas d’égalité, le premier numéro rencontré est conservé.
5. `QRCodeFunctionPatterns::Generate` crée la matrice et réserve les motifs fonctionnels. Elle inscrit aussi le niveau de correction et le masque dans les informations de format, ainsi que la version pour les versions 7 à 40.
6. `QRCodeBitPlacement::PlaceCodewords` place les bits des codewords entrelacés en zigzag dans les cases non réservées.
7. `QRCodeMasking::ApplyMask` inverse les cases de données désignées par le masque sélectionné.
8. `QRCodeGenerator::GenerateQRPixelData` transforme la matrice en pixels RGBA en ajoutant la marge de fond.
9. `QRCodePNGFile::GeneratePNGFile` écrit l’image PNG. Le code construit les blocs PNG et le flux zlib non compressé lui-même, sans bibliothèque PNG externe.

Le score de masque pénalise les suites trop longues d’une même couleur, les carrés uniformes de 2 × 2, les motifs de modules ressemblant aux repères de position et un déséquilibre entre modules foncés et clairs.

## Compilation

Prérequis : CMake 3.10 ou supérieur et un compilateur compatible avec C++17.

Depuis la racine du projet :

```sh
cmake -S . -B build
cmake --build build
```

La bibliothèque est construite sous forme de cible statique `QRCodeGenerator`. La cible `Test` est liée à cette bibliothèque et construit l’exemple interactif.

Les fichiers source sont collectés avec `GLOB_RECURSE` dans `src/CMakeLists.txt`. Après l’ajout de nouveaux fichiers `.cpp`, relance la configuration CMake.

## Limites et points à vérifier

- **Indexation des niveaux de correction :** `QRCodeData.h` déclare `L = 1`, `M = 0`, `Q = 3` et `H = 2`. Les tables `DataCodewords`, `EccCodewordsPerBlock` et `BlockCount` sont organisées dans l’ordre L/M/Q/H, mais le code les indexe directement avec la valeur de l’énumération. Ainsi, M utilise l’index des données L, L celui de M, Q celui de H et H celui de Q. Le mappage dans `QRCodeFunctionPatterns::DrawFormat` utilise également un tableau indexé par cette valeur et inverse les paires L/M et Q/H. Il faut corriger ce mappage avant de considérer la sélection de version et les niveaux de correction comme fiables.
- **Texte non ASCII :** la longueur est comptée en octets de `std::string`; aucun indicateur ECI ne précise l’encodage du texte. Les caractères UTF-8 accentués occupent plusieurs octets.
- **Marge :** la marge produite par défaut est de 2 modules. Elle n’est pas configurable par l’API publique de génération d’image.
- **Validation :** aucun test automatique de conformité ni de lecture par scanner n’est fourni. Vérifier les PNG générés avec un lecteur QR après correction de l’indexation des niveaux.
- **Facteur d’échelle :** l’API ne vérifie pas qu’il est strictement positif; fournir une valeur positive, par exemple `20`.

