#include "QRCodeFormatPattern.h"
#include "QRCodeBCHCode.h"

void QRCodeFormatPattern::GenerateFormatPattern(std::vector<std::vector<int>> &_SimpleQRCode, int _MatrixSize, CorrectionLevel _CorrectionLevel, uint8_t _MaskPatern)
{
    uint8_t formatInformationValue = (static_cast<uint8_t>(_CorrectionLevel) << 3) | (_MaskPatern & 0x07);
    int bch = QRCodeBCHCode::CalculateBCHCode(formatInformationValue, 10);
    uint16_t formatInformation = (formatInformationValue << 10) | bch;

    // Zone A : coin supérieur gauche
    for (int bitIndex = 0; bitIndex < 15; bitIndex++)
    {
        int bit = (formatInformation >> bitIndex) & 1;

        if (bitIndex < 7) // vertical
        {
            if (bitIndex >= 6)
            {
                _SimpleQRCode[bitIndex + 1][8] = bit;
            }
            else
            {
                _SimpleQRCode[bitIndex][8] = bit;
            }

        }
        else // horizontal
        {

            if (14 - bitIndex >= 6)
            {
                _SimpleQRCode[8][14 - bitIndex + 1] = bit;
            }
            else
            {
                _SimpleQRCode[8][ 14 - bitIndex] = bit;
            }
        }
    }

    // Zone B : coin inférieur gauche et coin supérieur droit
    for (int bitIndex = 0; bitIndex < 15; bitIndex++)
    {
        int bit = (formatInformation >> bitIndex) & 1;
        if (bitIndex < 8) // coin inférieur gauche
        {
            _SimpleQRCode[_MatrixSize - 1 - bitIndex][8] = bit;
        }
        else // coin supérieur droit
        {
            _SimpleQRCode[8][_MatrixSize - 15 + bitIndex] = bit;
        }
    }
}
