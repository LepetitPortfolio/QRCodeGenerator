#pragma once
#include "QRCodeData.h"

class QRCodeFunctionPatterns
{
public:

    /**
    * @brief Génère tous les motifs fonctionnels pour un code QR.
    * @param _QRCodeData Structure contenant la matrice du code QR et les zones réservées.
    * @param _Version Version du code QR (de 1 à 40).
    * @param _CorrectionLevel Niveau de correction d'erreur (L, M, Q, ou H).
    * @param _MaskPattern Masque de données à appliquer (de 0 à 7).
    *
    * @throws std::invalid_argument Si la version est invalide.
    *
    * @details
    * ### Étapes principales :
    * 1. **Validation de la version** :
    *    - Vérifie que `_Version` est entre 1 et 40.
    *
    * 2. **Initialisation de la matrice** :
    *    - Calcule la taille de la matrice avec `size = 17 + 4 * _Version`.
    *    - Initialise `_QRCodeData.MatrixQR` et `_QRCodeData.Reserved` comme des matrices carrées de taille `size x size`, remplies de `0` (blanc) et `false` (non réservé).
    *
    * 3. **Motifs de synchronisation** :
    *    - Dessine les lignes de synchronisation (ligne 6 et colonne 6) en alternant les modules noirs et blancs (`i % 2 == 0`).
    *    - Les repères de position (dessinés plus tard) **recouvrent** ces lignes aux intersections.
    *
    * 4. **Repères de position (Finder Patterns)** :
    *    - Dessine trois repères de position aux coins :
    *      - Haut-gauche : `(3, 3)`
    *      - Haut-droite : `(size - 4, 3)`
    *      - Bas-gauche : `(3, size - 4)`
    *
    * 5. **Motifs d'alignement (Alignment Patterns)** :
    *    - Calcule les centres des motifs d'alignement avec `GetAlignmentCenters(_Version, size)`.
    *    - Dessine un motif d'alignement à chaque centre, sauf aux coins déjà occupés par les repères de position.
    *
    * 6. **Information de format (Format Information)** :
    *    - Dessine les bits d'information de format avec `DrawFormat(_QRCodeData, _CorrectionLevel, _MaskPattern)`.
    *
    * 7. **Information de version (Version Information)** :
    *    - Dessine les bits d'information de version avec `DrawVersion(_QRCodeData, _Version)` (uniquement pour les versions >= 7).
    */
    static void Generate(QRCodeData& _QRCodeData, int _Version, CorrectionLevel _CorrectionLevel, int _MaskPattern);

private:

    /**
    * @brief Définit la valeur d'un module dans la matrice du code QR et le marque comme réservé.
    * @param _QRCodeData Structure contenant la matrice et les zones réservées.
    * @param _X Coordonnée X du module.
    * @param _Y Coordonnée Y du module.
    * @param _Dark Si true, le module est noir (1) ; sinon, il est blanc (0).
    *
    * @details
    * - Vérifie que (`_X`, `_Y`) est dans les limites de la matrice.
    * - Si c'est le cas :
    *   - Définit `_QRCodeData.MatrixQR[_Y][_X]` à `1` (noir) ou `0` (blanc) selon `_Dark`.
    *   - Marque `_QRCodeData.Reserved[_Y][_X]` à `true` pour indiquer que cette position est réservée.
    */
    static void Set(QRCodeData& _QRCodeData, int _X, int _Y, bool _Dark);

