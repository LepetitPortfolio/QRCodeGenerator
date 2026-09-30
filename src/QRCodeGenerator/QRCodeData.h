
#pragma once
#include "QRCodeColor.h"

#include <array>
#include <cstdint>
#include <vector>

/**
 * @enum CorrectionLevel
 * @brief Définit les niveaux de correction d'erreur pour un code QR.
 *
 * Les niveaux de correction déterminent la quantité de données de correction (ECC) ajoutée au code QR.
 * Plus le niveau est élevé, plus le code QR peut résister à des dommages, mais moins il peut stocker de données.
 *
 * @var CorrectionLevel::L
 * Niveau L (Low) : ~7% de correction.
 * - Permet de corriger jusqu'à 7% de modules endommagés.
 * - Capacité de stockage : la plus élevée.
 *
 * @var CorrectionLevel::M
 * Niveau M (Medium) : ~15% de correction.
 * - Permet de corriger jusqu'à 15% de modules endommagés.
 * - Équilibre entre capacité et robustesse.
 *
 * @var CorrectionLevel::Q
 * Niveau Q (Quartile) : ~25% de correction.
 * - Permet de corriger jusqu'à 25% de modules endommagés.
 * - Capacité de stockage : moyenne.
 *
 * @var CorrectionLevel::H
 * Niveau H (High) : ~30% de correction.
 * - Permet de corriger jusqu'à 30% de modules endommagés.
 * - Capacité de stockage : la plus faible.
 *
 * @note
 * Les valeurs associées (L=1, M=0, Q=3, H=2) sont utilisées pour indexer les tableaux de constantes
 * (ex: `DataCodewords`, `EccCodewordsPerBlock`, `BlockCount`).
 * Ces valeurs semblent arbitraires, mais elles sont cohérentes avec l'ordre des niveaux dans la norme QR.
 */
enum class CorrectionLevel
{
	L = 1,
	M = 0,
	Q = 3,
	H = 2
};


/**
 * @struct QRCodeData
 * @brief Structure contenant les données d'un code QR en cours de génération.
 *
 * Cette structure est utilisée pour stocker :
 * - La matrice du code QR (modules noirs et blancs).
 * - Les zones réservées (motifs fonctionnels).
 * - Les données encodées (codewords).
 * - Les couleurs pour le rendu final.
 */
struct QRCodeData
{
public:

	/**
	* @brief Matrice du code QR.
	* - `MatrixQR[y][x]` = 0 → module blanc.
	* - `MatrixQR[y][x]` = 1 → module noir.
	* - La taille de la matrice dépend de la version (ex: 21x21 pour la version 1, 25x25 pour la version 2, etc.).
	*/
    std::vector<std::vector<int>> MatrixQR;

	/**
	 * @brief Masque des zones réservées.
	 * - `Reserved[y][x]` = true → la cellule (y, x) est réservée (ex: repère de position, motif de synchronisation).
	 * - `Reserved[y][x]` = false → la cellule peut contenir des données.
	 * - Utilisé pour éviter de placer des données dans les zones fonctionnelles.
	 */
	std::vector<std::vector<bool>> Reserved;

	/**
	* @brief Données encodées (codewords).
	* - Contient les **codewords** (octets) représentant le texte encodé + les octets de correction (ECC).
	* - Ces codewords sont ensuite placés dans la matrice via `QRCodeBitPlacement::PlaceCodewords`.
	*/
    std::vector<uint8_t> Bits;

	/**
	 * @brief Couleur des modules noirs (1).
	 * - Utilisée pour le rendu final du code QR.
	 * - Format RGBA : { Rouge, Vert, Bleu, Alpha }.
	 */
    RGBA Bit1Color;

	/**
	 * @brief Couleur des modules blancs (0).
	 * - Utilisée pour le rendu final du code QR.
	 * - Format RGBA : { Rouge, Vert, Bleu, Alpha }.
	 */
    RGBA Bit0Color;

	/**
	* @brief Couleur de la police (fond).
	* - Utilisée comme couleur de fond pour l'image finale.
	* - Peut aussi être utilisée pour dessiner du texte ou des logos dans la marge.
	* - Format RGBA : { Rouge, Vert, Bleu, Alpha }.
	*/
    RGBA FontColor;
};

