#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <cstdint>

enum CorrectionLevel
{
	L = 1,
	M = 0,
	Q = 3,
	H = 2
};

void WriteInt(std::ofstream& _Out, uint32_t value)
{
	_Out.put((value >> 24) & 0xff);
	_Out.put((value >> 16) & 0xff);
	_Out.put((value >> 8) & 0xff);
	_Out.put(value & 0xff);
}

uint32_t CRC32(const std::vector<unsigned char>& _Data, const char* _Type)
{
	uint32_t crc = 0xffffffffu;
	auto UpdateCRC = [&](unsigned char byte)
		{
			crc ^= (uint32_t)byte;
			for (int i = 0; i < 8; i++)
			{
				if (crc & 1u)
				{
					crc = (crc >> 1) ^ 0xEDB88320u;
				}
				else
				{
					crc = (crc >> 1);
				}
			}
		};

	for (int i = 0; i < 4; i++)
	{
		UpdateCRC(static_cast<unsigned char>(_Type[i]));
	}

	for (unsigned char c : _Data)
	{
		UpdateCRC(c);
	}

	return ~crc;
}

void WriteChunk(std::ofstream& _Out, const char* _Type, const std::vector<unsigned char>& _Data)
{
	WriteInt(_Out, _Data.size()); // length
	_Out.write(_Type, 4); // Chunk type

	if (!_Data.empty())
	{
		_Out.write(reinterpret_cast<const char*>(_Data.data()), _Data.size()); // data
	}

	uint32_t crc32 = CRC32(_Data, _Type);

	WriteInt(_Out, crc32); // CRC
}

uint32_t Adler32(const std::vector<unsigned char>& _Data)
{
	const uint32_t MOD_ADLER = 65521;
	uint32_t a = 1, b = 0;

	for (unsigned char c : _Data)
	{
		a = (a + c) % MOD_ADLER;
		b = (b + a) % MOD_ADLER;
	}

	return (b << 16) | a;
}

void SavePNG(const std::string& _Filename, int _Width, int _Height, const std::vector<unsigned char>& _PixelData)
{
	std::ofstream out(_Filename, std::ios::binary);

	unsigned char pngSignature[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };
	out.write((char*)pngSignature, 8);

	// IHDR chunk

	std::vector<unsigned char> ihdrData;
	for (int i = 24; i >= 0; i -= 8)
	{
		ihdrData.push_back((_Width >> i) & 0xff);
	}

	for (int i = 24; i >= 0; i -= 8)
	{
		ihdrData.push_back((_Height >> i) & 0xff);
	}

	ihdrData.insert(ihdrData.end(), { 8, 6, 0, 0, 0 }); // Bit depth, color type, compression, filter, interlace

	WriteChunk(out, "IHDR", ihdrData);

	std::vector<unsigned char> raw;
	raw.reserve((_Width * 4 + 1) * _Height);
	for (int y = 0; y < _Height; y++)
	{
		raw.push_back(0x00); // No filter
		const unsigned char* row = &_PixelData[y * _Width * 4];
		raw.insert(raw.end(), row, row + _Width * 4);
	}

	// IDAT (pas compressé, format alib trés simplifié)
	std::vector<unsigned char> idatData;
	idatData.push_back(0x78);
	idatData.push_back(0x01);

	size_t pos = 0;
	while (pos < raw.size())
	{
		size_t remain = raw.size() - pos;
		uint16_t chunkSize = static_cast<uint16_t>(remain > 65535 ? 65535 : remain);
		bool isFinal = (pos + chunkSize) == raw.size();

		unsigned char blockHeader = isFinal ? 0x01 : 0x00;
		idatData.push_back(blockHeader);

		idatData.push_back(static_cast<unsigned char>(chunkSize & 0xff));
		idatData.push_back(static_cast<unsigned char>((chunkSize >> 8) & 0xff));
		uint16_t nlen = static_cast<uint16_t>(~chunkSize);
		idatData.push_back(static_cast<unsigned char>(nlen & 0xff));
		idatData.push_back(static_cast<unsigned char>((nlen >> 8) & 0xff));

		idatData.insert(idatData.end(), raw.begin() + pos, raw.begin() + pos + chunkSize);

		pos += chunkSize;
	}

	uint32_t adler = Adler32(raw);
	idatData.push_back((adler >> 24) & 0xff);
	idatData.push_back((adler >> 16) & 0xff);
	idatData.push_back((adler >> 8) & 0xff);
	idatData.push_back(adler & 0xff);

	WriteChunk(out, "IDAT", idatData);
	WriteChunk(out, "IEND", {});

	out.close();
}

