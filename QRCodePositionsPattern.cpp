#include "QRCodePositionsPattern.h"

void QRCodePositionsPattern::GeneratePositionsPattern(std::vector<std::vector<int>> &_SimpleQRCode, int _MatrixSize)
{
    GeneratePositionPattern(_SimpleQRCode, 0, 0);
    GeneratePositionPattern(_SimpleQRCode, 0, _MatrixSize - 7);
    GeneratePositionPattern(_SimpleQRCode, _MatrixSize - 7, 0);
}

void QRCodePositionsPattern::GeneratePositionPattern(std::vector<std::vector<int>> &_SimpleQRCode, int _XPositionStart, int _YPositionStart)
{
    for (int y = 0; y < 7; y++)
    {
        for (int x = 0; x < 7; x++)
        {
            if ((x == 0) || (x == 6) || (y == 0) || (y == 6) || ((x >= 2) && (x <= 4) && (y >= 2) && (y <= 4)))
            {
                _SimpleQRCode[_YPositionStart + y][_XPositionStart + x] = 1;
            }
        }
    }
}
