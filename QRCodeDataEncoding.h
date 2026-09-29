#pragma once
#include "QRCodeFormatPattern.h"

#include <string>
#include <vector>

class QRCodeDataEncoding
{
public:

	int SelectVersionForText(const std::string& text, CorrectionLevel correctionLevel);

	/**
	* @brief Encode un texte en codewords pour un code QR, en utilisant le mode octets.
	* @param _Text Texte à encoder.
	* @param _Version Version du code QR (de 1 à 40).
	* @param _CorrectionLevel Niveau de correction d'erreur (L, M, Q, ou H).
	* @return Vecteur d'octets (codewords) représentant le texte encodé.
	* @throws std::invalid_argument Si la version ou le niveau de correction est invalide.
	* @throws std::length_error Si le texte est trop long pour la version et le niveau de correction donnés.
	*
	* @details
	* ### Étapes de l'encodage :
	* 1. **Validation des paramètres** :
	*    - Vérifie que `_Version` est entre 1 et 40.
	*    - Vérifie que `_CorrectionLevel` est valide (L=0, M=1, Q=2, H=3).
	*
	* 2. **Calcul de la capacité** :
	*    - `capacityCodewords` : Nombre maximal de codewords pour la version et le niveau de correction donnés.
	*    - `capacityBits` : Nombre maximal de bits (1 codeword = 8 bits).
	*    - `countBitWidth` : Largeur en bits de l'indicateur de longueur (8 bits pour les versions 1-9, 16 bits pour les versions 10-40).
	*    - `maxByteCount` : Longueur maximale du texte en mode octets (255 pour 8 bits, 65535 pour 16 bits).
	*
	* 3. **Vérification de la taille du texte** :
	*    - Si `_Text.size()` dépasse `maxByteCount`, lève une exception.
	*
	* 4. **Encodage du texte** :
	*    - **Indicateur de mode** : Ajoute `0100` (4 bits) pour indiquer le mode octets.
	*    - **Indicateur de longueur** : Encode la longueur du texte (`_Text.size()`) sur `countBitWidth` bits.
	*    - **Données** : Encode chaque caractère du texte en 8 bits (1 octet).
	*
	* 5. **Ajout du terminateur** :
	*    - Ajoute jusqu'à 4 zéros pour marquer la fin des données (si la capacité le permet).
	*
	* 6. **Remplissage jusqu'à un octet** :
	*    - Si le nombre total de bits n'est pas un multiple de 8, ajoute des zéros pour compléter jusqu'à l'octet suivant.
	*
	* 7. **Conversion en codewords** :
	*    - Regroupe les bits par paquets de 8 pour former des octets (codewords).
	*
	* 8. **Remplissage (padding)** :
	*    - Si le nombre de codewords est inférieur à `capacityCodewords`, ajoute des octets de remplissage en alternant entre `0xEC` et `0x11`.
	*/
	static std::vector<uint8_t> EncodeTextToDataCodewords(const std::string& _Text, int _Version, CorrectionLevel _CorrectionLevel);

private:

	/**
	* @brief Ajoute les bits d'une valeur entière à un vecteur de bits.
	* @param _Bits Vecteur de bits où ajouter les nouveaux bits.
	* @param _Value Valeur entière à encoder en bits.
	* @param _BitCount Nombre de bits à extraire de `_Value` (doit être <= 32, car `_Value` est un uint32_t).
	*
	* @details
	* - Parcourt les bits de `_Value` du **bit le plus significatif** au **bit le moins significatif** (de gauche à droite).
	* - Pour chaque bit, extrait-le avec `(_Value >> i) & 1` et l'ajoute à `_Bits`.
	* - Exemple : Si `_Value = 5` (binaire `0101`) et `_BitCount = 4`, les bits ajoutés seront `[0, 1, 0, 1]`.
	*
	* @note
	* - Les bits sont ajoutés dans l'ordre **MSB (Most Significant Bit) first**, ce qui est cohérent avec la norme QR Code.
	* - Si `_BitCount` est supérieur à 32, le comportement est indéfini (car `_Value` est un `uint32_t`).
	*/
	static void AppendBits(std::vector<uint8_t>& _Bits, uint32_t _Value, int _BitCount);
};

