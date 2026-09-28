#include "QRCodeFunctionPatterns.h"

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <stdexcept>
#include <vector>

void QRCodeFunctionPatterns::Generate(QRCodeData& _QRCodeData, int _Version, CorrectionLevel _CorrectionLevel, int _MaskPattern)
{
    if (_Version < 1 || _Version > 40)
        throw std::invalid_argument("La version QR doit être comprise entre 1 et 40.");

    const int size = 17 + 4 * _Version;;
    _QRCodeData.MatrixQR = std::vector<std::vector<int>>(static_cast<std::size_t>(size), std::vector<int>(static_cast<std::size_t>(size), 0));
    _QRCodeData.Reserved = std::vector<std::vector<bool>>(static_cast<std::size_t>(size), std::vector<bool>(static_cast<std::size_t>(size), false));

    // Lignes de synchronisation; les repères les recouvrent aux intersections.
    for (int i = 0; i < size; ++i) 
    {
        Set(_QRCodeData, 6, i, i % 2 == 0);
        Set(_QRCodeData, i, 6, i % 2 == 0);
    }

    DrawFinder(_QRCodeData, 3, 3);
    DrawFinder(_QRCodeData, size - 4, 3);
    DrawFinder(_QRCodeData, 3, size - 4);

    const auto centers = GetAlignmentCenters(_Version, size);
    for (std::size_t row = 0; row < centers.size(); ++row) 
    {
        for (std::size_t col = 0; col < centers.size(); ++col) 
        {
            // Les trois coins contenant déjà un grand repère n'ont pas d'alignement.
            if ((row == 0 && col == 0) ||
                (row == 0 && col + 1 == centers.size()) ||
                (row + 1 == centers.size() && col == 0))
            {
                continue;
            }

            DrawAlignment(_QRCodeData, centers[col], centers[row]);
        }
    }

    DrawFormat(_QRCodeData, _CorrectionLevel, _MaskPattern);
    DrawVersion(_QRCodeData, _Version);
}

void QRCodeFunctionPatterns::Set(QRCodeData& _QRCodeData, int _X, int _Y, bool _Dark)
{
    const int size = static_cast<int>(_QRCodeData.MatrixQR.size());
    if (_X < 0 || _Y < 0 || _X >= size || _Y >= size)
    {
        return;
    }

    _QRCodeData.MatrixQR[static_cast<std::size_t>(_Y)][static_cast<std::size_t>(_X)] = _Dark ? 1 : 0;
    _QRCodeData.Reserved[static_cast<std::size_t>(_Y)][static_cast<std::size_t>(_X)] = true;
}

void QRCodeFunctionPatterns::DrawFinder(QRCodeData& _QRCodeData, int _CenterX, int _CenterY)
{
    for (int dy = -4; dy <= 4; ++dy) 
    {
        for (int dx = -4; dx <= 4; ++dx) 
        {
            const int distance = std::max(std::abs(dx), std::abs(dy));
            Set(_QRCodeData, _CenterX + dx, _CenterY + dy, distance != 2 && distance != 4);
        }
    }
}

std::vector<int> QRCodeFunctionPatterns::GetAlignmentCenters(int _Version, int _Size)
{
    if (_Version == 1)
    {
        return {};
    }

    const int count = _Version / 7 + 2;
    const int step = (_Version * 8 + count * 3 + 5) / (count * 4 - 4) * 2;
    std::vector<int> centers(static_cast<std::size_t>(count));
    centers[0] = 6;
    
    for (int i = count - 1, position = _Size - 7; i >= 1; --i, position -= step)
    {
        centers[static_cast<std::size_t>(i)] = position;
    }

    return centers;
}

void QRCodeFunctionPatterns::DrawAlignment(QRCodeData& _QRCodeData, int _CenterX, int _CenterY)
{
    for (int dy = -2; dy <= 2; ++dy) 
    {
        for (int dx = -2; dx <= 2; ++dx) 
        {
            const int distance = std::max(std::abs(dx), std::abs(dy));
            Set(_QRCodeData, _CenterX + dx, _CenterY + dy, distance != 1);
        }
    }
}

void QRCodeFunctionPatterns::DrawFormat(QRCodeData& _QRCodeData, CorrectionLevel _CorrectionLevel, int _MaskPattern)
{
    // Format encodage: L=01, M=00, Q=11, H=10.
    static constexpr int levelBits[] = { 1, 0, 3, 2 };
    const int levelIndex = static_cast<int>(_CorrectionLevel);
    
    if (levelIndex < 0 || levelIndex > 3)
    {
        throw std::invalid_argument("Niveau de correction invalide.");
    }
    
    if (_MaskPattern < 0 || _MaskPattern > 7)
    {
        throw std::invalid_argument("Le masque QR doit être compris entre 0 et 7.");
    }

    const int data = (levelBits[levelIndex] << 3) | _MaskPattern;
    int remainder = data;
    
    for (int i = 0; i < 10; ++i)
    {
        remainder = (remainder << 1) ^ ((remainder >> 9) * 0x537);
    }

    const int bits = ((data << 10) | remainder) ^ 0x5412;
    const int size = static_cast<int>(_QRCodeData.MatrixQR.size());

    // Première copie, autour du repère supérieur gauche.
    for (int i = 0; i <= 5; ++i)
    {
        Set(_QRCodeData, 8, i, ((bits >> i) & 1) != 0);
    }
    
    Set(_QRCodeData, 8, 7, ((bits >> 6) & 1) != 0);
    Set(_QRCodeData, 8, 8, ((bits >> 7) & 1) != 0);
    Set(_QRCodeData, 7, 8, ((bits >> 8) & 1) != 0);
    
    for (int i = 9; i < 15; ++i)
    {
        Set(_QRCodeData, 14 - i, 8, ((bits >> i) & 1) != 0);
    }

    // Deuxième copie, près des bords inférieur gauche et supérieur droit.
    for (int i = 0; i < 8; ++i) 
    {    
        Set(_QRCodeData, size - 1 - i, 8, ((bits >> i) & 1) != 0);
    }
    
    for (int i = 8; i < 15; ++i)
    {
        Set(_QRCodeData, 8, size - 15 + i, ((bits >> i) & 1) != 0);
    }
    
    Set(_QRCodeData, 8, size - 8, true); // module sombre fixe
}

void QRCodeFunctionPatterns::DrawVersion(QRCodeData& _QRCodeData, int _Version)
{
    if (_Version < 7)
    {
        return;
    }

    int remainder = _Version;
    
    for (int i = 0; i < 12; ++i)
    {
        remainder = (remainder << 1) ^ ((remainder >> 11) * 0x1F25);
    }

    const int bits = (_Version << 12) | remainder;
    const int size = static_cast<int>(_QRCodeData.MatrixQR.size());

    for (int i = 0; i < 18; ++i) 
    {
        const bool bit = ((bits >> i) & 1) != 0;
        const int a = size - 11 + i % 3;
        const int b = i / 3;
        Set(_QRCodeData, a, b, bit);
        Set(_QRCodeData, b, a, bit);
    }
}


