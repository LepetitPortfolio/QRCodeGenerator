#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

class QRCodeBitPlacement
{
public:
	/**
	* @brief Place les codewords entrelacés dans la matrice du code QR.
	* @param _InterleavedCodewords Vecteur des codewords entrelacés (issus de l'encodage et de la correction d'erreur).
	* @param _Version Version du code QR (de 1 à 40).
	* @param _Matrix Matrice 2D où placer les codewords (sera modifiée).
	*
	* @details
	* - Crée un **masque de fonctions** (`functionMask`) pour la version donnée en appelant `MakeFunctionMask(_Version)`.
	* - Appelle la version complète de `PlaceCodewords` avec le masque de fonctions.
	*
	* @note
	* Cette version est une **surcharge** pour simplifier l'appel lorsque le masque de fonctions n'est pas déjà disponible.
	*/
	static void PlaceCodewords(const std::vector<uint8_t>& _InterleavedCodewords, int _Version, std::vector<std::vector<int>>& _Matrix);

	/**
    * @brief Place les codewords entrelacés dans la matrice du code QR, en évitant les zones réservées.
    * @param _InterleavedCodewords Vecteur des codewords entrelacés.
    * @param _Version Version du code QR (de 1 à 40).
    * @param _Matrix Matrice 2D où placer les codewords (sera modifiée).
    * @param _Function Masque de fonctions indiquant les zones réservées (true = réservée, false = disponible).
    *
    * @throws std::invalid_argument Si la version est invalide, ou si la taille de la matrice ou du masque est incorrecte.
    * @throws std::logic_error Si toutes les données n'ont pas été placées dans la matrice.
    *
    * @details
    * ### Étapes principales :
    * 1. **Validation des paramètres** :
    *    - Vérifie que `_Version` est entre 1 et 40.
    *    - Vérifie que `_Matrix` et `_Function` sont des matrices carrées de taille `size = 17 + 4 * _Version`.
    *
    * 2. **Vérification de la taille des codewords** :
    *    - Vérifie que `_InterleavedCodewords.size()` correspond au nombre attendu de codewords pour la version (via `GetRawCodewordCount`).
    *
    * 3. **Placement des codewords** :
    *    - Parcourt la matrice en **colonnes**, de droite à gauche, en sautant la colonne 6 (motif de synchronisation vertical).
    *    - Pour chaque colonne, parcourt les lignes en **zigzag** (alternance entre le haut et le bas).
    *    - Pour chaque position (x, y) :
    *      - Vérifie si la position est réservée (`_Function[y][x] == true`). Si oui, passe à la suivante.
    *      - Sinon, extrait le bit suivant de `_InterleavedCodewords` et le place dans `_Matrix[y][x]`.
    *
    * 4. **Extraction des bits** :
    *    - Les codewords sont lus **bit par bit**, du bit le plus significatif (MSB) au bit le moins significatif (LSB).
    *    - `bitIndex` suit la position du bit courant dans `_InterleavedCodewords`.
    *    - Pour chaque bit, utilise `(byte >> (7 - (bitIndex % 8))) & 1U` pour extraire le bit correspondant.
    *
    * 5. **Vérification finale** :
    *    - Vérifie que tous les bits des codewords ont été placés (`bitIndex == _InterleavedCodewords.size() * 8`).
    *
    * @note
    * - Le **chemin de placement** est conçu pour répartir les données de manière uniforme dans la matrice, en évitant les zones réservées.
    * - La **colonne 6** est ignorée car elle contient le motif de synchronisation vertical.
    * - Le **zigzag** (alternance entre le haut et le bas) permet de répartir les données de manière à éviter les motifs réguliers qui pourraient interférer avec la lecture du code.
    */
	static void PlaceCodewords(const std::vector<uint8_t>& _InterleavedCodewords, int _Version, std::vector<std::vector<int>>& _Matrix, const std::vector<std::vector<bool>>& _Function);

private:
    /**
    * @brief Crée un masque de fonctions pour une version de code QR donnée.
    * @param _Version Version du code QR (de 1 à 40).
    * @return Matrice 2D de booléens où true = zone réservée, false = zone disponible.
    *
    * @throws std::invalid_argument Si la version est invalide.
    *
    * @details
    * ### Zones réservées dans un code QR :
    * 1. **Repères de position** (Finder Patterns) :
    *    - Trois carrés 9x9 (ou 7x7 pour les versions >= 7) situés aux coins de la matrice.
    *    - Ces repères permettent au lecteur de détecter et d'orienter le code QR.
    *    - Chaque repère est entouré d'une **bordure blanche** (1 module de large).
    *
    * 2. **Motifs de synchronisation** (Timing Patterns) :
    *    - Lignes alternées noires et blanches entre les repères de position.
    *    - Ces motifs aident à déterminer la taille et la position des modules.
    *
    * 3. **Motifs d'alignement** (Alignment Patterns) :
    *    - Présents pour les versions >= 2.
    *    - Petits carrés (5x5) répartis dans la matrice pour aider à corriger les distorsions.
    *    - Leur nombre et leur position dépendent de la version.
    *
    * 4. **Information de format** (Format Information) :
    *    - Deux copies de 15 bits (plus 1 bit sombre fixe) qui stockent le niveau de correction et le masque utilisé.
    *    - Situées autour des repères de position.
    *
    * 5. **Information de version** (Version Information) :
    *    - Présente pour les versions >= 7.
    *    - Deux zones de 3x6 modules qui stockent la version du code QR.
    *
    * ### Étapes de création du masque :
    * 1. **Initialisation** :
    *    - Crée une matrice `function` de taille `size x size` (où `size = 17 + 4 * _Version`), initialisée à `false`.
    *
    * 2. **Ajout des repères de position** :
    *    - Réserve trois carrés 9x9 aux coins de la matrice (en haut à gauche, en haut à droite, en bas à gauche).
    *
    * 3. **Ajout des motifs de synchronisation** :
    *    - Réserve les lignes et colonnes alternées entre les repères de position (ligne 6 et colonne 6).
    *
    * 4. **Ajout des motifs d'alignement** (pour les versions >= 2) :
    *    - Calcule les centres des motifs d'alignement en fonction de la version.
    *    - Réserve un carré 5x5 autour de chaque centre.
    *
    * 5. **Ajout de l'information de format** :
    *    - Réserve les 15 bits d'information de format (plus 1 bit sombre fixe) autour des repères de position.
    *
    * 6. **Ajout de l'information de version** (pour les versions >= 7) :
    *    - Réserve deux zones de 3x6 modules pour stocker la version.
    */
	static  std::vector<std::vector<bool>> MakeFunctionMask(int _Version);

