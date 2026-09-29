#include "QRCodeDataEncoding.h"
#include "QRCodeData.h"

#include <stdexcept>


int QRCodeDataEncoding::SelectVersionForText(const std::string& _Text, CorrectionLevel _CorrectionLevel)
{
	const auto levelIndex = static_cast<std::size_t>(_CorrectionLevel);
	if (levelIndex > 3)
	{
		throw std::invalid_argument("Niveau de correction invalide (L, M, Q ou H).");
	}

	for (int version = 1; version <= 40; ++version) 
	{
		// Le champ de longueur du mode octets fait 8 bits en versions 1–9,
		// puis 16 bits en versions 10–40.
		const int countBitWidth = (version <= 9) ? 8 : 16;
		const std::size_t maxByteCount = (countBitWidth == 8) ? 255 : 65535;

		if (_Text.size() > maxByteCount)
		{
			continue;
		}

		const std::size_t requiredBits = 4 + countBitWidth + _Text.size() * 8;
		const std::size_t availableBits = static_cast<std::size_t>(DataCodewords[version - 1][levelIndex]) * 8;

		if (requiredBits <= availableBits)
		{
			return version;
		}
	}

	throw std::length_error("Le texte ne tient dans aucune version QR de 1 à 40 pour ce niveau.");
}

std::vector<uint8_t> QRCodeDataEncoding::EncodeTextToDataCodewords(const std::string& _Text, int _Version, CorrectionLevel _CorrectionLevel)
{
	if (_Version < 1 || _Version > 40)
	{
		throw std::invalid_argument("La version QR doit être comprise entre 1 et 40.");
	}

	const auto levelIndex = static_cast<std::size_t>(_CorrectionLevel);
	if (levelIndex > 3)
	{
		throw std::invalid_argument("Niveau de correction invalide (L, M, Q ou H).");
	}

	const std::size_t capacityCodewords = DataCodewords[_Version - 1][levelIndex];
	const std::size_t capacityBits = capacityCodewords * 8;
	const int countBitWidth = _Version <= 9 ? 8 : 16;
	const std::size_t maxByteCount = countBitWidth == 8 ? 255 : 65535;

	if (_Text.size() > maxByteCount)
	{
		throw std::length_error("Le texte dépasse le champ de longueur du mode octets.");
	}

	std::vector<uint8_t> bits;
	bits.reserve(4 + countBitWidth + _Text.size() * 8);
	AppendBits(bits, 0b0100, 4); // Indicateur du mode octets
	AppendBits(bits, static_cast<uint32_t>(_Text.size()), countBitWidth);

	for (unsigned char byte : _Text)
	{
		AppendBits(bits, byte, 8);
	}

	if (bits.size() > capacityBits)
	{
		throw std::length_error("Le texte ne tient pas dans cette version et ce niveau de correction.");
	}

	// Ajouter le terminateur (au plus 4 zéros), puis compléter jusqu'à un octet.
	for (int i = 0; i < 4 && bits.size() < capacityBits; ++i)
	{
		bits.push_back(0);
	}

	while (bits.size() % 8 != 0)
	{
		bits.push_back(0);
	}

	std::vector<uint8_t> data;
	data.reserve(capacityCodewords);

	for (std::size_t i = 0; i < bits.size(); i += 8)
	{
		uint8_t codeword = 0;
		for (int bit = 0; bit < 8; ++bit)
		{
			codeword = static_cast<uint8_t>((codeword << 1) | bits[i + bit]);
		}

		data.push_back(codeword);
	}

	uint8_t pad = 0xEC;

	while (data.size() < capacityCodewords)
	{
		data.push_back(pad);
		pad = (pad == 0xEC) ? 0x11 : 0xEC;
	}

	return data;
}

void QRCodeDataEncoding::AppendBits(std::vector<uint8_t>& _Bits, uint32_t _Value, int _BitCount)
{
	for (int i = _BitCount - 1; i >= 0; --i)
	{
		_Bits.push_back(static_cast<uint8_t>((_Value >> i) & 1));
	}
}
