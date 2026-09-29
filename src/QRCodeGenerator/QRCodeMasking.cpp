#include "QRCodeMasking.h"

#include "QRCodeBitPlacement.h"
#include "QRCodeFunctionPatterns.h"

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <stdexcept>


int QRCodeMasking::SelectBestMask(const std::vector<uint8_t>& _InterleavedCodewords, int _Version, CorrectionLevel _CorrectionLevel)
{
    int bestMask = 0;
    int bestScore = -1;
    for (int mask = 0; mask < 8; ++mask)
    {
        QRCodeData candidate{};
        QRCodeFunctionPatterns::Generate(candidate, _Version, _CorrectionLevel, mask);
        QRCodeBitPlacement::PlaceCodewords(_InterleavedCodewords, _Version, candidate.MatrixQR, candidate.Reserved);

        ApplyMask(candidate.MatrixQR, candidate.Reserved, mask);
        const int score = GetPenaltyScore(candidate.MatrixQR);

        if (bestScore < 0 || score < bestScore)
        {
            bestScore = score;
            bestMask = mask;
        }
    }
    return bestMask;
}

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

int QRCodeMasking::GetPenaltyScore(const std::vector<std::vector<int>>& _Matrix)
{
    const std::size_t size = _Matrix.size();
    if (size == 0)
    {
        throw std::invalid_argument("La matrice QR ne peut pas être vide.");
    }
    for (const auto& row : _Matrix)
    {
        if (row.size() != size)
        {
            throw std::invalid_argument("La matrice QR doit être carrée.");
        }
    }

    int score = 0;
    auto scoreLine = [&score, size](auto getBit)
    {
        int runColor = getBit(0);
        int runLength = 1;
        for (std::size_t i = 1; i < size; ++i)
        {
            const int bit = getBit(i);
            if (bit == runColor)
            {
                ++runLength;
                if (runLength == 5) score += 3;
                else if (runLength > 5) ++score;
            }
            else 
            { 
                runColor = bit; runLength = 1; 
            }
        }

        // N3: finder-like pattern 1011101 with four light modules
        // immediately before and/or after it.
        if (size >= 11)
        {
            for (std::size_t i = 0; i + 11 <= size; ++i)
            {
                bool before = true, after = true;
                for (std::size_t j = 0; j < 11; ++j)
                {
                    const int expectedBefore = (j == 4 || j == 6 || j == 7 || j == 8 || j == 10) ? 1 : 0; // 00001011101
                    const int expectedAfter = (j == 0 || j == 2 || j == 3 || j == 4 || j == 6) ? 1 : 0; // 10111010000
                    const int bit = getBit(i + j);
                    if (bit != expectedBefore) before = false;
                    if (bit != expectedAfter) after = false;
                }
                if (before)
                {
                    score += 40;
                }
                if (after)
                {
                    score += 40;
                }
            }
        }
    };

    for (std::size_t y = 0; y < size; ++y)
    {
        scoreLine([&](std::size_t x) { return _Matrix[y][x] != 0; });
    }
    for (std::size_t x = 0; x < size; ++x)
    {
        scoreLine([&](std::size_t y) { return _Matrix[y][x] != 0; });
    }

    // N2: monochrome 2x2 blocks.
    for (std::size_t y = 0; y + 1 < size; ++y)
    {
        for (std::size_t x = 0; x + 1 < size; ++x)
        {
            if (_Matrix[y][x] == _Matrix[y][x + 1] && _Matrix[y][x] == _Matrix[y + 1][x] && _Matrix[y][x] == _Matrix[y + 1][x + 1])
            {
                score += 3;
            }
        }
    }

    // N4: ten points for each complete 5% deviation from 50% dark modules.
    long long dark = 0;
    for (const auto& row : _Matrix)
    {
        for (int bit : row)
        {
            dark += bit != 0;
        }
    }

    const long long total = static_cast<long long>(size) * static_cast<long long>(size);
    const long long deviation = std::llabs(20 * dark - 10 * total) / total;
    score += static_cast<int>(deviation * 10);
    return score;
}
