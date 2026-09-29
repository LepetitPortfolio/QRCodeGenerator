#include "QRCodeBCHCode.h"

int QRCodeBCHCode::CalculateBCHCode(int _InputeValue, int _MaxPower)
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

std::vector<int> QRCodeBCHCode::CalculateBCHPoly(int _MaxPower)
{
    std::vector<int> g = { 1 }; // Polynôme initial : 1

    for (int i = 0; i < _MaxPower; ++i) {
        std::vector<int> minimal = MinimalPoly(i);
        g = MultiplPoly(g, minimal);
    }

    return g;
}

std::vector<int> QRCodeBCHCode::MinimalPoly(int _Power)
{
    // Exemple simplifié : polynômes minimaux pour GF(2^4)
    switch (_Power) 
    {
        case 1: return { 1, 1, 0, 0, 1 }; // x^4 + x + 1
        case 2: return { 1, 1, 0, 0, 1 }; // x^4 + x + 1
        case 3: return { 1, 0, 1, 0, 1 }; // x^4 + x^2 + 1
        case 4: return { 1, 1, 0, 0, 1 }; // x^4 + x + 1
        default: return { 1, 1 }; // x + 1 (par défaut)
    }
}

std::vector<int> QRCodeBCHCode::MultiplPoly(const std::vector<int> &_P1, const std::vector<int> &_P2)
{
    int deg_p1 = _P1.size() - 1;
    int deg_p2 = _P2.size() - 1;
    std::vector<int> result(deg_p1 + deg_p2 + 1, 0);

    for (int i = 0; i <= deg_p1; ++i) 
    {
        for (int j = 0; j <= deg_p2; ++j) 
        {
            result[i + j] ^= _P1[i] & _P2[j];
        }
    }

    return result;
}