/**
 * @struct QRPixelData
 * @brief Structure contenant les données de pixels pour l'image finale du code QR.
 *
 * Cette structure est utilisée pour :
 * - Stocker les dimensions de l'image (largeur, hauteur).
 * - Stocker les données de pixels au format RGBA.
 * - Générer le fichier PNG final.
 */
struct QRPixelData
{
public:

	/**
	* @brief Constructeur par défaut.
	* - Initialise les membres avec des valeurs par défaut.
	*/
	QRPixelData() = default;

	/**
	 * @brief Constructeur avec paramètres.
	 * @param _Width Largeur de l'image en pixels.
	 * @param _Height Hauteur de l'image en pixels.
	 * @param _PixelScale Facteur d'échelle (nombre de pixels par module).
	 * @param _DefaultFontColor Couleur de fond par défaut (RGBA).
	 *
	 * @details
	 * - Initialise `Width`, `Height`, et `PixelScale`.
	 * - Initialise `PixelData` comme un vecteur de `RGBA` de taille `_Width * _Height`, rempli avec `_DefaultFontColor`.
	 */
	QRPixelData(int _Width, int _Height, int _PixelScale, RGBA _DefaultFontColor = RGBA{}) 
	{
		Width = _Width;
		Height = _Height;
		PixelScale = _PixelScale;
		PixelData = std::vector<RGBA>(_Width * _Height, _DefaultFontColor);
	}

	/**
	 * @brief Données de pixels au format RGBA.
	 * - Chaque pixel est représenté par 4 octets : { Rouge, Vert, Bleu, Alpha }.
	 * - Les pixels sont stockés en **ordre ligne par ligne** (row-major).
	 */
	std::vector<RGBA> PixelData;

	/**
	* @brief Facteur d'échelle (nombre de pixels par module).
	* - Exemple : `PixelScale = 10` → chaque module du code QR est représenté par 10x10 pixels.
	*/
	int PixelScale = 1;
	
	/**
	 * @brief Largeur de l'image en pixels.
	 */
	int Width = 0;

	/**
	 * @brief Hauteur de l'image en pixels.
	 */
	int Height = 0;
};


/**
 * @brief Nombre de codewords disponibles pour chaque version et niveau de correction.
 *
 * @details
 * - `DataCodewords[version - 1][level]` donne le nombre de codewords pour la version `version` et le niveau de correction `level`.
 * - Les niveaux de correction sont indexés comme suit :
 *   - 0 : M (Medium)
 *   - 1 : L (Low)
 *   - 2 : H (High)
 *   - 3 : Q (Quartile)
 * - Exemple :
 *   - `DataCodewords[0][1]` = 19 → Version 1, niveau L : 19 codewords.
 *   - `DataCodewords[39][3]` = 1666 → Version 40, niveau Q : 1666 codewords.
 *
 * @note
 * - Les valeurs sont basées sur la **norme ISO/IEC 18004**.
 * - Le nombre de codewords inclut à la fois les **données** et les **octets de correction (ECC)**.
 */
const std::array<std::array<uint16_t, 4>, 40> DataCodewords =
{ {
	{{19, 16, 13, 9}},     {{34, 28, 22, 16}},    {{55, 44, 34, 26}},
	{{80, 64, 48, 36}},    {{108, 86, 62, 46}},   {{136, 108, 76, 60}},
	{{156, 124, 88, 66}},  {{194, 154, 110, 86}}, {{232, 182, 132, 100}},
	{{274, 216, 154, 122}},{{324, 254, 180, 140}},{{370, 290, 206, 158}},
	{{428, 334, 244, 180}},{{461, 365, 261, 197}},{{523, 415, 295, 223}},
	{{589, 453, 325, 253}},{{647, 507, 367, 283}},{{721, 563, 397, 313}},
	{{795, 627, 445, 341}},{{861, 669, 485, 385}},{{932, 714, 512, 406}},
	{{1006, 782, 568, 442}},{{1094, 860, 614, 464}},{{1174, 914, 664, 514}},
	{{1276, 1000, 718, 538}},{{1370, 1062, 754, 596}},{{1468, 1128, 808, 628}},
	{{1531, 1193, 871, 661}},{{1631, 1267, 911, 701}},{{1735, 1373, 985, 745}},
	{{1843, 1455, 1033, 793}},{{1955, 1541, 1115, 845}},{{2071, 1631, 1171, 901}},
	{{2191, 1725, 1231, 961}},{{2306, 1812, 1286, 986}},{{2434, 1914, 1354, 1054}},
	{{2566, 1992, 1426, 1096}},{{2702, 2102, 1502, 1142}},{{2812, 2216, 1582, 1222}},
	{{2956, 2334, 1666, 1276}}
} };


