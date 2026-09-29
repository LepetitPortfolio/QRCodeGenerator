#pragma once
#include "QRCodeData.h"

#include <string>

class QRCodeGenerator
{
public:

	/**
	* @brief Génère un code QR avec des couleurs par défaut et l'enregistre dans un fichier PNG.
	* @param _Text Texte à encoder dans le code QR.
	* @param _CorrectionLevel Niveau de correction d'erreur (L, M, Q, ou H).
	* @param _PixelScale Facteur d'échelle pour l'image (nombre de pixels par module du code QR).
	* @param _MaskPatern Masque de données à appliquer (de 0 à 7).
	*
	* @details
	* - Appelle la version complète de `GenerateQRCode` avec des couleurs par défaut :
	*   - `_Bit1Color = { 20, 20, 20, 255 }` (noir).
	*   - `_Bit0Color = { 240, 240, 240, 255 }` (gris clair).
	*   - `_FontColor = { 240, 240, 240, 255 }` (gris clair, pour le fond).
	* - Le fichier PNG résultant est nommé **"QRCode.png"**.
	*/
	static void GenerateQRCode(const std::string _Text, CorrectionLevel _CorrectionLevel, int _PixelScale = 20, uint8_t _MaskPatern = 0);

	/**
	* @brief Génère un code QR avec des couleurs personnalisées et l'enregistre dans un fichier PNG.
	* @param _Text Texte à encoder dans le code QR.
	* @param _CorrectionLevel Niveau de correction d'erreur (L, M, Q, ou H).
	* @param _Bit1Color Couleur des modules noirs (1) au format RGBA.
	* @param _Bit0Color Couleur des modules blancs (0) au format RGBA.
	* @param _FontColor Couleur de fond du code QR au format RGBA.
	* @param _PixelScale Facteur d'échelle pour l'image (nombre de pixels par module du code QR).
	* @param _MaskPatern Masque de données à appliquer (de 0 à 7).
	*
	* @details
	* ### Étapes de la génération :
	* 1. **Sélection de la version** :
	*    - Utilise `QRCodeDataEncoding::SelectVersionForText` pour déterminer la version minimale nécessaire pour encoder `_Text` avec `_CorrectionLevel`.
	*
	* 2. **Génération du code QR simple** :
	*    - Appelle `GenerateSimpleQR` pour créer une matrice de code QR avec les données encodées, la correction d'erreur, et les motifs fonctionnels.
	*
	* 3. **Personnalisation des couleurs** :
	*    - Définit les couleurs pour les modules noirs (`Bit1Color`), blancs (`Bit0Color`), et le fond (`FontColor`) dans `data`.
	*
	* 4. **Génération des données de pixels** :
	*    - Appelle `GenerateQRPixelData` pour convertir la matrice du code QR en une structure `QRPixelData` (contenant les pixels de l'image).
	*
	* 5. **Génération du fichier PNG** :
	*    - Appelle `QRCodePNGFile::GeneratePNGFile` pour créer un fichier PNG nommé **"QRCode.png"** avec les données de pixels.
	*/
	static void GenerateQRCode(const std::string _Text, CorrectionLevel _CorrectionLevel, RGBA _Bit1Color, RGBA _Bit0Color, RGBA _FontColor, int _PixelScale = 20, uint8_t _MaskPatern = 0);

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
	* @brief Convertit la matrice du code QR en données de pixels pour une image.
	* @param _QRData Structure contenant la matrice du code QR et les couleurs.
	* @param _PixelScale Facteur d'échelle (nombre de pixels par module du code QR).
	* @return Structure `QRPixelData` contenant les dimensions et les données de pixels de l'image.
	*
	* @details
	* ### Étapes de la conversion :
	* 1. **Calcul des dimensions de l'image** :
	*    - `size = _QRData.MatrixQR.size() * _PixelScale` (taille en pixels de l'image carrée).
	*
	* 2. **Initialisation de `QRPixelData`** :
	*    - Crée une instance `outQRPixelData` avec les dimensions `size x size`, le facteur d'échelle `_PixelScale`, et la couleur de fond `_QRData.FontColor`.
	*
	* 3. **Remplissage des données de pixels** :
	*    - Parcourt chaque pixel de l'image (coordonnées `(x, y)`).
	*    - Calcule les coordonnées correspondantes dans la matrice du code QR :
	*      - `qrX = x / _PixelScale` (colonne dans la matrice).
	*      - `qrY = y / _PixelScale` (ligne dans la matrice).
	*    - Récupère la valeur du bit dans la matrice : `bit = _QRData.MatrixQR[qrY][qrX]`.
	*    - Définit la couleur du pixel :
	*      - Si `bit == 1`, utilise `_QRData.Bit1Color` (couleur des modules noirs).
	*      - Sinon, utilise `_QRData.Bit0Color` (couleur des modules blancs).
	*    - Stocke la couleur dans `outQRPixelData.PixelData` à l'index `idx = y * size + x`.
	*
	* 4. **Retourne `QRPixelData`** :
	*    - Contient les dimensions (`Width`, `Height`) et les données de pixels (`PixelData`) pour l'image.
	*/
	static QRPixelData GenerateQRPixelData(const QRCodeData& _QRData, int _PixelScale);

};

