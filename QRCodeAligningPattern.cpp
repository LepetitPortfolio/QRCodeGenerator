#include "QRCodeAligningPattern.h"

void QRCodeAligningPattern::GenerateAligningPattern(std::vector<std::vector<int>> &_SimpleQRCode, int _MatrixSize)
{
    int xPositionStart = _MatrixSize - 9;
	int yPositionStart = _MatrixSize - 9;
	for (int y = 0; y < 5; y++)
	{
		for (int x = 0; x < 5; x++)
		{
			if ((x == 0) || (x == 4) || (y == 0) || (y == 4) || ((x >= 2) && (x <= 2) && (y >= 2) && (y <= 2)))
			{
				_SimpleQRCode[yPositionStart + y][xPositionStart + x] = 1;
			}
		}
	}
}
