#pragma once

#include <vector>

class QRCodeVersionPattern
{
public:
    QRCodeVersionPattern() = default;
    ~QRCodeVersionPattern() = default;

    /// @brief Génère les motifs de version pour les QR codes de version 7 et supérieure
    /// @param _SimpleQRCode Matrice QR à modifier
    /// @param _MatrixSize Taille de la matrice QR
    /// @param _QRVersion Version du QR code
    static void GenerateVersionPattern(std::vector<std::vector<int>>& _SimpleQRCode, int _MatrixSize, int _QRVersion);

private :

};