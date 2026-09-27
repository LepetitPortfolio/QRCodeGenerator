
#pragma once
#include "QRCodeColor.h"

#include <vector>
#include <cstdint>


struct QRCodeData
{
public:
    std::vector<std::vector<int>> MatrixQR;
    std::vector<uint8_t> Bits;
    RGBA Bit1Color;
    RGBA Bit0Color;

    RGBA FontColor;


};
