#include "QRCodeVersionPattern.h"
#include "QRCodeBCHCode.h"

void QRCodeVersionPattern::GenerateVersionPattern(std::vector<std::vector<int>> &_SimpleQRCode, int _MatrixSize, int _QRVersion)
{
    int bch = QRCodeBCHCode::CalculateBCHCode(_QRVersion, 12);

    int versionInformation = (_QRVersion << 12) | bch;
    int versionInformationBitIndex = 18;

    // Coin inférieur gauche
    for (int x = 0; x < 6; x++)
    {
        for (int y = _MatrixSize - 11; y < _MatrixSize - 8; y++)
        {
            _SimpleQRCode[y][x] = ((versionInformation >> versionInformationBitIndex) & 1);
            versionInformationBitIndex--;
        }
    }

    versionInformationBitIndex = 18;

    // Coin supérieur droit
    for (int y = 0; y < 6; y++)
    {
        for (int x = _MatrixSize - 11; x < _MatrixSize - 8; x++)
        {
            _SimpleQRCode[y][x] = ((versionInformation >> versionInformationBitIndex) & 1);
            versionInformationBitIndex--;
        }
    }
}