/**
 * @brief Nombre d'octets de correction (ECC) par bloc pour chaque version et niveau de correction.
 *
 * @details
 * - `EccCodewordsPerBlock[level][version - 1]` donne le nombre d'octets ECC par bloc pour la version `version` et le niveau de correction `level`.
 * - Les niveaux de correction sont indexés comme suit :
 *   - 0 : M (Medium)
 *   - 1 : L (Low)
 *   - 2 : Q (Quartile)
 *   - 3 : H (High)
 * - Exemple :
 *   - `EccCodewordsPerBlock[0][0]` = 7 → Version 1, niveau M : 7 octets ECC par bloc.
 *   - `EccCodewordsPerBlock[3][39]` = 30 → Version 40, niveau H : 30 octets ECC par bloc.
 *
 * @note
 * - Les valeurs sont basées sur la **norme ISO/IEC 18004**.
 * - Le nombre d'octets ECC dépend de la **version** et du **niveau de correction**.
 */
const std::array<std::array<uint8_t, 40>, 4> EccCodewordsPerBlock = 
{ {
	{{7,10,15,20,26,18,20,24,30,18,20,24,26,30,22,24,28,30,28,28,28,28,30,30,26,28,30,30,30,30,30,30,30,30,30,30,30,30,30,30}},
	{{10,16,26,18,24,16,18,22,22,26,30,22,22,24,24,28,28,26,26,26,26,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28}},
	{{13,22,18,26,18,24,18,22,20,24,28,26,24,20,30,24,28,28,26,30,28,30,30,30,30,28,30,30,30,30,30,30,30,30,30,30,30,30,30,30}},
	{{17,28,22,16,22,28,26,26,24,28,24,28,22,24,24,30,28,28,26,28,30,24,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30}}
} };

/**
 * @brief Nombre de blocs pour chaque version et niveau de correction.
 *
 * @details
 * - `BlockCount[level][version - 1]` donne le nombre de blocs pour la version `version` et le niveau de correction `level`.
 * - Les niveaux de correction sont indexés comme suit :
 *   - 0 : M (Medium)
 *   - 1 : L (Low)
 *   - 2 : Q (Quartile)
 *   - 3 : H (High)
 * - Exemple :
 *   - `BlockCount[0][0]` = 1 → Version 1, niveau M : 1 bloc.
 *   - `BlockCount[3][39]` = 81 → Version 40, niveau H : 81 blocs.
 *
 * @note
 * - Les valeurs sont basées sur la **norme ISO/IEC 18004**.
 * - Le nombre de blocs dépend de la **version** et du **niveau de correction**.
 * - Plus la version est élevée, plus le nombre de blocs est grand.
 */
const std::array<std::array<uint8_t, 40>, 4> BlockCount = 
{ {
	{{1,1,1,1,1,2,2,2,2,4,4,4,4,4,6,6,6,6,7,8,8,9,9,10,12,12,12,13,14,15,16,17,18,19,19,20,21,22,24,25}},
	{{1,1,1,2,2,4,4,4,5,5,5,8,9,9,10,10,11,13,14,16,17,17,18,20,21,23,25,26,28,29,31,33,35,37,38,40,43,45,47,49}},
	{{1,1,2,2,4,4,6,6,8,8,8,10,12,16,12,17,16,18,21,20,23,23,25,27,29,34,34,35,38,40,43,45,48,51,53,56,59,62,65,68}},
	{{1,1,2,4,4,4,5,6,8,8,11,11,16,16,18,16,19,21,25,25,25,34,30,32,35,37,40,42,45,48,51,54,57,60,63,66,70,74,77,81}}
} };