#pragma once
#include "QRCodeFormatPattern.h"

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

class QRCodeReedSolomonCorrector
{
public :


    /**
    * @brief Effectue la correction d'erreur Reed-Solomon et l'entrelacement des données pour un code QR.
    * @param _Data Données brutes à encoder (sous forme de codewords).
    * @param _Version Version du code QR (de 1 à 40).
    * @param _CorrectionLevel Niveau de correction d'erreur (L, M, Q, ou H).
    * @return Vecteur d'octets contenant les données entrelacées avec les codes de correction.
    * @throws std::invalid_argument Si la version ou le niveau de correction est invalide, ou si la taille des données ne correspond pas.
    * @throws std::logic_error Si la répartition des données en blocs est incohérente.
    *
    * @details
    * ### Étapes principales :
    * 1. **Validation des paramètres** :
    *    - Vérifie que `_Version` est entre 1 et 40.
    *    - Vérifie que `_CorrectionLevel` est valide (L=0, M=1, Q=2, H=3).
    *    - Vérifie que la taille de `_Data` correspond au nombre attendu de codewords pour la version et le niveau de correction donnés.
    *
    * 2. **Calcul des paramètres de bloc** :
    *    - `eccLength` : Nombre d'octets de correction (ECC) par bloc.
    *    - `blockCount` : Nombre total de blocs.
    *    - `rawCodewords` : Nombre total de codewords (données + ECC) pour la version.
    *    - `shortBlockCount` : Nombre de blocs "courts" (qui ont un codeword de moins que les blocs longs).
    *    - `shortBlockLength` : Longueur des blocs courts (sans les ECC).
    *
    * 3. **Génération du polynôme générateur** :
    *    - Utilise `MakeGenerator(eccLength)` pour créer un polynôme générateur de degré `eccLength`.
    *
    * 4. **Division des données en blocs** :
    *    - Les données sont divisées en `blockCount` blocs.
    *    - Chaque bloc a une longueur de `shortBlockLength - eccLength` (ou `shortBlockLength - eccLength + 1` pour les blocs longs).
    *    - Pour chaque bloc :
    *      - Extrait une sous-séquence de `_Data` pour former `blockData`.
    *      - Calcule les octets ECC pour ce bloc avec `MakeRemainder(blockData, generator)`.
    *      - Ajoute un octet fictif (`0`) aux blocs courts pour égaliser leur longueur avec les blocs longs.
    *      - Ajoute les octets ECC à `blockData`.
    *
    * 5. **Entrelacement des blocs** :
    *    - Les blocs sont entrelacés **colonne par colonne** : d'abord toutes les données du premier octet de chaque bloc, puis toutes les données du deuxième octet, etc.
    *    - Les octets fictifs des blocs courts sont ignorés pendant l'entrelacement.
    *
    * 6. **Validation du résultat** :
    *    - Vérifie que la taille du résultat final correspond à `rawCodewords`.
    */
    static std::vector<uint8_t> ErrorCorrectionAndInterleave(const std::vector<uint8_t>& _Data, int _Version, CorrectionLevel correctionLevel);

    /**
    * @brief Calcule le nombre total de codewords (données + ECC) pour une version de code QR donnée.
    * @param _Version Version du code QR (de 1 à 40).
    * @return Nombre total de codewords.
    *
    * @details
    * ### Calcul du nombre de modules :
    * - Pour la version 1, le nombre de modules est fixe (20x20 = 400 modules, mais 264 modules de données).
    * - Pour les versions > 1, le nombre de modules est calculé avec la formule :
    *   `modules = (16 * _Version + 128) * _Version + 64`.
    *   Cette formule donne le nombre total de modules (pixels) dans le code QR.
    *
    * ### Ajustement pour les motifs d'alignement :
    * - Pour les versions >= 2, des **motifs d'alignement** sont ajoutés pour aider à la lecture du code.
    *   - Le nombre de motifs d'alignement est `_Version / 7 + 2`.
    *   - Chaque motif d'alignement occupe un certain nombre de modules, qui est soustrait du total.
    * - Pour les versions >= 7, un ajustement supplémentaire de 36 modules est appliqué.
    *
    * ### Conversion en codewords :
    * - Chaque codeword est composé de **8 modules** (1 octet = 8 bits).
    * - Le nombre total de codewords est donc `modules / 8`.
    */
    static int GetRawCodewordCount(int _Version);

private:

