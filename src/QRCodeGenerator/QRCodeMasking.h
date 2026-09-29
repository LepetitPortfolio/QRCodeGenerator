#pragma once
#include "QRCodeData.h"

#include <vector>

class QRCodeMasking
{
public:
    
    /**
    * @brief Sélectionne le meilleur masque pour un code QR en minimisant le score de pénalité.
    * @param _InterleavedCodewords Vecteur des codewords entrelacés (données + ECC).
    * @param _Version Version du code QR (de 1 à 40).
    * @param _CorrectionLevel Niveau de correction d'erreur (L, M, Q, ou H).
    * @return Masque sélectionné (0 à 7).
    *
    * @details
    * ### Algorithme :
    * 1. **Initialisation** :
    *    - `bestMask = 0` : Masque initial (par défaut).
    *    - `bestScore = -1` : Score initial (invalide).
    *
    * 2. **Parcours des masques (0 à 7)** :
    *    - Pour chaque masque `mask` :
    *      a. **Génération des motifs fonctionnels** :
    *         - Crée une instance `candidate` de `QRCodeData`.
    *         - Appelle `QRCodeFunctionPatterns::Generate(candidate, _Version, _CorrectionLevel, mask)` pour dessiner les motifs fonctionnels (repères, synchronisation, etc.).
    *      b. **Placement des codewords** :
    *         - Appelle `QRCodeBitPlacement::PlaceCodewords(_InterleavedCodewords, _Version, candidate.MatrixQR, candidate.Reserved)` pour placer les codewords dans la matrice.
    *      c. **Application du masque** :
    *         - Appelle `ApplyMask(candidate.MatrixQR, candidate.Reserved, mask)` pour inverser certains modules selon le masque.
    *      d. **Calcul du score de pénalité** :
    *         - Appelle `GetPenaltyScore(candidate.MatrixQR)` pour évaluer la qualité du masque.
    *      e. **Mise à jour du meilleur masque** :
    *         - Si `bestScore < 0` (première itération) ou `score < bestScore`, met à jour `bestMask` et `bestScore`.
    *
    * 3. **Retourne le meilleur masque** :
    *    - Retourne `bestMask` (le masque avec le score de pénalité le plus bas).
    *
    * @note
    * - **Complexité** : Cette fonction est **coûteuse en calculs**, car elle génère et évalue 8 matrices complètes.
    * - **Optimisation possible** : Si les performances sont critiques, tu pourrais optimiser en **réutilisant des parties de la matrice** ou en **parallélisant** les calculs.
    */
    static int SelectBestMask(const std::vector<uint8_t>& _InterleavedCodewords, int _Version, CorrectionLevel _CorrectionLevel);

    /**
    * @brief Applique un masque de données à une matrice de code QR.
    * @param _Matrix Matrice du code QR à masquer (sera modifiée).
    * @param _Reserved Masque des zones réservées (true = réservée, false = modifiable).
    * @param _MaskPattern Masque à appliquer (de 0 à 7).
    *
    * @throws std::invalid_argument Si le masque est invalide ou si les tailles de la matrice et du masque ne correspondent pas.
    *
    * @details
    * ### Étapes de l'application du masque :
    * 1. **Validation du masque** :
    *    - Vérifie que `_MaskPattern` est entre 0 et 7.
    *    - Si non, lève une exception `std::invalid_argument`.
    *
    * 2. **Validation des tailles** :
    *    - Vérifie que `_Matrix` et `_Reserved` ont la même taille (nombre de lignes).
    *    - Vérifie que chaque ligne de `_Matrix` et `_Reserved` a la même taille (matrices carrées).
    *    - Si non, lève une exception `std::invalid_argument`.
    *
    * 3. **Application du masque** :
    *    - Parcourt chaque cellule de la matrice (`x` de 0 à `size-1`, `y` de 0 à `size-1`).
    *    - Pour chaque cellule **non réservée** (`!_Reserved[y][x]`), vérifie si elle doit être inversée avec `ShouldInvert(x, y, _MaskPattern)`.
    *    - Si `ShouldInvert` retourne `true`, inverse le bit de la cellule (`_Matrix[y][x] ^= 1`).
    *
    * @note
    * - **Inversion de bit** : `_Matrix[y][x] ^= 1` bascule entre 0 (blanc) et 1 (noir).
    * - **Zones réservées** : Les zones réservées (repères de position, motifs de synchronisation, etc.) ne sont **jamais masquées**.
    */
    static void ApplyMask(std::vector<std::vector<int>>& _Matrix, const std::vector<std::vector<bool>>& _Reserved, int _MaskPattern);

private:

