#pragma once
#include "QRCodeData.h"

class QRCodeFunctionPatterns
{
public:

    static void Generate(QRCodeData& _QRCodeData, int _Version, CorrectionLevel _CorrectionLevel, int _MaskPattern);

private:

    static void Set(QRCodeData& _QRCodeData, int _X, int _Y, bool _Dark);

	static void DrawFinder(QRCodeData& _QRCodeData, int centerX, int centerY);

    static std::vector<int> GetAlignmentCenters(int _Version, int _Size);

    static void DrawAlignment(QRCodeData& _QRCodeData, int _CenterX, int _CenterY);

    static void DrawFormat(QRCodeData& _QRCodeData, CorrectionLevel _CorrectionLevel, int _MaskPattern);

    static void DrawVersion(QRCodeData& _QRCodeData, int _Version);
};