    /**
    * @brief Dessine un repère de position (Finder Pattern) à une position donnée.
    * @param _QRCodeData Structure contenant la matrice et les zones réservées.
    * @param _CenterX Coordonnée X du centre du repère.
    * @param _CenterY Coordonnée Y du centre du repère.
    *
    * @details
    * ### Structure d'un repère de position :
    * - **Taille** : 7x7 modules.
    * - **Carré noir extérieur** : Modules où `distance <= 4` (distance de Chebyshev).
    * - **Bordure blanche** : Modules où `distance == 3`.
    * - **Carré noir central** : Modules où `distance <= 2`.
    * - **Centre blanc** : Module où `distance == 0` (non dessiné, car `distance != 2 && distance != 4`).
    *
    * ### Algorithme :
    * - Parcourt un carré 7x7 centré sur (`_CenterX`, `_CenterY`).
    * - Pour chaque module, calcule la **distance de Chebyshev** (`max(|dx|, |dy|)`).
    * - Si `distance != 2 && distance != 4`, le module est noir (`true`).
    *   - Cela signifie que les modules où `distance == 0, 1, 3` sont blancs, et ceux où `distance == 2, 4` sont noirs.
    *   - **Correction** : La condition semble inversée. Pour un repère de position standard, les modules où `distance == 0, 4` sont noirs, et ceux où `distance == 2` sont blancs.
    *     - **Condition correcte** : `Set(_QRCodeData, _CenterX + dx, _CenterY + dy, distance == 0 || distance == 4 || distance == 2);`
    *     - **Explication** :
    *       - `distance == 0` : Centre (noir).
    *       - `distance == 2` : Bordure intérieure (blanc).
    *       - `distance == 4` : Bordure extérieure (noir).
    *
    * @note
    * - **⚠️ Bug** : La condition `distance != 2 && distance != 4` semble incorrecte. Elle devrait être `distance == 0 || distance == 2 || distance == 4` pour un repère de position standard.
    */
	static void DrawFinder(QRCodeData& _QRCodeData, int centerX, int centerY);

    /**
    * @brief Calcule les centres des motifs d'alignement pour une version de code QR donnée.
    * @param _Version Version du code QR.
    * @param _Size Taille de la matrice du code QR.
    * @return Vecteur des coordonnées X (ou Y) des centres des motifs d'alignement.
    *
    * @details
    * ### Algorithme :
    * - Pour la **version 1**, retourne un vecteur vide (pas de motifs d'alignement).
    * - Pour les versions >= 2 :
    *   - Calcule le **nombre de motifs d'alignement** : `count = _Version / 7 + 2`.
    *   - Calcule l'**espacement entre les motifs** : `step = (_Version * 8 + count * 3 + 5) / (count * 4 - 4) * 2`.
    *   - Initialise un vecteur `centers` de taille `count`.
    *   - Le premier centre est toujours à `6`.
    *   - Les autres centres sont calculés en partant de `_Size - 7` et en reculant de `step` à chaque fois.
    *
    * @note
    * - Les centres sont **symétriques** en X et Y, donc le même vecteur peut être utilisé pour les deux axes.
    * - Les motifs d'alignement sont placés aux intersections des lignes et colonnes définies par `centers`.
    */
    static std::vector<int> GetAlignmentCenters(int _Version, int _Size);

    /**
    * @brief Dessine un motif d'alignement (Alignment Pattern) à une position donnée.
    * @param _QRCodeData Structure contenant la matrice et les zones réservées.
    * @param _CenterX Coordonnée X du centre du motif.
    * @param _CenterY Coordonnée Y du centre du motif.
    *
    * @details
    * ### Structure d'un motif d'alignement :
    * - **Taille** : 5x5 modules.
    * - **Carré noir extérieur** : Modules où `distance == 2` (distance de Chebyshev).
    * - **Bordure blanche** : Modules où `distance == 1`.
    * - **Centre noir** : Module où `distance == 0`.
    *
    * ### Algorithme :
    * - Parcourt un carré 5x5 centré sur (`_CenterX`, `_CenterY`).
    * - Pour chaque module, calcule la **distance de Chebyshev** (`max(|dx|, |dy|)`).
    * - Si `distance != 1`, le module est noir (`true`).
    *   - Cela signifie que les modules où `distance == 0, 2` sont noirs, et ceux où `distance == 1` sont blancs.
    */
    static void DrawAlignment(QRCodeData& _QRCodeData, int _CenterX, int _CenterY);

