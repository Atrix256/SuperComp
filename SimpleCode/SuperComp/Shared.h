#include <vector>
#include <stdint.h>
#include <iostream>
#include <algorithm>
#include "CSuperInt.h"

void WaitForEnter ();

//=================================================================================
template <size_t NUMBITS>
void ReportBitsAndError(const CSuperInt<NUMBITS, false> &superInt)
{
    const std::array<TINT, NUMBITS> &bits = superInt.GetBits();
    const std::vector<TINT> &keys = superInt.GetKeySet()->GetKeys();

    for (size_t i = 0; i < NUMBITS; ++i)
    {
        std::cout << "--Bit " << i << "--\n" << bits[i] << "\n";

        float maxError = 0.0f;
        for (int keyIndex = 0, keyCount = keys.size(); keyIndex < keyCount; ++keyIndex)
        {
            TINT residue = bits[i] % keys[keyIndex];
            float error = 100.0f * residue.convert_to<float>() / keys[keyIndex].convert_to<float>();
            if (error > maxError)
                maxError = error;
        }
        std::cout << "Highest Error = " << std::setprecision(2) << maxError << "%\n\n";
    }
}

//=================================================================================
template <typename L, size_t NUMBITS>
bool PermuteResults2Inputs(const CSuperInt<NUMBITS, false> &A, const CSuperInt<NUMBITS, false> &B, const CSuperInt<NUMBITS, false> &superResult, const std::vector<TINT> &keys, const L& lambda)
{
    bool ret = true;
    for (size_t b = 0, bc = (1 << NUMBITS) - 1; b <= bc; ++b)
    {
        for (size_t a = 0, ac = (1 << NUMBITS) - 1; a <= ac; ++a)
        {
            // get the index of our key for this specific set of inputs
            size_t keyIndex = (b << NUMBITS) | a;

            // decode result for this specific key
            size_t result = superResult.DecodeBinary(keys[keyIndex]);

            // call the lambda!
            ret = ret && lambda(a, b, keyIndex, keys[keyIndex], result);
        }
    }
    return ret;
}
