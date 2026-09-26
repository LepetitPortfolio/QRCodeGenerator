#pragma once

#include <vector>

class QRCodeAligningPattern
{
public:

    QRCodeAligningPattern() = default;
    ~QRCodeAligningPattern() = default;

    static void GenerateAligningPattern(std::vector<std::vector<int>>& _SimpleQRCode, int _MatrixSize);


private:
};