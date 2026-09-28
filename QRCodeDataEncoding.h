#pragma once
#include "QRCodeFormatPattern.h"

#include <string>
#include <vector>

class QRCodeDataEncoding
{
public:
	static std::vector<uint8_t> EncodeTextToDataCodewords(const std::string& _Text, int _Version, CorrectionLevel _CorrectionLevel);

private:

	static void AppendBits(std::vector<uint8_t>& _Bits, uint32_t _Value, int _BitCount);
};

