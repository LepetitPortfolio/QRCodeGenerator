#pragma once

#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>


class QRCodePNGFile
{
public:

    /// @brief Sauvegarde une image PNG à partir de données de pixels RGBA
    /// @param _Filename Nom du fichier de sortie
    /// @param _Width Largeur de l'image en pixels
    /// @param _Height Hauteur de l'image en pixels
    /// @param _PixelData Données des pixels au format RGBA (4 octets par pixel)
    static void GeneratePNGFile(const std::string& _Filename, int _Width, int _Height, const std::vector<unsigned char>& _PixelData);

private:

    /// @brief Calcule la somme de contrôle Adler-32 pour les données
    /// @param _Data Données sous forme de tableau d'octets
    /// @return Valeur Adler-32 calculée
    static uint32_t Adler32(const std::vector<unsigned char>& _Data);

    /// @brief Écrit un chunk PNG dans le fichier (longueur, type, données, CRC)
    /// @param _Out Flux de sortie binaire
    /// @param _Type Type de chunk (4 caractères)
    /// @param _Data Données du chunk
    static void WriteChunk(std::ofstream& _Out, const char* _Type, const std::vector<unsigned char>& _Data);

    /// @brief Calcule la somme de contrôle CRC32 pour des données et un type donnés
    /// @param _Data Données sous forme de tableau d'octets
    /// @param _Type Type de chunk (4 caractères) utilisé dans le calcul CRC
    /// @return Valeur CRC32 calculée
    static uint32_t CRC32(const std::vector<unsigned char>& _Data, const char* _Type);

    /// @brief Écrit un entier 32 bits dans un flux binaire au format big-endian (4 octets)
    /// @param _Out Flux de sortie binaire
    /// @param value Valeur entière 32 bits à écrire
    static void WriteInt(std::ofstream& _Out, uint32_t value);

};