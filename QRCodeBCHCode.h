#pragma once

#include <vector>

class QRCodeBCHCode
{
public:

    /// @brief Calcule le code BCH pour une valeur d'entrée donnée
    /// @param _InputeValue Valeur d'entrée à encoder
    /// @param _MaxPower Puissance maximale utilisée pour le calcul
    /// @return Code BCH calculé
    static int CalculateBCHCode(int _InputeValue, int _MaxPower);

private:

    /// @brief Calcule le polynôme générateur BCH pour une puissance maximale donnée
    /// @param _MaxPower Puissance maximale pour le calcul
    /// @return Polynôme générateur BCH
    static std::vector<int> CalculateBCHPoly(int _MaxPower);

    /// @brief Retourne le polynôme minimal pour une puissance donnée (version simplifiée)
    /// @param _Power Puissance pour laquelle calculer le polynôme minimal
    /// @return Polynôme minimal sous forme de tableau de coefficients
    static std::vector<int> MinimalPoly(int _Power);

    /// @brief Multiplie deux polynômes dans le corps GF(2)
    /// @param _P1 Premier polynôme (coefficient de degré i à l'index i)
    /// @param _P2 Deuxième polynôme
    /// @return Résultat de la multiplication des deux polynômes
    static std::vector<int> MultiplPoly(const std::vector<int>& _P1, const std::vector<int>& _P2); 
};