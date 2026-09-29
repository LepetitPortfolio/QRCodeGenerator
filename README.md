# QRCodeGenerator

Projet C++17 qui encode un texte ou une URL dans un QR Code et l’enregistre en image PNG. La version du QR Code est choisie automatiquement selon le texte et le niveau de correction demandé. Le meilleur masque parmi les huit masques QR est également sélectionné automatiquement.

> **État : prototype en cours de développement.** Le projet comprend l’encodage en mode octets, la correction Reed–Solomon, les motifs fonctionnels, le placement des données, le choix et l’application d’un masque, ainsi que la création du PNG. La conformité complète du QR Code et sa lecture par différents scanners ne sont pas garanties par le dépôt.

## Fonctionnalités

- Encodage du texte en mode octets.
- Choix automatique de la plus petite version adaptée, de 1 à 40.
- Paramètre de niveau de correction `L`, `M`, `Q` ou `H` (voir le problème d’indexation signalé dans les limites ci-dessous).
- Ajout et entrelacement des codewords de correction Reed–Solomon.
- Génération des repères de position, de synchronisation, d’alignement, des informations de format et de version.
- Placement des codewords dans la matrice.
- Évaluation des huit masques et application de celui dont le score de pénalité est le plus faible.
- Export PNG en couleurs RGBA, avec échelle réglable et marge de fond.
- Petit programme de démonstration qui demande le texte dans la console.

## Organisation

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

## Utilisation de la bibliothèque

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

La version de QR est choisie automatiquement. Le dernier paramètre facultatif règle l’échelle en pixels par module; sa valeur par défaut est `20`.

Une surcharge permet de définir les couleurs des modules foncés, des modules clairs et du fond :

```cpp
QRCodeGenerator::GenerateQRCode(
    "QRCode.png",
    "https://www.lepetitportfolio.fr",
    CorrectionLevel::M,
    {20, 20, 20, 255},   // module foncé
    {240, 240, 240, 255}, // module clair
    {240, 240, 240, 255}, // fond / marge
    20);                 // pixels par module
```

Le masque est sélectionné automatiquement par `QRCodeMasking::SelectBestMask`; l’API publique de `QRCodeGenerator` ne permet pas de le forcer. L’image est enregistrée dans le répertoire de travail du programme sous le nom fourni.

## Fonctionnement de la génération

1. `QRCodeDataEncoding::SelectVersionForText` parcourt les versions 1 à 40 et retient la première qui peut contenir le texte pour le niveau demandé.
2. `QRCodeDataEncoding::EncodeTextToDataCodewords` construit les données en mode octets, ajoute le terminateur et les octets de remplissage.
3. `QRCodeReedSolomonCorrector::ErrorCorrectionAndInterleave` calcule les codewords de correction, les répartit en blocs puis entrelace les blocs.
4. `QRCodeMasking::SelectBestMask` construit une matrice candidate pour chacun des huit masques et conserve le masque au score de pénalité le plus faible. En cas d’égalité, le premier masque rencontré (donc le numéro le plus faible) reste choisi.
5. `QRCodeFunctionPatterns::Generate` dessine les motifs fixes et inscrit le niveau de correction ainsi que le numéro du masque dans l’information de format. Les informations de version sont générées à partir de la version 7.
6. `QRCodeBitPlacement::PlaceCodewords` place les bits dans les cellules qui ne sont pas réservées.
7. `QRCodeMasking::ApplyMask` applique le masque aux seules cellules de données.
8. `QRCodeGenerator::GenerateQRPixelData` transforme la matrice en pixels RGBA et laisse une marge de fond autour du code. La valeur par défaut interne de cette marge est de **2 modules**.
9. `QRCodePNGFile::GeneratePNGFile` écrit le fichier PNG.

Le score du masque pénalise les longues suites de modules identiques, les blocs monochromes 2 × 2, les motifs proches des repères de position et un déséquilibre entre modules foncés et clairs.

## Compilation avec CMake

Prérequis : CMake 3.10 ou supérieur et un compilateur compatible C++17.

À lancer depuis la racine du projet :

```sh
cmake -S . -B build
cmake --build build
```

La bibliothèque statique s’appelle `QRCodeGenerator`. La cible `Test` construit le programme de démonstration et est liée à cette bibliothèque dans `test/CMakeLists.txt`.

Pour lancer le programme, exécuter la cible `Test` depuis l’environnement de compilation. Il demande le texte ou l’URL, puis écrit `QRCode.png` dans le répertoire de travail courant.

Les fichiers `.cpp` sont collectés par `GLOB_RECURSE` dans `src/CMakeLists.txt`. Si de nouveaux fichiers sont ajoutés au projet, relancer la configuration CMake.

## Limites visibles dans le code

- Le contenu est encodé en mode octets. La taille de `std::string` est interprétée comme un nombre d’octets. Aucun marqueur ECI n’est ajouté pour indiquer explicitement l’encodage du texte (par exemple UTF-8).
- La marge blanche par défaut du générateur est de 2 modules. La norme QR recommande généralement une zone libre plus large; cette implémentation ne met pas 4 modules par défaut.
- **Indexation des niveaux à corriger :** `CorrectionLevel` déclare `L=1`, `M=0`, `Q=3`, `H=2`, alors que les tables de capacité et de Reed–Solomon sont indexées dans l’ordre L/M/Q/H et consultées avec la valeur numérique de l’énumération. Le niveau sélectionné peut donc utiliser les paramètres d’un autre niveau (M/L et Q/H sont inversés). Les niveaux sont acceptés par l’API, mais ne doivent pas être considérés comme correctement pris en charge tant que cette incohérence n’est pas corrigée.
- Les fonctions et tables sont prévues pour les versions 1 à 40; cette documentation ne constitue pas une validation de conformité de toutes les versions.
- Le dépôt ne contient pas de tests automatisés de décodage; vérifie le PNG produit avec un lecteur QR avant de t’y fier.
- Des fichiers tels que `QRCodeFormatPattern` et `QRCodeBCHCode` sont présents dans la bibliothèque, mais le chemin principal construit les motifs et les informations de format/version via `QRCodeFunctionPatterns`.
