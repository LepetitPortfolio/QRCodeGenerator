#include "QRCodePNGFile.h"


void QRCodePNGFile::GeneratePNGFile(const std::string &_Filename, int _Width, int _Height, const std::vector<unsigned char> &_PixelData)
{
}

void QRCodePNGFile::SavePNG(const std::string &_Filename, int _Width, int _Height, const std::vector<unsigned char> &_PixelData)
{
    std::ofstream out(_Filename, std::ios::binary);

    unsigned char pngSignature[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };
    out.write((char*)pngSignature, 8);

    // IHDR chunk

    std::vector<unsigned char> ihdrData;
    for (int i = 24; i >= 0; i -= 8)
    {
        ihdrData.push_back((_Width >> i) & 0xff);
    }

    for (int i = 24; i >= 0; i -= 8)
    {
        ihdrData.push_back((_Height >> i) & 0xff);
    }

    ihdrData.insert(ihdrData.end(), { 8, 6, 0, 0, 0 }); // Bit depth, color type, compression, filter, interlace

    WriteChunk(out, "IHDR", ihdrData);

    std::vector<unsigned char> raw;
    raw.reserve((_Width * 4 + 1) * _Height);
    for (int y = 0; y < _Height; y++)
    {
        raw.push_back(0x00); // No filter
        const unsigned char* row = &_PixelData[y * _Width * 4];
        raw.insert(raw.end(), row, row + _Width * 4);
    }

    // IDAT (non compressé, format très simplifié)
    std::vector<unsigned char> idatData;
    idatData.push_back(0x78);
    idatData.push_back(0x01);

    size_t pos = 0;
    while (pos < raw.size())
    {
        size_t remain = raw.size() - pos;
        uint16_t chunkSize = static_cast<uint16_t>(remain > 65535 ? 65535 : remain);
        bool isFinal = (pos + chunkSize) == raw.size();

        unsigned char blockHeader = isFinal ? 0x01 : 0x00;
        idatData.push_back(blockHeader);

        idatData.push_back(static_cast<unsigned char>(chunkSize & 0xff));
        idatData.push_back(static_cast<unsigned char>((chunkSize >> 8) & 0xff));
        uint16_t nlen = static_cast<uint16_t>(~chunkSize);
        idatData.push_back(static_cast<unsigned char>(nlen & 0xff));
        idatData.push_back(static_cast<unsigned char>((nlen >> 8) & 0xff));

        idatData.insert(idatData.end(), raw.begin() + pos, raw.begin() + pos + chunkSize);

        pos += chunkSize;
    }

    uint32_t adler = Adler32(raw);
    idatData.push_back((adler >> 24) & 0xff);
    idatData.push_back((adler >> 16) & 0xff);
    idatData.push_back((adler >> 8) & 0xff);
    idatData.push_back(adler & 0xff);

    WriteChunk(out, "IDAT", idatData);
    WriteChunk(out, "IEND", {});

    out.close();
}

uint32_t QRCodePNGFile::Adler32(const std::vector<unsigned char> &_Data)
{
    const uint32_t MOD_ADLER = 65521;
    uint32_t a = 1, b = 0;

    for (unsigned char c : _Data)
    {
        a = (a + c) % MOD_ADLER;
        b = (b + a) % MOD_ADLER;
    }

    return (b << 16) | a;
}

void QRCodePNGFile::WriteChunk(std::ofstream& _Out, const char* _Type, const std::vector<unsigned char>& _Data)
{
    WriteInt(_Out, _Data.size()); // length
    _Out.write(_Type, 4); // Chunk type

    if (!_Data.empty())
    {
        _Out.write(reinterpret_cast<const char*>(_Data.data()), _Data.size()); // data
    }

    uint32_t crc32 = CRC32(_Data, _Type);

    WriteInt(_Out, crc32); // CRC
}

uint32_t QRCodePNGFile::CRC32(const std::vector<unsigned char> &_Data, const char *_Type)
{
    uint32_t crc = 0xffffffffu;
    auto UpdateCRC = [&](unsigned char byte)
        {
            crc ^= (uint32_t)byte;
            for (int i = 0; i < 8; i++)
            {
                if (crc & 1u)
                {
                    crc = (crc >> 1) ^ 0xEDB88320u;
                }
                else
                {
                    crc = (crc >> 1);
                }
            }
        };

    for (int i = 0; i < 4; i++)
    {
        UpdateCRC(static_cast<unsigned char>(_Type[i]));
    }

    for (unsigned char c : _Data)
    {
        UpdateCRC(c);
    }

    return ~crc;
}

void QRCodePNGFile::WriteInt(std::ofstream &_Out, uint32_t value)
{
    _Out.put((value >> 24) & 0xff);
    _Out.put((value >> 16) & 0xff);
    _Out.put((value >> 8) & 0xff);
    _Out.put(value & 0xff);
}
