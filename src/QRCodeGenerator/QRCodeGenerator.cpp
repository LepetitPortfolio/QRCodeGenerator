#include "QRCodeGenerator.h"
#include "QRCodeFunctionPatterns.h"
#include "QRCodeDataEncoding.h"
#include "QRCodeReedSolomonCorrector.h"
#include "QRCodeBitPlacement.h"
#include "QRCodeMasking.h"
#include "QRCodePNGFile.h"

void QRCodeGenerator::GenerateQRCode(const std::string& _Filename, const std::string _Text, CorrectionLevel _CorrectionLevel, int _PixelScale)
{
	GenerateQRCode(_Filename, _Text, _CorrectionLevel, { 20, 20, 20, 255 }, { 240, 240, 240, 255 }, { 240, 240, 240, 255 }, _PixelScale);
}

void QRCodeGenerator::GenerateQRCode(const std::string& _Filename, const std::string _Text, CorrectionLevel _CorrectionLevel, RGBA _Bit1Color, RGBA _Bit0Color, RGBA _FontColor, int _PixelScale)
{
	int version = QRCodeDataEncoding().SelectVersionForText(_Text, _CorrectionLevel);
	QRCodeData data = GenerateSimpleQR(_Text, version, _CorrectionLevel);

	int scale = _PixelScale;

	// Couleur du QR (modifiables)
	data.FontColor = _FontColor;
	data.Bit1Color = _Bit1Color;
	data.Bit0Color = _Bit0Color;

	QRPixelData pixelData = GenerateQRPixelData(data, scale);

	QRCodePNGFile::GeneratePNGFile(_Filename, pixelData.Width, pixelData.Height, pixelData.PixelData);
}

QRCodeData QRCodeGenerator::GenerateSimpleQR(const std::string _Text, int _QRVersion, CorrectionLevel _CorrectionLevel)
{
	QRCodeData outQRData{};

	outQRData.Bits = QRCodeDataEncoding::EncodeTextToDataCodewords(_Text, _QRVersion, _CorrectionLevel);

	outQRData.Bits = QRCodeReedSolomonCorrector::ErrorCorrectionAndInterleave(outQRData.Bits, _QRVersion, _CorrectionLevel);

	const int maskPattern = QRCodeMasking::SelectBestMask(outQRData.Bits, _QRVersion, _CorrectionLevel);
	if (maskPattern < 0 || maskPattern > 7)
	{
		throw std::invalid_argument("Le masque QR doit être compris entre 0 et 7, ou 255 pour le choix automatique.");
	}

	QRCodeFunctionPatterns::Generate(outQRData, _QRVersion, _CorrectionLevel, maskPattern);

	QRCodeBitPlacement::PlaceCodewords(outQRData.Bits, _QRVersion, outQRData.MatrixQR, outQRData.Reserved);

	QRCodeMasking::ApplyMask(outQRData.MatrixQR, outQRData.Reserved, maskPattern);

	return outQRData;
}

QRPixelData QRCodeGenerator::GenerateQRPixelData(const QRCodeData& _QRData, int _PixelScale, int _MargeSize)
{
	int size = (_QRData.MatrixQR.size() + _MargeSize * 2) * _PixelScale;
	int pixelMargeSize = _MargeSize * _PixelScale;

	QRPixelData outQRPixelData(size, size, _PixelScale, _QRData.FontColor);

	for (int y = pixelMargeSize; y < size - pixelMargeSize; y++)
	{
		for (int x = pixelMargeSize; x < size - pixelMargeSize; x++)
		{
			int qrX = (x / _PixelScale) - _MargeSize;
			int qrY = (y / _PixelScale) - _MargeSize;

			bool bit = _QRData.MatrixQR[qrY][qrX];

			int idx = (y * size + x);

			outQRPixelData.PixelData[idx] = bit ? _QRData.Bit1Color : _QRData.Bit0Color;
		}
	}

	return outQRPixelData;
}


