#pragma once
#include "QRCodeFormatPattern.h"

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

class QRCodeReedSolomonCorrector
{
public :

    static std::vector<uint8_t> ErrorCorrectionAndInterleave(const std::vector<uint8_t>& _Data, int _Version, CorrectionLevel correctionLevel);

    static int GetRawCodewordCount(int _Version);

private:

    // Multiplication dans GF(256), polynôme primitif 0x11D.
    static uint8_t MultiplyGF256(uint8_t _X, uint8_t _Y);

	static std::vector<uint8_t> MakeGenerator(int _Degree);

    static std::vector<uint8_t> MakeRemainder(const std::vector<uint8_t>& _Data, const std::vector<uint8_t>& _Generator);
};

