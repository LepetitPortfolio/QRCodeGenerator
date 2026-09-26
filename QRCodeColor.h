#pragma once
#include <cstdint>

struct RGB
{
public:
    uint8_t R = 0;
    uint8_t G = 0;
    uint8_t B = 0;
};

struct RGBA : public RGB
{
public:
    uint8_t A = 0;
    
};