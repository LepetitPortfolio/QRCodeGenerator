#include "QRCodeReedSolomonCorrector.h"
#include "QRCodeData.h"

std::vector<uint8_t> QRCodeReedSolomonCorrector::ErrorCorrectionAndInterleave(const std::vector<uint8_t>& _Data, int _Version, CorrectionLevel _CorrectionLevel)
{
    if (_Version < 1 || _Version > 40)
    {
        throw std::invalid_argument("La version QR doit être comprise entre 1 et 40.");
    }

    const auto level = static_cast<std::size_t>(_CorrectionLevel);
    if (level > 3)
    {
        throw std::invalid_argument("Niveau de correction invalide (L, M, Q ou H).");
    }

    const std::size_t expectedData = DataCodewords[_Version - 1][level];
    if (_Data.size() != expectedData)
    {
        throw std::invalid_argument("Le nombre de codewords de données ne correspond pas à la version/niveau.");
    }

    const int eccLength = EccCodewordsPerBlock[level][_Version - 1];
    const int blockCount = BlockCount[level][_Version - 1];
    const int rawCodewords = GetRawCodewordCount(_Version);
    const int shortBlockCount = blockCount - rawCodewords % blockCount;
    const int shortBlockLength = rawCodewords / blockCount;
    const auto generator = MakeGenerator(eccLength);

    std::vector<std::vector<uint8_t>> blocks;
    blocks.reserve(blockCount);
    std::size_t offset = 0;

    for (int blockIndex = 0; blockIndex < blockCount; ++blockIndex) 
    {
        const std::size_t blockDataLength = static_cast<std::size_t>(
            shortBlockLength - eccLength + (blockIndex < shortBlockCount ? 0 : 1));
        std::vector<uint8_t> blockData(
            _Data.begin() + static_cast<std::ptrdiff_t>(offset),
            _Data.begin() + static_cast<std::ptrdiff_t>(offset + blockDataLength));
        offset += blockDataLength;

        auto ecc = MakeRemainder(blockData, generator);

        if (blockIndex < shortBlockCount)
        {
            blockData.push_back(0); // place réservée pour égaliser la longueur des blocs
        }

        blockData.insert(blockData.end(), ecc.begin(), ecc.end());
        blocks.push_back(std::move(blockData));
    }

    if (offset != _Data.size())
    {
        throw std::logic_error("La répartition des données en blocs est incohérente.");
    }

    std::vector<uint8_t> result;
    result.reserve(rawCodewords);
    // Entrelacement colonne par colonne : d'abord données, puis octets ECC.
    for (std::size_t column = 0; column < blocks.front().size(); ++column) 
    {
        for (int blockIndex = 0; blockIndex < blockCount; ++blockIndex) 
        {
            // Les blocs courts possèdent un octet fictif avant leur zone ECC.
            if (column == static_cast<std::size_t>(shortBlockLength - eccLength) && blockIndex < shortBlockCount)
                continue;
            result.push_back(blocks[blockIndex][column]);
        }
    }
    if (result.size() != static_cast<std::size_t>(rawCodewords))
    {
        throw std::logic_error("Le nombre final de codewords est incohérent.");
    }
    return result;
}

int QRCodeReedSolomonCorrector::GetRawCodewordCount(int _Version)
{
    int modules = (16 * _Version + 128) * _Version + 64;

    if (_Version >= 2) 
    {
        const int alignmentCount = _Version / 7 + 2;
        modules -= (25 * alignmentCount - 10) * alignmentCount - 55;

        if (_Version >= 7)
        {
            modules -= 36;
        }
    }
    return modules / 8;
}

uint8_t QRCodeReedSolomonCorrector::MultiplyGF256(uint8_t _X, uint8_t _Y)
{
    uint16_t z = 0;
    for (int i = 7; i >= 0; --i) {
        z = static_cast<uint16_t>((z << 1) ^ ((z & 0x80) ? 0x11D : 0));
        if ((_Y >> i) & 1U) z ^= _X;
    }
    return static_cast<uint8_t>(z);
}

std::vector<uint8_t> QRCodeReedSolomonCorrector::MakeGenerator(int _Degree)
{
    std::vector<uint8_t> divisor(static_cast<std::size_t>(_Degree), 0);
    divisor.back() = 1;
    uint8_t root = 1;
    
    for (int i = 0; i < _Degree; ++i) 
    {
        for (std::size_t j = 0; j < divisor.size(); ++j) 
        {
            divisor[j] = MultiplyGF256(divisor[j], root);
            if (j + 1 < divisor.size())
            {
                divisor[j] ^= divisor[j + 1];
            }
        }
        root = MultiplyGF256(root, 2);
    }

    return divisor;
}

std::vector<uint8_t> QRCodeReedSolomonCorrector::MakeRemainder(const std::vector<uint8_t>& _Data, const std::vector<uint8_t>& _Generator)
{
    std::vector<uint8_t> remainder(_Generator.size(), 0);
    for (uint8_t byte : _Data) 
    {
        const uint8_t factor = byte ^ remainder.front();
        for (std::size_t i = 0; i + 1 < remainder.size(); ++i)
        {
            remainder[i] = remainder[i + 1];
        }
        
        remainder.back() = 0;

        for (std::size_t i = 0; i < remainder.size(); ++i)
        {
            remainder[i] ^= MultiplyGF256(_Generator[i], factor);
        }
    }
    return remainder;
}
