#include "QRCodeGenerator.h"
#include "QRCodeFunctionPatterns.h"
#include "QRCodeFormatPattern.h"
#include "QRCodeDataEncoding.h"
#include "QRCodeReedSolomonCorrector.h"
#include "QRCodeBitPlacement.h"
#include "QRCodeMasking.h"
#include "QRCodePNGFile.h"

void QRCodeGenerator::GenerateQRCode(const std::string& _Filename, const std::string _Text, CorrectionLevel _CorrectionLevel, int _PixelScale, uint8_t _MaskPatern)
{
	GenerateQRCode(_Filename, _Text, _CorrectionLevel, { 20, 20, 20, 255 }, { 240, 240, 240, 255 }, { 240, 240, 240, 255 }, _PixelScale, _MaskPatern);
}

void QRCodeGenerator::GenerateQRCode(const std::string& _Filename, const std::string _Text, CorrectionLevel _CorrectionLevel, RGBA _Bit1Color, RGBA _Bit0Color, RGBA _FontColor, int _PixelScale, uint8_t _MaskPatern)
{
	int version = QRCodeDataEncoding().SelectVersionForText(_Text, _CorrectionLevel);
	QRCodeData data = GenerateSimpleQR(_Text, version, _CorrectionLevel, _MaskPatern);

	int scale = _PixelScale;

	// Couleur du QR (modifiables)
	data.FontColor = _FontColor;
	data.Bit1Color = _Bit1Color;
	data.Bit0Color = _Bit0Color;

	QRPixelData pixelData = GenerateQRPixelData(data, scale);

	QRCodePNGFile::GeneratePNGFile(_Filename, pixelData.Width, pixelData.Height, pixelData.PixelData);
}

QRCodeData QRCodeGenerator::GenerateSimpleQR(const std::string _Text, int _QRVersion, CorrectionLevel _CorrectionLevel, uint8_t _MaskPatern)
{
	QRCodeData outQRData{};
	int matrixSize = 21 + (_QRVersion - 1) * 4;

	QRCodeFunctionPatterns::Generate(outQRData, _QRVersion, _CorrectionLevel, _MaskPatern);

	QRCodeFormatPattern::GenerateFormatPattern(outQRData.MatrixQR, matrixSize, _CorrectionLevel, _MaskPatern);

	outQRData.Bits = QRCodeDataEncoding::EncodeTextToDataCodewords(_Text, _QRVersion, _CorrectionLevel);

	outQRData.Bits = QRCodeReedSolomonCorrector::ErrorCorrectionAndInterleave(outQRData.Bits, _QRVersion, _CorrectionLevel);

	QRCodeBitPlacement::PlaceCodewords(outQRData.Bits, _QRVersion, outQRData.MatrixQR);

	QRCodeMasking::ApplyMask(outQRData.MatrixQR, outQRData.Reserved, _MaskPatern);

	return outQRData;
}

QRPixelData QRCodeGenerator::GenerateQRPixelData(const QRCodeData& _QRData, int _PixelScale)
{
	int size = _QRData.MatrixQR.size() * _PixelScale;
	QRPixelData outQRPixelData(size, size, _PixelScale, _QRData.FontColor);

	for (int y = 0; y < size; y++)
	{
		for (int x = 0; x < size; x++)
		{
			int qrX = x / _PixelScale;
			int qrY = y / _PixelScale;

			bool bit = _QRData.MatrixQR[qrY][qrX];

			int idx = (y * size + x);

			outQRPixelData.PixelData[idx] = bit ? _QRData.Bit1Color : _QRData.Bit0Color;
		}
	}

	return outQRPixelData;
}
