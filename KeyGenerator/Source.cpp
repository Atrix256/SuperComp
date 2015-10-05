#include <stdio.h>
#include <algorithm>
#include <array>
#include <vector>
#include "Shared/Shared.h"
#include "Shared/CKeySet.h"

//=================================================================================
bool TestResults (const std::vector<TINT> &superPositionedBits, const std::vector<TINT> &keys)
{
    bool ret = true;
    for (size_t bitIndex = 0, bitCount = superPositionedBits.size(); bitIndex < bitCount; ++bitIndex)
    {
        const TINT bitMask = 1 << bitIndex;
        for (size_t keyIndex = 0, keyCount = keys.size(); keyIndex < keyCount; ++keyIndex)
        {
            TINT desiredResult = ((keyIndex & bitMask) != 0) ? 1 : 0;
            
            if (superPositionedBits[bitIndex] % keys[keyIndex] != desiredResult)
            {
                ret = false;
                std::cout << "ERROR: bit " << bitIndex << " (" << superPositionedBits[bitIndex] << ") "
                          << "mod key " << keyIndex << " (" << keys[keyIndex] << ") "
                          << "did not equal " << desiredResult << "!\n";
            }
        }
    }
    return ret;
}

//=================================================================================
int main (int argc, char **argv)
{
    // get and verify parameters
    size_t numBits, minKey;
    std::cout << "--KeyGenerator--\n\nGenerates superpositioned bit values and keys for use in superpositional\ncomputation using HE over the integers.\n\n";
    if (argc < 4 || sscanf(argv[1], "%i", &numBits) != 1 || sscanf(argv[2], "%i", &minKey) != 1)
    {
        std::cout << "Usage: <numBits> <minKey> <outputFile>\n";;
        ExitCode_(1);
    }

    // calculate superpositioned bits and keys
    std::cout << "Calculating values for " << numBits << " bits.\n";
    CKeySet keySet;
    keySet.Calculate(numBits, minKey);

    // Verify results
    std::cout << "Done.\n\nVerifying results...\n";
    if (!TestResults(keySet.GetSuperPositionedBits(), keySet.GetKeys()))
    {
        std::cout << "Results Invalid!!\n";
        ExitCode_(1);
    }
    std::cout << "Done.\n";

    // Write results
    if (!keySet.Write(argv[3]))
    {
        std::cout << "\nCould not write results to " << argv[3] << "\n";
        ExitCode_(1);
    }
    std::cout << "\nResults written to " << argv[3] << "\n";
    ExitCode_(0);
}