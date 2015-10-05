#include "shared.h"
#include "CKeySet.h"

//=================================================================================
void WaitForEnter ()
{
    std::cout << "\nPress Enter to quit";
    fflush(stdin);
    getchar();
}

//=================================================================================
void ReportBitsAndError (const CSuperInt &superInt)
{
    const std::vector<TINT> &bits = superInt.GetBits();
    const std::vector<TINT> &keys = superInt.GetKeySet().GetKeys();

    for (size_t i = 0, c = bits.size(); i < c; ++i)
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