// Fonction pour multiplier deux polynômes dans GF(2)
std::vector<int> MultiplPoly(const std::vector<int>& _P1, const std::vector<int>& _P2) {
	int deg_p1 = _P1.size() - 1;
	int deg_p2 = _P2.size() - 1;
	std::vector<int> result(deg_p1 + deg_p2 + 1, 0);

	for (int i = 0; i <= deg_p1; ++i) {
		for (int j = 0; j <= deg_p2; ++j) {
			result[i + j] ^= _P1[i] & _P2[j];
		}
	}

	return result;
}

// Fonction pour calculer le polynôme minimal pour une puissance donnée (simplifiée)
std::vector<int> MinimalPoly(int _Power) {
	// Exemple simplifié : polynômes minimaux pour GF(2^4)
	// En pratique, il faudrait une table ou un calcul plus complexe
	switch (_Power) {
	case 1: return { 1, 1, 0, 0, 1 }; // x^4 + x + 1
	case 2: return { 1, 1, 0, 0, 1 }; // x^4 + x + 1 (exemple)
	case 3: return { 1, 0, 1, 0, 1 }; // x^4 + x^2 + 1 (exemple)
	case 4: return { 1, 1, 0, 0, 1 }; // x^4 + x + 1 (exemple)
	default: return { 1, 1 }; // x + 1 (par défaut)
	}
}

// Fonction pour calculer le polynôme générateur BCH
std::vector<int> CalculateBCHPoly(int _MaxPower) {
	std::vector<int> g = { 1 }; // Polynôme initial : 1

	for (int i = 0; i < _MaxPower; ++i) {
		std::vector<int> minimal = MinimalPoly(i);
		g = MultiplPoly(g, minimal);
	}

	return g;
}

int CalculateBCHCode(int _InputeValue, int _MaxPower)
{
	std::vector<int> bchPoly = CalculateBCHPoly(_MaxPower);

	int vX = _InputeValue << (bchPoly.size() - 1);
	int gX = 0;
	for (size_t i = 0; i < bchPoly.size(); ++i)
	{
		if (bchPoly[i])
		{
			gX |= (1 << (bchPoly.size() - 1 - i));
		}
	}
	return vX ^ gX;
}

int GetMatrixSize(int _QRVersion)
{
	return 21 + (_QRVersion - 1) * 4;
}

void GeneratePositionPattern(std::vector<std::vector<int>>& _SimpleQRCode, int _XPositionStart, int _YPositionStart)
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

void GeneratePositionsPattern(std::vector<std::vector<int>>& _SimpleQRCode, int _MatrixSize)
{
	GeneratePositionPattern(_SimpleQRCode, 0, 0);
	GeneratePositionPattern(_SimpleQRCode, 0, _MatrixSize - 7);
	GeneratePositionPattern(_SimpleQRCode, _MatrixSize - 7, 0);
}

void GenerateAligningPattern(std::vector<std::vector<int>>& _SimpleQRCode, int _MatrixSize)
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

void GenerateTimingPattern(std::vector<std::vector<int>>& _SimpleQRCode, int _MatrixSize)
{
	for (int i = 8; i < _MatrixSize - 8; i++)
	{
		_SimpleQRCode[6][i] = (i % 2 == 0) ? 1 : 0;
		_SimpleQRCode[i][6] = (i % 2 == 0) ? 1 : 0;
	}
}

