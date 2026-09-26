#pragma once

#include <vector>

class QRCodePositionsPattern
{
public:

    QRCodePositionsPattern() = default;
    ~QRCodePositionsPattern() = default;

    /// @brief Génère les trois motifs de positionnement aux coins de la matrice QR
    /// @param _SimpleQRCode Matrice QR à modifier
    /// @param _MatrixSize Taille de la matrice QR
    static void GeneratePositionsPattern(std::vector<std::vector<int>>& _SimpleQRCode, int _MatrixSize);

private:

    /// @brief Génère un motif de positionnement à une position spécifique dans la matrice QR
    /// @param _SimpleQRCode Matrice QR à modifier
    /// @param _XPositionStart Position X de départ du motif
    /// @param _YPositionStart Position Y de départ du motif
    static void GeneratePositionPattern(std::vector<std::vector<int>>& _SimpleQRCode, int _XPositionStart, int _YPositionStart);

};