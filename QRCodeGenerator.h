#pragma once
#include "QRCodeData.h"

#include <string>

class QRCodeGenerator
{
public:

	static void GenerateQRCode(const std::string _Text, CorrectionLevel _CorrectionLevel, int _PixelScale = 20, uint8_t _MaskPatern = 0);

	static void GenerateQRCode(const std::string _Text, CorrectionLevel _CorrectionLevel, RGBA _Bit1Color, RGBA _Bit0Color, RGBA _FontColor, int _PixelScale = 20, uint8_t _MaskPatern = 0);

private:

	static QRCodeData GenerateSimpleQR(const std::string _Text, int _QRVersion, CorrectionLevel _CorrectionLevel, uint8_t _MaskPatern);

	static QRPixelData GenerateQRPixelData(const QRCodeData& _QRData, int _PixelScale);

};

