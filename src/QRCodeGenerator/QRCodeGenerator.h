#pragma once
#include "QRCodeData.h"

#include <string>

class QRCodeGenerator
{
public:

	/**
	* @brief Génère un code QR et l'enregistre dans un fichier PNG avec des couleurs par défaut.
	* @param _Filename Nom du fichier PNG de sortie.
	* @param _Text Texte à encoder dans le code QR.
	* @param _CorrectionLevel Niveau de correction d'erreur (L, M, Q, ou H).
	* @param _PixelScale Facteur d'échelle pour l'image (nombre de pixels par module).
	* @param _MaskPatern Masque de données à appliquer (de 0 à 7).
	*
	* @details
	* - Appelle la version complète de `GenerateQRCode` avec des couleurs par défaut :
	*   - `_Bit1Color` : Noir (`{ 20, 20, 20, 255 }`).
	*   - `_Bit0Color` : Blanc (`{ 240, 240, 240, 255 }`).
	*   - `_FontColor` : Blanc (`{ 240, 240, 240, 255 }`).
	* - Cela permet de générer un code QR en **noir et blanc** avec un fond blanc.
	*/
	static void GenerateQRCode(const std::string& _Filename, const std::string _Text, CorrectionLevel _CorrectionLevel, int _PixelScale = 20, uint8_t _MaskPatern = 0);

	/**
	* @brief Génère un code QR et l'enregistre dans un fichier PNG avec des couleurs personnalisées.
	* @param _Filename Nom du fichier PNG de sortie.
	* @param _Text Texte à encoder dans le code QR.
	* @param _CorrectionLevel Niveau de correction d'erreur (L, M, Q, ou H).
	* @param _Bit1Color Couleur des modules noirs (1) au format RGBA.
	* @param _Bit0Color Couleur des modules blancs (0) au format RGBA.
	* @param _FontColor Couleur de la police (non utilisée dans cette implémentation).
	* @param _PixelScale Facteur d'échelle pour l'image (nombre de pixels par module).
	* @param _MaskPatern Masque de données à appliquer (de 0 à 7).
	*
	* @details
	* ### Étapes de la génération :
	* 1. **Sélection de la version** :
	*    - Utilise `QRCodeDataEncoding::SelectVersionForText(_Text, _CorrectionLevel)` pour déterminer la version minimale nécessaire.
	*
	* 2. **Génération du code QR simple** :
	*    - Appelle `GenerateSimpleQR(_Text, version, _CorrectionLevel, _MaskPatern)` pour créer une matrice de code QR avec les motifs fonctionnels et les données placées.
	*
	* 3. **Application des couleurs** :
	*    - Définit les couleurs pour les modules noirs (`Bit1Color`), blancs (`Bit0Color`), et la police (`FontColor`).
	*
	* 4. **Génération des données de pixels** :
	*    - Appelle `GenerateQRPixelData(data, _PixelScale)` pour convertir la matrice en données de pixels (RGBA).
	*
	* 5. **Génération du fichier PNG** :
	*    - Appelle `QRCodePNGFile::GeneratePNGFile` pour enregistrer l'image PNG.
	*/
	static void GenerateQRCode(const std::string& _Filename, const std::string _Text, CorrectionLevel _CorrectionLevel, RGBA _Bit1Color, RGBA _Bit0Color, RGBA _FontColor, int _PixelScale = 20, uint8_t _MaskPatern = 0);

private:

	/**
	* @brief Génère une matrice de code QR simple avec les données encodées et les motifs fonctionnels.
	* @param _Text Texte à encoder.
	* @param _QRVersion Version du code QR.
	* @param _CorrectionLevel Niveau de correction d'erreur.
	* @param _MaskPatern Masque de données à appliquer.
	* @return Structure `QRCodeData` contenant la matrice du code QR et les données encodées.
	*
	* @details
	* ### Étapes de la génération :
	* 1. **Initialisation de la structure `QRCodeData`** :
	*    - Crée une instance vide `outQRData`.
	*
	* 2. **Calcul de la taille de la matrice** :
	*    - `matrixSize = 21 + (_QRVersion - 1) * 4` (taille en modules pour la version donnée).
	*
	* 3. **Génération des motifs fonctionnels** :
	*    - Appelle `QRCodeFunctionPatterns::Generate` pour dessiner les repères de position, les motifs de synchronisation, les motifs d'alignement, et les informations de format/version.
	*
	* 4. **Encodage des données** :
	*    - Appelle `QRCodeDataEncoding::EncodeTextToDataCodewords` pour convertir `_Text` en une séquence de codewords (`outQRData.Bits`).
	*
	* 5. **Correction d'erreur et entrelacement** :
	*    - Appelle `QRCodeReedSolomonCorrector::ErrorCorrectionAndInterleave` pour ajouter des octets de correction et entrelacer les données.
	*
	* 6. **Placement des codewords** :
	*    - Appelle `QRCodeBitPlacement::PlaceCodewords` pour placer les codewords entrelacés dans la matrice `outQRData.MatrixQR`.
	*
	* 7. **Retourne la structure `QRCodeData`** :
	*    - Contient la matrice du code QR (`MatrixQR`) et les données encodées (`Bits`).
	*/
	static QRCodeData GenerateSimpleQR(const std::string _Text, int _QRVersion, CorrectionLevel _CorrectionLevel, uint8_t _MaskPatern);