    /**
    * @brief Applique un masque de données à une matrice de code QR.
    * @param _Matrix Matrice du code QR à masquer (sera modifiée).
    * @param _Reserved Masque des zones réservées (true = réservée, false = modifiable).
    * @param _MaskPattern Masque à appliquer (de 0 à 7).
    *
    * @throws std::invalid_argument Si le masque est invalide ou si les tailles de la matrice et du masque ne correspondent pas.
    *
    * @details
    * ### Étapes de l'application du masque :
    * 1. **Validation du masque** :
    *    - Vérifie que `_MaskPattern` est entre 0 et 7.
    *    - Si non, lève une exception `std::invalid_argument`.
    *
    * 2. **Validation des tailles** :
    *    - Vérifie que `_Matrix` et `_Reserved` ont la même taille (nombre de lignes).
    *    - Vérifie que chaque ligne de `_Matrix` et `_Reserved` a la même taille (matrices carrées).
    *    - Si non, lève une exception `std::invalid_argument`.
    *
    * 3. **Application du masque** :
    *    - Parcourt chaque cellule de la matrice (`x` de 0 à `size-1`, `y` de 0 à `size-1`).
    *    - Pour chaque cellule **non réservée** (`!_Reserved[y][x]`), vérifie si elle doit être inversée avec `ShouldInvert(x, y, _MaskPattern)`.
    *    - Si `ShouldInvert` retourne `true`, inverse le bit de la cellule (`_Matrix[y][x] ^= 1`).
    *
    * @note
    * - **Inversion de bit** : `_Matrix[y][x] ^= 1` bascule entre 0 (blanc) et 1 (noir).
    * - **Zones réservées** : Les zones réservées (repères de position, motifs de synchronisation, etc.) ne sont **jamais masquées**.
    */
    static bool ShouldInvert(int _X, int _Y, int _MaskPattern);

    /**
    * @brief Calcule le score de pénalité d'une matrice de code QR.
    * @param _Matrix Matrice du code QR à évaluer.
    * @return Score de pénalité (plus le score est bas, meilleure est la matrice).
    *
    * @throws std::invalid_argument Si la matrice est vide ou non carrée.
    *
    * @details
    * ### Critères de pénalité (norme ISO/IEC 18004) :
    * Le score de pénalité est la somme de 4 critères :
    * 1. **N1 : Lignes droites** (horizontales ou verticales) de 5 modules ou plus.
    * 2. **N2 : Blocs monochromes 2x2**.
    * 3. **N3 : Motifs similaires aux repères de position** (ex: `1011101` entouré de modules clairs).
    * 4. **N4 : Écart par rapport à un équilibre 50/50** entre modules noirs et blancs.
    *
    * ### Étapes du calcul :
    * 1. **Validation de la matrice** :
    *    - Vérifie que `_Matrix` n'est pas vide.
    *    - Vérifie que chaque ligne de `_Matrix` a la même taille (matrice carrée).
    *
    * 2. **Initialisation du score** :
    *    - `score = 0`.
    *
    * 3. **Définition de la lambda `scoreLine`** :
    *    - Cette lambda calcule les pénalités **N1** et **N3** pour une **ligne** (horizontale ou verticale) de la matrice.
    *    - **Paramètre** : `getBit` est une fonction qui retourne la valeur d'un bit à une position donnée (0 = blanc, 1 = noir).
    *
    * 4. **Calcul des pénalités N1 et N3 pour les lignes horizontales et verticales** :
    *    - Pour chaque **ligne horizontale** (`y` de 0 à `size-1`) :
    *      - Appelle `scoreLine` avec une lambda qui retourne `_Matrix[y][x] != 0`.
    *    - Pour chaque **colonne verticale** (`x` de 0 à `size-1`) :
    *      - Appelle `scoreLine` avec une lambda qui retourne `_Matrix[y][x] != 0`.
    *
    * 5. **Calcul de la pénalité N2 (blocs 2x2 monochromes)** :
    *    - Parcourt chaque **bloc 2x2** de la matrice.
    *    - Si les 4 modules du bloc sont de la même couleur, ajoute **3 points** au score.
    *
    * 6. **Calcul de la pénalité N4 (équilibre 50/50)** :
    *    - Compte le nombre total de **modules noirs** (`dark`).
    *    - Calcule l'**écart par rapport à 50%** :
    *      - `deviation = |20 * dark - 10 * total| / total` (où `total = size * size`).
    *    - Ajoute `deviation * 10` au score.
    *
    * 7. **Retourne le score total** :
    *    - Plus le score est **bas**, meilleure est la matrice.
    */
    static int GetPenaltyScore(const std::vector<std::vector<int>>& matrix);

};