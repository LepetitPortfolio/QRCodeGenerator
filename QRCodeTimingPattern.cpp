#include "QRCodeTimingPattern.h"

void QRCodeTimingPattern::GenerateTimingPattern(std::vector<std::vector<int>> &_SimpleQRCode, int _MatrixSize)
{
    for (int i = 8; i < _MatrixSize - 8; i++)
    {
        _SimpleQRCode[6][i] = (i % 2 == 0) ? 1 : 0;
        _SimpleQRCode[i][6] = (i % 2 == 0) ? 1 : 0;
    }
}
