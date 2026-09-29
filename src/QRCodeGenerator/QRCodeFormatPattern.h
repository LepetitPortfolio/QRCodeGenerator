#pragma once
#include "QRCodeData.h"

#include <cstdint>
#include <vector>


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