    /**
    * @brief Dessine les bits d'information de format (Format Information) dans la matrice du code QR.
    * @param _QRCodeData Structure contenant la matrice et les zones réservées.
    * @param _CorrectionLevel Niveau de correction d'erreur (L, M, Q, ou H).
    * @param _MaskPattern Masque de données à appliquer (de 0 à 7).
    *
    * @throws std::invalid_argument Si le niveau de correction ou le masque est invalide.
    *
    * @details
    * ### Encodage de l'information de format :
    * - **Niveau de correction** : Encodé sur 2 bits :
    *   - L (Low) : `01`
    *   - M (Medium) : `00`
    *   - Q (Quartile) : `11`
    *   - H (High) : `10`
    * - **Masque de données** : Encodé sur 3 bits (valeur de 0 à 7).
    * - **Total** : 5 bits pour le niveau de correction et le masque.
    *
    * ### Calcul des bits de format :
    * 1. **Encodage du niveau de correction et du masque** :
    *    - `data = (levelBits[levelIndex] << 3) | _MaskPattern`.
    *    - `levelBits` est un tableau qui mappe le niveau de correction à ses 2 bits : `{1, 0, 3, 2}` pour L, M, Q, H.
    *
    * 2. **Calcul du code de correction (BCH)** :
    *    - Initialise `remainder = data`.
    *    - Effectue 10 itérations de calcul de reste avec le polynôme **0x537** (pour le code BCH(15,5)).
    *    - Le résultat est un code de correction de 10 bits.
    *
    * 3. **Combinaison des bits** :
    *    - `bits = ((data << 10) | remainder) ^ 0x5412`.
    *    - Le XOR avec `0x5412` est une étape de masquage pour éviter les motifs problématiques.
    *
    * 4. **Placement des bits** :
    *    - **Première copie** : Autour du repère supérieur gauche (colonne 8, lignes 0-8 et 7-8).
    *    - **Deuxième copie** : Autour des repères inférieur gauche et supérieur droit (ligne 8, colonnes 0-7 et `size-15` à `size-8`).
    *    - **Module sombre fixe** : À `(8, size - 8)`.
    *
    * @note
    * - Les bits d'information de format sont **dupliqués** pour une redondance accrue.
    * - Le **module sombre fixe** est toujours noir, indépendamment des autres bits.
    */
    static void DrawFormat(QRCodeData& _QRCodeData, CorrectionLevel _CorrectionLevel, int _MaskPattern);

    /**
    * @brief Dessine les bits d'information de version (Version Information) dans la matrice du code QR.
    * @param _QRCodeData Structure contenant la matrice et les zones réservées.
    * @param _Version Version du code QR (de 7 à 40).
    *
    * @details
    * - **Ne fait rien** si `_Version < 7` (les versions 1-6 n'ont pas d'information de version).
    *
    * ### Encodage de l'information de version :
    * - **Version** : Encodée sur 6 bits (valeur de 7 à 40).
    * - **Code de correction (BCH)** : 12 bits calculés à partir de la version.
    *
    * ### Calcul des bits de version :
    * 1. **Encodage de la version** :
    *    - `remainder = _Version`.
    * 2. **Calcul du code de correction (BCH)** :
    *    - Effectue 12 itérations de calcul de reste avec le polynôme **0x1F25** (pour le code BCH(18,6)).
    * 3. **Combinaison des bits** :
    *    - `bits = (_Version << 12) | remainder`.
    *
    * ### Placement des bits :
    * - Les 18 bits sont placés dans deux zones de **3x6 modules** :
    *   - Une zone dans le **coin haut-droite** (lignes `size-11` à `size-9`, colonnes 0-2).
    *   - Une zone dans le **coin bas-gauche** (lignes 0-2, colonnes `size-11` à `size-9`).
    * - Les bits sont placés en **zigzag** :
    *   - Pour chaque bit `i` (0 à 17) :
    *     - `a = size - 11 + i % 3` (colonne ou ligne).
    *     - `b = i / 3` (ligne ou colonne).
    *     - Le bit est placé à `(a, b)` et `(b, a)` pour une symétrie.
    */
    static void DrawVersion(QRCodeData& _QRCodeData, int _Version);
};

