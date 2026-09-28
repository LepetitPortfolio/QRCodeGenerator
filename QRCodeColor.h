#pragma once
#include <cstdint>

struct RGB
{
public:
    RGB() = default;
    RGB(uint8_t _R, uint8_t _G, uint8_t _B)
    {
        R = _R;
        G = _G;
        B = _B;
    }

    uint8_t R = 0;
    uint8_t G = 0;
    uint8_t B = 0;
};

struct RGBA : public RGB
{
public:
    RGBA() = default;
    RGBA(uint8_t _R, uint8_t _G, uint8_t _B, uint8_t _A) : RGB(_R, _G, _B)
    {
       
        A = _A;
    }
    uint8_t A = 0;
};