#include "QRCodeData.h"
#include "QRCodePositionsPattern.h"
#include "QRCodeTimingPattern.h"
#include "QRCodeAligningPattern.h"
#include "QRCodeVersionPattern.h"
#include "QRCodeFormatPattern.h"
#include "QRCodeDataEncoding.h"


#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <cstdint>


/// @brief Calcule la taille de la matrice QR en fonction de la version
/// @param _QRVersion Version du QR code (1 à 40)
/// @return Taille de la matrice (nombre de modules par côté)
int GetMatrixSize(int _QRVersion)
{
    return 21 + (_QRVersion - 1) * 4;
}



/// @brief Génère une matrice QR simple avec les motifs de base
/// @param _Text Texte ou URL à encoder
/// @param _QRVersion Version du QR code
/// @param _CorrectionLevel Niveau de correction d'erreur
/// @param _MaskPatern Motif de masque à appliquer
/// @return Matrice QR sous forme de tableau 2D d'entiers (0 ou 1)
std::vector<std::vector<int>> GenerateSimpleQR(const std::string _Text, int _QRVersion, CorrectionLevel _CorrectionLevel, uint8_t _MaskPatern)
{
	QRCodeData qrData;
    int matrixSize = GetMatrixSize(_QRVersion);
    std::vector<std::vector<int>> simpleQRCode(matrixSize, std::vector<int>(matrixSize, 0));

    QRCodePositionsPattern::GeneratePositionsPattern(simpleQRCode, matrixSize);
    QRCodeTimingPattern::GenerateTimingPattern(simpleQRCode, matrixSize);
    
    if (_QRVersion > 1)
    {
        QRCodeAligningPattern::GenerateAligningPattern(simpleQRCode, matrixSize);

        if (_QRVersion > 6)
        {
            QRCodeVersionPattern::GenerateVersionPattern(simpleQRCode, matrixSize, _QRVersion);
        }
    }

    QRCodeFormatPattern::GenerateFormatPattern(simpleQRCode, matrixSize, _CorrectionLevel, _MaskPatern);

    qrData.Bits = QRCodeDataEncoding::EncodeTextToDataCodewords(_Text, _QRVersion, _CorrectionLevel);

    return simpleQRCode;
}

/// @brief Affiche la matrice QR dans la console avec des caractères personnalisables
/// @param _MatrixQR Matrice QR à afficher
/// @param _0Bit Caractère utilisé pour représenter un 0
/// @param _1bit Caractère utilisé pour représenter un 1
void ShowMatrixQR(const std::vector<std::vector<int>>& _MatrixQR, std::string _0Bit, std::string _1bit)
{
    for (const auto& row : _MatrixQR)
    {
        for (int bit : row)
        {
            std::cout << (bit ? _1bit : _0Bit);
        }
        std::cout << std::endl;
    }
}

/// @brief Fonction principale : demande le texte à l'utilisateur, génère le QR code et l'affiche
/// @return Code de retour du programme
int main()
{
    std::string text;
    std::cout << "Entrez le texte ou URL à coder : ";
    std::getline(std::cin, text);

    std::vector<std::vector<int>> qrMatrix = GenerateSimpleQR(text, 7, CorrectionLevel::M, 2);

    ShowMatrixQR(qrMatrix, " ", "1");

    int scale = 20;
    int size = qrMatrix.size() * scale;

    // Couleur du QR (modifiables)
    unsigned char R1 = 20, G1 = 20, B1 = 20, A1 = 255; // module
    unsigned char R0 = 240, G0 = 240, B0 = 240, A0 = 255; // fond (transparent)

    std::vector<unsigned char> pixelData(size * size * 4);

    for (int y = 0; y < size; y++)
    {
        for (int x = 0; x < size; x++)
        {
            int qrX = x / scale;
            int qrY = y / scale;

            bool bit = qrMatrix[qrY][qrX];

            int idx = (y * size + x) * 4;

            pixelData[idx + 0] = bit ? R1 : R0;
            pixelData[idx + 1] = bit ? G1 : G0;
            pixelData[idx + 2] = bit ? B1 : B0;
            pixelData[idx + 3] = bit ? A1 : A0;
        }
    }

    //SavePNG("QRCode.png", size, size, pixelData);
    //std::cout << "QR Code enregistré sous 'QRCode.png'" << std::endl;

    return 0;
}