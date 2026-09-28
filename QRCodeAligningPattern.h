#pragma once

#include <vector>

class QRCodeAligningPattern
{
public:

    /**
    * @brief Génère un motif d'alignement (Alignment Pattern) dans une matrice de code QR.
    * @param _SimpleQRCode Matrice 2D où générer le motif d'alignement (sera modifiée).
    * @param _MatrixSize Taille de la matrice du code QR (en modules).
    *
    * @details
    * ### Description :
    * - Cette fonction place un **motif d'alignement 5x5** dans le coin **bas-droite** de la matrice.
    * - Le motif est positionné à `_MatrixSize - 9` en X et Y, ce qui le place à 8 modules du bord droit et du bord inférieur (car 5 + 3 = 8, mais le calcul semble incorrect, voir la note ci-dessous).
    *
    * ### Structure du motif :
    * Le motif d'alignement est un carré 5x5 où :
    * - Les **bords** (x=0, x=4, y=0, y=4) sont noirs (1).
    * - Le **centre** (x=2, y=2) est noir (1).
    * - Le reste est blanc (0, par défaut dans la matrice).
    *
    * ### Algorithme :
    * 1. **Calcul de la position de départ** :
    *    - `xPositionStart = _MatrixSize - 9` : Position X du coin supérieur gauche du motif.
    *    - `yPositionStart = _MatrixSize - 9` : Position Y du coin supérieur gauche du motif.
    *    - **⚠️ Note** : Le calcul `_MatrixSize - 9` semble incorrect pour un motif 5x5. Pour un motif 5x5, la position devrait être `_MatrixSize - 5` pour le placer dans le coin bas-droite.
    *      - Exemple : Pour une matrice de taille 21 (version 1), `_MatrixSize - 9 = 12`, ce qui place le motif à (12, 12). Cela semble correct pour un motif d'alignement, mais il faudrait vérifier la norme QR.
    *
    * 2. **Placement des modules noirs** :
    *    - Parcourt chaque cellule du motif 5x5 (x de 0 à 4, y de 0 à 4).
    *    - Si la cellule est sur le **bord** (x=0, x=4, y=0, y=4) ou au **centre** (x=2, y=2), elle est marquée comme noire (`1`).
    *    - Les autres cellules restent blanches (`0`, par défaut).
    *
    * @note
    * - **Problème potentiel** : La condition `((x >= 2) && (x <= 2) && (y >= 2) && (y <= 2))` peut être simplifiée en `(x == 2 && y == 2)`.
    * - **Positionnement** : Le motif est toujours placé dans le coin bas-droite. Dans un code QR réel, les motifs d'alignement sont répartis dans toute la matrice (pas seulement dans le coin bas-droite).
    *   - Pour une implémentation complète, il faudrait une fonction qui place **tous les motifs d'alignement** pour une version donnée (voir la norme QR pour les positions).
    */
    static void GenerateAligningPattern(std::vector<std::vector<int>>& _SimpleQRCode, int _MatrixSize);

private:
};