#include "QRCodeMasking.h"

#include <cstddef>
#include <stdexcept>

void QRCodeMasking::ApplyMask(std::vector<std::vector<int>>& _Matrix, const std::vector<std::vector<bool>>& _Reserved, int _MaskPattern)
{
    if (_MaskPattern < 0 || _MaskPattern > 7)
    {
        throw std::invalid_argument("Le masque QR doit être compris entre 0 et 7.");
    }

    if (_Matrix.empty() || _Matrix.size() != _Reserved.size())
    {
        throw std::invalid_argument("La matrice et la carte réservée doivent avoir la même taille.");
    }

    const std::size_t size = _Matrix.size();
    for (std::size_t y = 0; y < size; ++y)
    {
        if (_Matrix[y].size() != size || _Reserved[y].size() != size)
        {
            throw std::invalid_argument("La matrice et la carte réservée doivent être carrées.");
        }

        for (std::size_t x = 0; x < size; ++x)
        {
            if (!_Reserved[y][x] && ShouldInvert(static_cast<int>(x), static_cast<int>(y), _MaskPattern))
            {
                _Matrix[y][x] ^= 1;
            }
        }
    }
}

bool QRCodeMasking::ShouldInvert(int _X, int _Y, int _MaskPattern)
{
    switch (_MaskPattern)
    {
        case 0: 
            return (_X + _Y) % 2 == 0;
        case 1: 
            return _Y % 2 == 0;
        case 2: 
            return _X % 3 == 0;
        case 3: 
            return (_X + _Y) % 3 == 0;
        case 4: 
            return (_X / 3 + _Y / 2) % 2 == 0;
        case 5: 
            return (_X * _Y) % 2 + (_X * _Y) % 3 == 0;
        case 6: 
            return ((_X * _Y) % 2 + (_X * _Y) % 3) % 2 == 0;
        case 7: 
            return ((_X + _Y) % 2 + (_X * _Y) % 3) % 2 == 0;
        default: 
            throw std::invalid_argument("Le masque QR doit être compris entre 0 et 7.");
    }
}