void GenerateFormatPattern(std::vector<std::vector<int>>& _SimpleQRCode, int _MatrixSize, CorrectionLevel _CorrectionLevel, uint8_t _MaskPatern)
{
	uint8_t formatInformationValue = (static_cast<uint8_t>(_CorrectionLevel) << 3) | (_MaskPatern & 0x07);
	int bch = CalculateBCHCode(formatInformationValue, 10);
	uint16_t formatInformation = (formatInformationValue << 10) | bch;

	// Zone A Up-Left
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

	//Zone B Down-Left and Up-Right

	for (int bitIndex = 0; bitIndex < 15; bitIndex++)
	{
		int bit = (formatInformation >> bitIndex) & 1;
		if (bitIndex < 8) // down left
		{
			_SimpleQRCode[_MatrixSize - 1 - bitIndex][8] = bit;
		}
		else // up right
		{
			_SimpleQRCode[8][_MatrixSize - 15 + bitIndex] = bit;
		}
	}
}

void GenerateVersionPattern(std::vector<std::vector<int>>& _SimpleQRCode, int _MatrixSize, int _QRVersion)
{
	int bch = CalculateBCHCode(_QRVersion, 12);

	int versionInformation = (_QRVersion << 12) | bch;
	int versionInformationBitIndex = 18;

	// down left

	for (int x = 0; x < 6; x++)
	{
		for (int y = _MatrixSize - 11; y < _MatrixSize - 8; y++)
		{
			_SimpleQRCode[y][x] = ((versionInformation >> versionInformationBitIndex) & 1);
			versionInformationBitIndex--;
		}
	}

	versionInformationBitIndex = 18;

	// up right
	for (int y = 0; y < 6; y++)
	{
		for (int x = _MatrixSize - 11; x < _MatrixSize - 8; x++)
		{
			_SimpleQRCode[y][x] = ((versionInformation >> versionInformationBitIndex) & 1);
			versionInformationBitIndex--;
		}
	}
}

std::vector<std::vector<int>> GenerateSimpleQR(const std::string _Text, int _QRVersion, CorrectionLevel _CorrectionLevel, uint8_t _MaskPatern)
{
	int matrixSize = GetMatrixSize(_QRVersion);
	std::vector<std::vector<int>> simpleQRCode(matrixSize, std::vector<int>(matrixSize, 0));

	GeneratePositionsPattern(simpleQRCode, matrixSize);
	GenerateTimingPattern(simpleQRCode, matrixSize);
	if (_QRVersion > 1)
	{
		GenerateAligningPattern(simpleQRCode, matrixSize);

		if (_QRVersion > 6)
		{
			GenerateVersionPattern(simpleQRCode, matrixSize, _QRVersion);
		}
	}

	GenerateFormatPattern(simpleQRCode, matrixSize, _CorrectionLevel, _MaskPatern);

	// Fill with deterministic pseudo-random bits
	/*int hash = 0;
	for (char c : _Text)
	{
		hash = (hash * 131 + static_cast<unsigned char>(c)) & 0xFFFF;
	}

	for (int y = 7; y < _MatrixSize - 7; y++)
	{
		for (int x = 7; x < _MatrixSize - 7; x++)
		{
			simpleQRCode[y][x] = ((x * 17 + y * 31 + hash) & 1);
		}
	}*/

	return simpleQRCode;
}

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

int main()
{
	std::string text;
	std::cout << "Entrez le texte ou URL à coder : ";
	std::getline(std::cin, text);

	std::vector<std::vector<int>> qrMatrix = GenerateSimpleQR(text, 7, CorrectionLevel::M, 2);

	ShowMatrixQR(qrMatrix, " ", "1");

	int scale = 20;
	int size = qrMatrix.size() * scale;

	// couleur du QR (Modifiables)
	unsigned char R1 = 20, G1 = 20, B1 = 20, A1 = 255; //module 
	unsigned char R0 = 240, G0 = 240, B0 = 240, A0 = 255; //fond (transparent)

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