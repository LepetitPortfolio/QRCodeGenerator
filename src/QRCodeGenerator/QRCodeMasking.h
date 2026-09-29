#pragma once

#include <vector>

class QRCodeMasking
{
public:
    
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

};