    /**
    * @brief Multiplie deux octets dans le corps fini GF(256).
    * @param _X Premier octet.
    * @param _Y Deuxième octet.
    * @return Résultat de la multiplication dans GF(256).
    *
    * @details
    * ### Corps fini GF(256) :
    * - GF(256) est un corps fini avec 256 éléments (0 à 255), où les opérations arithmétiques sont définies de manière à former un corps algébrique.
    * - La multiplication dans GF(256) est basée sur un **polynôme irréductible** (souvent x^8 + x^4 + x^3 + x^2 + 1, représenté par 0x11D en hexadécimal).
    *
    * ### Algorithme :
    * - Initialise `z` à 0.
    * - Pour chaque bit de `_Y` (du bit 7 au bit 0) :
    *   - Décale `z` d'un bit vers la gauche.
    *   - Si le bit de poids fort de `z` est 1, applique un XOR avec le polynôme irréductible (0x11D).
    *   - Si le bit courant de `_Y` est 1, applique un XOR avec `_X`.
    * - Le résultat final est `z` (tronqué à 8 bits).
    *
    * @note
    * - Cet algorithme est une implémentation standard de la multiplication dans GF(256).
    * - Le polynôme irréductible utilisé ici est **x^8 + x^4 + x^3 + x^2 + 1** (0x11D).
    */
    static uint8_t MultiplyGF256(uint8_t _X, uint8_t _Y);

    /**
    * @brief Génère un polynôme générateur de degré `_Degree` pour la correction d'erreur Reed-Solomon.
    * @param _Degree Degré du polynôme générateur (nombre d'octets ECC).
    * @return Vecteur d'octets représentant le polynôme générateur.
    *
    * @details
    * ### Algorithme :
    * - Initialise un vecteur `divisor` de taille `_Degree` avec tous les éléments à 0, sauf le dernier (mis à 1).
    * - Initialise `root` à 1 (élément primitif du corps GF(256)).
    * - Pour chaque itération de 0 à `_Degree - 1` :
    *   - Multiplie chaque coefficient du polynôme `divisor` par `root` (en utilisant `MultiplyGF256`).
    *   - Effectue une opération XOR entre les coefficients adjacents.
    *   - Met à jour `root` en le multipliant par 2 (dans GF(256)).
    *
    * @note
    * - Le polynôme générateur est de la forme : (x - α^0)(x - α^1)...(x - α^(degree-1)), où α est un élément primitif de GF(256).
    * - Ici, `root` commence à 1 (α^0) et est multiplié par 2 à chaque itération pour obtenir α^1, α^2, etc.
    */
	static std::vector<uint8_t> MakeGenerator(int _Degree);

    /**
    * @brief Calcule le reste (ECC) pour un bloc de données en utilisant un polynôme générateur.
    * @param _Data Données à corriger (sous forme de codewords).
    * @param _Generator Polynôme générateur (obtenu via `MakeGenerator`).
    * @return Vecteur d'octets représentant les octets de correction (ECC).
    *
    * @details
    * ### Algorithme :
    * - Initialise un vecteur `remainder` de la même taille que `_Generator`, rempli de zéros.
    * - Pour chaque octet dans `_Data` :
    *   - Calcule un `factor` comme le XOR entre l'octet courant et le premier octet du reste (`remainder.front()`).
    *   - Décale les octets du reste vers la gauche (le premier octet est perdu, le dernier est mis à 0).
    *   - Pour chaque octet du reste, effectue un XOR avec `MultiplyGF256(_Generator[i], factor)`.
    * - Le résultat final est le reste de la division polynomiale, qui représente les octets ECC.
    *
    * @note
    * - Cet algorithme est une implémentation standard de la division polynomiale dans GF(256) pour Reed-Solomon.
    * - Le reste a la même longueur que le polynôme générateur (donc `_Generator.size()` octets).
    */
    static std::vector<uint8_t> MakeRemainder(const std::vector<uint8_t>& _Data, const std::vector<uint8_t>& _Generator);
};