    /**
    * @brief Réserve un rectangle dans le masque de fonctions.
    * @param _Function Masque de fonctions à modifier.
    * @param _X Coordonnée X du coin supérieur gauche du rectangle.
    * @param _Y Coordonnée Y du coin supérieur gauche du rectangle.
    * @param _Width Largeur du rectangle.
    * @param _Height Hauteur du rectangle.
    * @param _VersionSize Taille de la matrice (pour vérifier les limites).
    *
    * @details
    * - Parcourt chaque cellule du rectangle défini par (`_X`, `_Y`, `_Width`, `_Height`).
    * - Pour chaque cellule, appelle `Reserve(_Function, _X + dx, _Y + dy, _VersionSize)`.
    */
	static void ReserveRect(std::vector<std::vector<bool>>& _Function, int _X, int _Y, int _Width, int _Height, int _VersionSize);

    /**
    * @brief Réserve une cellule individuelle dans le masque de fonctions.
    * @param _Function Masque de fonctions à modifier.
    * @param _X Coordonnée X de la cellule.
    * @param _Y Coordonnée Y de la cellule.
    * @param _VersionSize Taille de la matrice (pour vérifier les limites).
    *
    * @details
    * - Vérifie que (`_X`, `_Y`) est dans les limites de la matrice (`0 <= _X, _Y < _VersionSize`).
    * - Si c'est le cas, définit `_Function[_Y][_X] = true` pour marquer la cellule comme réservée.
    */
	static void Reserve(std::vector<std::vector<bool>>& _Function, int _X, int _Y, int _VersionSize);

};

