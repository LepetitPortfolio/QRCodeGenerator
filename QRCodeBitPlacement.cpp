#include "QRCodeBitPlacement.h"
#include "QRCodeReedSolomonCorrector.h"

void QRCodeBitPlacement::PlaceCodewords(const std::vector<uint8_t>& _InterleavedCodewords, int _Version, std::vector<std::vector<int>>& _Matrix)
{
	const std::vector<std::vector<bool>> functionMask = MakeFunctionMask(_Version);
	PlaceCodewords(_InterleavedCodewords, _Version, _Matrix, functionMask);
}

void QRCodeBitPlacement::PlaceCodewords(const std::vector<uint8_t>& _InterleavedCodewords, int _Version, std::vector<std::vector<int>>& _Matrix, const std::vector<std::vector<bool>>& _Function)
{
	if (_Version < 1 || _Version > 40)
	{
		throw std::invalid_argument("La version QR doit être comprise entre 1 et 40.");
	}

	const int size = 17 + 4 * _Version;
	if (_Matrix.size() != static_cast<std::size_t>(size) || _Function.size() != static_cast<std::size_t>(size))
	{
		throw std::invalid_argument("La matrice ou la carte des motifs n'a pas la bonne taille.");
	}

	for (int y = 0; y < size; ++y)
	{
		if (_Matrix[static_cast<std::size_t>(y)].size() != static_cast<std::size_t>(size) ||
			_Function[static_cast<std::size_t>(y)].size() != static_cast<std::size_t>(size))
		{
			throw std::invalid_argument("La matrice ou la carte des motifs doit être carrée.");
		}
	}

	const std::size_t expected = static_cast<std::size_t>(QRCodeReedSolomonCorrector::GetRawCodewordCount(_Version));

	if (_InterleavedCodewords.size() != expected)
	{
		throw std::invalid_argument("Le nombre de codewords ne correspond pas à cette version.");
	}

	std::size_t bitIndex = 0;
	for (int right = size - 1; right >= 1; right -= 2)
	{
		if (right == 6)
		{
			right = 5; // la colonne 6 est la synchronisation verticale
		}

		const bool upward = ((right + 1) & 2) == 0;

		for (int vertical = 0; vertical < size; ++vertical)
		{
			const int y = upward ? size - 1 - vertical : vertical;
			for (int side = 0; side < 2; ++side)
			{
				const int x = right - side;
				if (_Function[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)])
				{
					continue;
				}

				int bit = 0;
				if (bitIndex < _InterleavedCodewords.size() * 8)
				{
					const uint8_t byte = _InterleavedCodewords[bitIndex / 8];
					bit = (byte >> (7 - (bitIndex % 8))) & 1U;
					++bitIndex;
				}

				_Matrix[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)] = bit;
			}
		}
	}

	if (bitIndex != _InterleavedCodewords.size() * 8)
	{
		throw std::logic_error("Toutes les données n'ont pas été placées dans la matrice.");
	}
}

std::vector<std::vector<bool>> QRCodeBitPlacement::MakeFunctionMask(int _Version)
{
	if (_Version < 1 || _Version > 40)
	{
		throw std::invalid_argument("La version QR doit être comprise entre 1 et 40.");
	}

	const int size = 17 + 4 * _Version;
	std::vector<std::vector<bool>> function(static_cast<std::size_t>(size), std::vector<bool>(static_cast<std::size_t>(size), false));

	// Repères de position et leur bordure de séparation blanche : carrés 9x9.
	ReserveRect(function, 0, 0, 9, 9, size);
	ReserveRect(function, size - 8, 0, 8, 9, size);
	ReserveRect(function, 0, size - 8, 9, 8, size);

	// Motifs de synchronisation, entre les repères.
	for (int i = 8; i < size - 8; ++i)
	{
		Reserve(function, 6, i, size);
		Reserve(function, i, 6, size);
	}

	// Centres des motifs d'alignement pour chaque version.
	if (_Version >= 2)
	{
		const int count = _Version / 7 + 2;
		const int step = (_Version * 8 + count * 3 + 5) / (count * 4 - 4) * 2;
		std::vector<int> centers(static_cast<std::size_t>(count));
		centers[0] = 6;
		for (int i = count - 1, position = size - 7; i >= 1; --i, position -= step)
		{
			centers[static_cast<std::size_t>(i)] = position;
		}

		for (int cy : centers)
		{
			for (int cx : centers)
			{
				// Les trois coins déjà occupés par les grands repères sont ignorés.
				if (function[static_cast<std::size_t>(cy)][static_cast<std::size_t>(cx)])
				{
					continue;
				}
				ReserveRect(function, cx - 2, cy - 2, 5, 5, size);
			}
		}
	}

	// Deux copies des 15 cases d'information de format et le module sombre fixe.
	for (int i = 0; i < 15; ++i)
	{
		int x1, y1;
		if (i <= 5) 
		{ 
			x1 = 8; y1 = i; 
		}
		else if (i == 6) 
		{ 
			x1 = 8; y1 = 7; 
		}
		else if (i == 7) 
		{ 
			x1 = 8; y1 = 8; 
		}
		else 
		{ 
			x1 = (i == 8) ? 7 : 14 - i; y1 = 8; 
		}
		
		Reserve(function, x1, y1, size);

		if (i <= 7) 
		{
			Reserve(function, size - 1 - i, 8, size);
		} 
		else 
		{
			Reserve(function, 8, size - 15 + i, size);
		}
	}
	
	Reserve(function, 8, size - 8, size);

	// Informations de version, présentes à partir de la version 7.
	if (_Version >= 7) 
	{
		ReserveRect(function, size - 11, 0, 3, 6, size);
		ReserveRect(function, 0, size - 11, 6, 3, size);
	}

	return function;
}

void QRCodeBitPlacement::ReserveRect(std::vector<std::vector<bool>>& _Function, int _X, int _Y, int _Width, int _Height, int _VersionSize)
{
	for (int dy = 0; dy < _Height; ++dy)
	{
		for (int dx = 0; dx < _Width; ++dx)
		{
			Reserve(_Function, _X + dx, _Y + dy, _VersionSize);
		}
	}
}

void QRCodeBitPlacement::Reserve(std::vector<std::vector<bool>>& _Function, int _X, int _Y, int _VersionSize)
{
	if (_X >= 0 && _Y >= 0 && _X < _VersionSize && _Y < _VersionSize)
	{
		_Function[static_cast<std::size_t>(_Y)][static_cast<std::size_t>(_X)] = true;
	}
}
