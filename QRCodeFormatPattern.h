#pragma once

#include <cstdint>
#include <vector>


/// @brief Niveaux de correction pour le QR code
enum CorrectionLevel
{
    L = 1,
    M = 0,
    Q = 3,
    H = 2
};

class QRCodeFormatPattern
{

public:

    /// @brief Génère les motifs de format contenant les informations de correction d'erreur et de masque
    /// @param _SimpleQRCode Matrice QR à modifier
    /// @param _MatrixSize Taille de la matrice QR
    /// @param _CorrectionLevel Niveau de correction d'erreur
    /// @param _MaskPatern Motif de masque utilisé
    static void GenerateFormatPattern(std::vector<std::vector<int>>& _SimpleQRCode, int _MatrixSize, CorrectionLevel _CorrectionLevel, uint8_t _MaskPatern);

};