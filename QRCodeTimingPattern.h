#pragma once

#include <vector>

class QRCodeTimingPattern
{
public:

    /// @brief Génère les motifs de synchronisation (timing patterns) entre les motifs de positionnement
    /// @param _SimpleQRCode Matrice QR à modifier
    /// @param _MatrixSize Taille de la matrice QR
    static void GenerateTimingPattern(std::vector<std::vector<int>>& _SimpleQRCode, int _MatrixSize);

private:

};