# QRCodeGenerator

Petit projet C++17 qui vise à générer un QR Code à partir d'un texte ou d'une URL, puis à l'enregistrer au format PNG. La version du QR Code est choisie automatiquement selon la longueur du texte et le niveau de correction demandé.

> **État du projet : prototype en cours de développement.** L'application d'un masque choisi est maintenant intégrée. La sélection automatique du meilleur masque, la bordure blanche de sécurité et des vérifications avec un lecteur QR restent à faire.

## Fonctionnalités prévues

- Encodage du texte en mode octets.
- Sélection de la plus petite version QR adaptée, parmi les versions 1 à 40.
- Niveaux de correction L, M, Q et H avec correction Reed–Solomon.
- Motifs fonctionnels, placement des codewords et génération d'une image PNG.
- Application d'un masque QR choisi parmi les huit masques normalisés (0 à 7).
- Choix de la taille en pixels par module et des couleurs RGBA.

## Organisation du projet

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
│       ├── QRCodePNGFile.h / .cpp
│       ├── QRCodeData.h
│       ├── QRCodeColor.h
│       └── QRCodeBCHCode.h / .cpp
└── test/
    ├── CMakeLists.txt
    └── Main.cpp
```

## Utilisation de la bibliothèque

La méthode principale demande un nom de fichier, le texte, un niveau de correction, puis éventuellement l'échelle et le numéro de masque :

```cpp
#include "QRCodeGenerator.h"

#include <string>

int main()
{
    const std::string texte = "https://www.lepetitportfolio.fr";

    // Génère QRCode.png avec correction M, 20 pixels par module et le masque 0.
    QRCodeGenerator::GenerateQRCode(
        "QRCode.png",
        texte,
        CorrectionLevel::M,
        20,
        0);
}
```

La première surcharge utilise des couleurs par défaut : modules foncés en `{20, 20, 20, 255}` et fond clair en `{240, 240, 240, 255}`. Une seconde surcharge permet de fournir les couleurs RGBA.

Le fichier `QRCode.png` est créé dans le répertoire de travail du programme. Le niveau de correction accepte `CorrectionLevel::L`, `CorrectionLevel::M`, `CorrectionLevel::Q` ou `CorrectionLevel::H`. Le masque doit être compris entre 0 et 7.

Le numéro du masque doit être le même dans les informations de format et dans l'appel à `QRCodeMasking`. Le projet applique le numéro fourni ; il ne compare pas encore les huit masques pour retenir automatiquement celui qui donne la matrice la plus lisible.

## Chaîne de génération

1. `QRCodeDataEncoding` sélectionne une version et convertit le texte en codewords de données.
2. `QRCodeReedSolomonCorrector` répartit les données en blocs, calcule les octets de correction et entrelace les blocs.
3. `QRCodeFunctionPatterns` dessine les motifs de position, de synchronisation, d'alignement, de format et de version, en marquant les cases réservées.
4. `QRCodeBitPlacement` place les codewords dans les cases disponibles de la matrice.
5. `QRCodeMasking` applique le masque choisi aux seules cases non réservées.
6. `QRCodeGenerator` transforme les modules en pixels RGBA et `QRCodePNGFile` écrit l'image PNG.

## Compilation

Prérequis : un compilateur C++17 et CMake 3.10 ou plus récent.

Depuis la racine du projet :

```sh
cmake -S . -B build
cmake --build build --target QRCodeGenerator
```

Cela configure le projet et construit la bibliothèque statique `QRCodeGenerator`. Comme CMake collecte les fichiers source avec `GLOB_RECURSE`, relancer la configuration après l'ajout d'un nouveau fichier `.cpp`.

### Exemple interactif

Le projet contient un exemple dans `test/Main.cpp`. Dans l'archive actuelle, la configuration CMake de `test` ne lie pas encore la cible `Test` à la bibliothèque `QRCodeGenerator`. Pour construire cet exécutable, ajouter dans `test/CMakeLists.txt` :

```cmake
target_link_libraries(Test PRIVATE QRCodeGenerator)
```

L'exemple passe actuellement `0` comme quatrième argument à `GenerateQRCode`, qui correspond à l'échelle en pixels. Utiliser l'appel sans cet argument pour prendre l'échelle par défaut, ou passer une valeur positive, par exemple `20`.

## Limites connues

- L'encodage du contenu est en mode octets. La longueur correspond au nombre d'octets de `std::string`; les textes UTF-8 accentués peuvent donc occuper plusieurs octets. Aucun marqueur ECI n'est ajouté pour préciser l'encodage Unicode.
- Le masque fourni (0 à 7) est appliqué et indiqué dans les informations de format ; le programme ne choisit pas automatiquement le meilleur masque.
- `QRCodeFormatPattern` et `QRCodeBCHCode` sont encore présents dans le projet, mais le chemin principal génère maintenant le format avec `QRCodeFunctionPatterns`.
- L'image PNG ne reçoit pas encore la bordure blanche standard (quiet zone) autour de la matrice.
- Le PNG doit encore être vérifié avec un lecteur QR ; sa lecture n'a pas été validée.