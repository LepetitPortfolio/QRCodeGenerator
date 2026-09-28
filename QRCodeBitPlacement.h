#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

class QRCodeBitPlacement
{
public:

	static void PlaceCodewords(const std::vector<uint8_t>& _InterleavedCodewords, int _Version, std::vector<std::vector<int>>& _Matrix);

 private:

     static void PlaceCodewords(const std::vector<uint8_t>& _InterleavedCodewords, int _Version, std::vector<std::vector<int>>& _Matrix, const std::vector<std::vector<bool>>& _Function);

     static  std::vector<std::vector<bool>> MakeFunctionMask(int _Version);

     static void ReserveRect(std::vector<std::vector<bool>>& _Function, int _X, int _Y, int _Width, int _Height, int _VersionSize);

     static void Reserve(std::vector<std::vector<bool>>& _Function, int _X, int _Y, int _VersionSize);

};