	/**
	* @brief Génère les données de pixels (RGBA) pour une image du code QR avec une marge blanche.
	* @param _QRData Structure contenant la matrice du code QR et les couleurs.
	* @param _PixelScale Facteur d'échelle (nombre de pixels par module).
	* @param _MargeSize Taille de la marge en modules (ex: 4 pour une marge standard).
	* @return Structure QRPixelData contenant les dimensions et les données de pixels.
	*
	* @details
	* ### Étapes de la génération :
	* 1. **Calcul des dimensions de l'image** :
	*    - `size = (_QRData.MatrixQR.size() + _MargeSize * 2) * _PixelScale` :
	*      - `_QRData.MatrixQR.size()` : Taille de la matrice du code QR en modules.
	*      - `_MargeSize * 2` : Ajoute une marge des deux côtés (gauche/droite et haut/bas).
	*      - `* _PixelScale` : Convertit les modules en pixels.
	*    - Exemple : Si la matrice fait 21x21 modules, `_MargeSize = 4`, et `_PixelScale = 10`, alors :
	*      - `size = (21 + 4 * 2) * 10 = (21 + 8) * 10 = 29 * 10 = 290` pixels.
	*
	* 2. **Calcul de la taille de la marge en pixels** :
	*    - `pixelMargeSize = _MargeSize * _PixelScale` :
	*      - Convertit la taille de la marge en modules en pixels.
	*    - Exemple : `_MargeSize = 4`, `_PixelScale = 10` → `pixelMargeSize = 40` pixels.
	*
	* 3. **Initialisation des données de pixels** :
	*    - Crée une instance de `QRPixelData` avec :
	*      - Largeur et hauteur = `size`.
	*      - Facteur d'échelle = `_PixelScale`.
	*      - Couleur de fond = `_QRData.FontColor` (généralement blanc).
	*
	* 4. **Remplissage des pixels du code QR** :
	*    - Parcourt chaque pixel de l'image **dans la zone du code QR** (en excluant la marge).
	*    - Pour chaque pixel `(x, y)` :
	*      - **Vérifie si le pixel est dans la zone du code QR** :
	*        - `x` de `pixelMargeSize` à `size - pixelMargeSize - 1`.
	*        - `y` de `pixelMargeSize` à `size - pixelMargeSize - 1`.
	*      - **Calcule les coordonnées dans la matrice du code QR** :
	*        - `qrX = (x / _PixelScale) - _MargeSize` : Convertit `x` en coordonnée de module et soustrait la marge.
	*        - `qrY = (y / _PixelScale) - _MargeSize` : Convertit `y` en coordonnée de module et soustrait la marge.
	*      - **Récupère la valeur du bit** :
	*        - `bit = _QRData.MatrixQR[qrY][qrX]` (0 = blanc, 1 = noir).
	*      - **Calcule l'index du pixel** :
	*        - `idx = y * size + x` : Index en **ordre ligne par ligne** (row-major).
	*      - **Définit la couleur du pixel** :
	*        - Si `bit == 1`, utilise `_QRData.Bit1Color` (couleur des modules noirs).
	*        - Sinon, utilise `_QRData.Bit0Color` (couleur des modules blancs).
	*
	* 5. **Retourne les données de pixels** :
	*    - La marge reste de la couleur de fond (`_QRData.FontColor`), car elle n'est pas modifiée.
	*
	* @note
	* - **Marge blanche** : La marge est automatiquement de la couleur de fond (`_QRData.FontColor`), car `outQRPixelData` est initialisée avec cette couleur.
	* - **Efficacité** : Seuls les pixels **dans la zone du code QR** sont modifiés. Les pixels de la marge restent inchangés (blancs).
	*/
	static QRPixelData GenerateQRPixelData(const QRCodeData& _QRData, int _PixelScale, int _MargeSize = 2);

};

