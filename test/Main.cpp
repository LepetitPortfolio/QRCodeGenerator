#include "QRCodeGenerator.h"

#include <iostream>
#include <vector>
#include <string>

/// @brief Affiche la matrice QR dans la console avec des caractères personnalisables
/// @param _MatrixQR Matrice QR à afficher
/// @param _0Bit Caractère utilisé pour représenter un 0
/// @param _1bit Caractère utilisé pour représenter un 1
void ShowMatrixQR(const std::vector<std::vector<int>>& _MatrixQR, std::string _0Bit, std::string _1bit)
{
	for (const auto& row : _MatrixQR)
	{
		for (int bit : row)
		{
			std::cout << (bit ? _1bit : _0Bit);
		}
		std::cout << std::endl;
	}
}
/// @brief Fonction principale : demande le texte à l'utilisateur, génère le QR code et l'affiche
/// @return Code de retour du programme
int main()
{
	std::string text;
	std::cout << "Entrez le texte ou URL à coder : ";
	std::getline(std::cin, text);

	std::string filename = "QRCode.png";

	QRCodeGenerator::GenerateQRCode(filename, text, CorrectionLevel::M);
	
	std::cout << "QR Code enregistré sous " << filename << std::endl;

	return 0;
}