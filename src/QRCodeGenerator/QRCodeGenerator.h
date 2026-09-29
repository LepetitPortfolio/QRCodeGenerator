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
	*
	* @details
	* - Appelle la version complète de `GenerateQRCode` avec des couleurs par défaut :
	*   - `_Bit1Color` : Noir (`{ 20, 20, 20, 255 }`).
	*   - `_Bit0Color` : Blanc (`{ 240, 240, 240, 255 }`).
	*   - `_FontColor` : Blanc (`{ 240, 240, 240, 255 }`).
	* - Cela permet de générer un code QR en **noir et blanc** avec un fond blanc.
	*/
	static void GenerateQRCode(const std::string& _Filename, const std::string _Text, CorrectionLevel _CorrectionLevel, int _PixelScale = 20);

	/**
	* @brief Génère un code QR et l'enregistre dans un fichier PNG avec des couleurs par défaut.
	* @param _Filename Nom du fichier PNG de sortie.
	* @param _Text Texte à encoder dans le code QR.
	* @param _CorrectionLevel Niveau de correction d'erreur (L, M, Q, ou H).
	* @param _PixelScale Facteur d'échelle pour l'image (nombre de pixels par module).
	*
	* @details
	* - Appelle la version complète de `GenerateQRCode` avec des couleurs par défaut :
	*   - `_Bit1Color` : Noir (`{ 20, 20, 20, 255 }`).
	*   - `_Bit0Color` : Blanc (`{ 240, 240, 240, 255 }`).
	*   - `_FontColor` : Blanc (`{ 240, 240, 240, 255 }`).
	* - Cela permet de générer un code QR en **noir et blanc** avec un fond blanc.
	*/
	static void GenerateQRCode(const std::string& _Filename, const std::string _Text, CorrectionLevel _CorrectionLevel, RGBA _Bit1Color, RGBA _Bit0Color, RGBA _FontColor, int _PixelScale = 20);

private:

	/**
	* @brief Génère une matrice de code QR complète (motifs fonctionnels + données + masque).
	* @param _Text Texte à encoder.
	* @param _QRVersion Version du code QR.
	* @param _CorrectionLevel Niveau de correction d'erreur.
	* @return Structure QRCodeData contenant la matrice, les données, et les zones réservées.
	*
	* @throws std::invalid_argument Si le masque sélectionné est invalide.
	*
	* @details
	* ### Étapes de la génération :
	* 1. **Initialisation de la structure** :
	*    - Crée une instance vide de `QRCodeData` (`outQRData`).
	*
	* 2. **Encodage du texte** :
	*    - Appelle `QRCodeDataEncoding::EncodeTextToDataCodewords(_Text, _QRVersion, _CorrectionLevel)` pour convertir le texte en **codewords**.
	*    - Stocke le résultat dans `outQRData.Bits`.
	*
	* 3. **Correction d'erreur et entrelacement** :
	*    - Appelle `QRCodeReedSolomonCorrector::ErrorCorrectionAndInterleave(outQRData.Bits, _QRVersion, _CorrectionLevel)` pour :
	*      - Ajouter des **octets de correction (ECC)**.
	*      - **Entrelacer** les données et les ECC.
	*    - Met à jour `outQRData.Bits` avec le résultat.
	*
	* 4. **Sélection du meilleur masque** :
	*    - Appelle `QRCodeMasking::SelectBestMask(outQRData.Bits, _QRVersion, _CorrectionLevel)` pour choisir le masque qui minimise les motifs problématiques.
	*    - Vérifie que `maskPattern` est valide (entre 0 et 7).
	*
	* 5. **Génération des motifs fonctionnels** :
	*    - Appelle `QRCodeFunctionPatterns::Generate(outQRData, _QRVersion, _CorrectionLevel, maskPattern)` pour dessiner :
	*      - Les **repères de position** (Finder Patterns).
	*      - Les **motifs de synchronisation** (Timing Patterns).
	*      - Les **motifs d'alignement** (Alignment Patterns).
	*      - L'**information de format** (Format Information).
	*      - L'**information de version** (Version Information, si version >= 7).
	*
	* 6. **Placement des codewords** :
	*    - Appelle `QRCodeBitPlacement::PlaceCodewords(outQRData.Bits, _QRVersion, outQRData.MatrixQR, outQRData.Reserved)` pour :
	*      - Placer les **codewords entrelacés** dans la matrice.
	*      - Éviter les **zones réservées** (motifs fonctionnels).
	*
	* 7. **Application du masque** :
	*    - Appelle `QRCodeMasking::ApplyMask(outQRData.MatrixQR, outQRData.Reserved, maskPattern)` pour :
	*      - Inverser certains modules selon le masque sélectionné.
	*      - Éviter les **motifs problématiques** (lignes droites, carrés uniformes, etc.).
	*
	* @note
	* - **Ordre des étapes** : Les étapes sont **séquentielles** et dépendent les unes des autres.
	*   - Exemple : Les motifs fonctionnels doivent être dessinés **avant** le placement des codewords.
	* - **Masque automatique** : `SelectBestMask` choisit automatiquement le meilleur masque parmi les 8 disponibles.
	*/
	static QRCodeData GenerateSimpleQR(const std::string _Text, int _QRVersion, CorrectionLevel _CorrectionLevel);

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

